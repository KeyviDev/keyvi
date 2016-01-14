// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2011, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

///////////////////////////////////////////////////////////////////////////////
/// \file parallel_sort.h
/// Simple parallel quick sort implementation with progress tracking.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PARALLEL_SORT_H__
#define __TPIE_PARALLEL_SORT_H__

#include <algorithm>
#include <cstdint>
#include <boost/iterator/iterator_traits.hpp>
#include <mutex>
#include <cmath>
#include <functional>
#include <tpie/progress_indicator_base.h>
#include <tpie/dummy_progress.h>
#include <tpie/internal_queue.h>
#include <tpie/job.h>
#include <tpie/config.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief A simple parallel sort implementation with progress tracking.
/// The partition step is sequential, as a parallel partition only speeds up
/// the top layer, which does not warrant the hassle of implementation.
/// Uses the TPIE job manager to transparently distribute work across the
/// machine cores.
/// Uses the pseudo median of nine as pivot.
///////////////////////////////////////////////////////////////////////////////
template <typename iterator_type, typename comp_type, bool Progress,
		  size_t min_size=1024*1024*8/sizeof(typename boost::iterator_value<iterator_type>::type)>
class parallel_sort_impl {
private:
	typedef progress_types<Progress> P;

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Internal class for parallel progress reporting.
	///////////////////////////////////////////////////////////////////////////////
	struct progress_t {
		typename P::base * pi;
		std::uint64_t work_estimate;
		std::uint64_t total_work_estimate;
		std::condition_variable cond;
		std::mutex mutex;
	};

	/** \brief The type of the values we sort. */
	typedef typename boost::iterator_value<iterator_type>::type value_type;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Guesstimate how much work a sort uses.
	///////////////////////////////////////////////////////////////////////////
	static inline std::uint64_t sortWork(std::uint64_t n) {
		if(n == 0)
			return 0;

		return static_cast<uint64_t>(log(static_cast<double>(n)) * static_cast<double>(n) * 1.8
				/ log(static_cast<double>(2)));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Partition using *first as pivot.
	/// \param first Iterator to left boundary.
	/// \param last Iterator to right boundary.
	/// \param comp Comparator.
	///////////////////////////////////////////////////////////////////////////
	template <typename comp_t>
	static inline iterator_type unguarded_partition(iterator_type first, 
													iterator_type last, 
													comp_t & comp) {
		// Textbook partitioning.
		iterator_type pivot = first;
		while (true) {
			do --last;
			while (comp(*pivot, *last));

			do {
				if (first == last) break;
				++first;
			} while (comp(*first, *pivot));

			if (first == last) break;

			std::iter_swap(first, last);
		}
		std::iter_swap(last, pivot);
		return last;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Median of three.
	/// \param a Iterator to an element.
	/// \param b Iterator to an element.
	/// \param c Iterator to an element.
	/// \param comp Comparator.
	///////////////////////////////////////////////////////////////////////////
	static inline iterator_type median(iterator_type a, iterator_type b, iterator_type c, comp_type & comp) {
		if (comp(*a, *b)) {
			if (comp(*b, *c)) return b;
			else if (comp(*a, *c)) return c;
			else return a;
		} else {
			if (comp(*a, *c)) return a;
			else if (comp(*b, *c)) return c;
			else return b;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Pseudo-median of nine.
	/// It uses the boundary elements as well as the seven 0.125-fractiles
	/// to find a good element for partitioning.
	/// \param a Iterator to left boundary.
	/// \param b Iterator to right boundary.
	/// \param comp Comparator.
	///////////////////////////////////////////////////////////////////////////
	static inline iterator_type pick_pivot(iterator_type a, iterator_type b, comp_type & comp) {
		if (a == b) return a;
		assert(a < b);

		// Since (b-a) is at least min_size, which is at least 100000 in
		// realistic contexts, ((b-a)/8)*c is a good approximation of
		// (c*(b-a))/8.
		size_t step = (b-a)/8;

		return median(median(a+0, a+step, a+step*2, comp),
					  median(a+step*3, a+step*4, a+step*5, comp),
					  median(a+step*6, a+step*7, b-1, comp), comp);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Partition using pivot returned by pick_pivot.
	/// \param a Iterator to left boundary.
	/// \param b Iterator to right boundary.
	/// \param comp Comparator.
	///////////////////////////////////////////////////////////////////////////
	static inline iterator_type partition(iterator_type a, iterator_type b, comp_type & comp) {
		iterator_type pivot = pick_pivot(a, b, comp);

		std::iter_swap(pivot, a);
		iterator_type l = unguarded_partition(a, b, comp);

		return l;
	}

#ifdef DOXYGEN
public:
#endif
	///////////////////////////////////////////////////////////////////////////
	/// \brief Represents quick sort work at a given level.
	///////////////////////////////////////////////////////////////////////////
	class qsort_job : public job {
	public:
		///////////////////////////////////////////////////////////////////////
		/// \brief Construct a qsort_job.
		///////////////////////////////////////////////////////////////////////
		qsort_job(iterator_type a, iterator_type b, comp_type comp, qsort_job * parent, progress_t & p)
			: a(a), b(b), comp(comp), parent(parent), progress(p) {

			// Does nothing.
		}

		~qsort_job() {
			for (size_t i = 0; i < children.size(); ++i) {
				delete children[i];
			}
			children.resize(0);
		}

		///////////////////////////////////////////////////////////////////////
		/// Running a job with iterators a and b will repeatedly partition
		/// [a,b), spawn a job on the left part and recurse on the right part,
		/// until the min_size limit is reached.
		///////////////////////////////////////////////////////////////////////
		virtual void operator()() override {
			assert(a <= b);
			while (static_cast<size_t>(b - a) >= min_size) {
				iterator_type pivot = partition(a, b, comp);
				add_progress(b - a);
				//qsort_job * j = tpie_new<qsort_job>(a, pivot, comp, this);
				qsort_job * j = new qsort_job(a, pivot, comp, this, progress);
				j->enqueue(this);
				children.push_back(j);
				a = pivot+1;
			}
			std::sort(a, b, comp);
			add_progress(sortWork(b - a));
		}

	protected:
		virtual void on_done() override {
			// Unfortunately, it might not be safe to delete our children at
			// this point, as other threads might in theory wait for them to
			// .join(). It is safer to postpone deletion until our own
			// deletion.
			if (!parent) {
				std::lock_guard<std::mutex> lock(progress.mutex);
				progress.work_estimate = progress.total_work_estimate;
				progress.cond.notify_one();
			}
		}

	private:
		iterator_type a;
		iterator_type b;
		comp_type comp;
		qsort_job * parent;
		progress_t & progress;

		std::vector<qsort_job *> children;

		void add_progress(uint64_t amount) {
			std::lock_guard<std::mutex> lock(progress.mutex);
			progress.work_estimate += amount;
			progress.cond.notify_one();
		}
	};
public:
	parallel_sort_impl(typename P::base * p) {
		progress.pi = p;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Perform a parallel sort of the items in the interval [a,b).
	/// Waits until all workers are done. The calling thread handles progress
	/// tracking, so a thread-safe progress tracker is not required.
	///////////////////////////////////////////////////////////////////////////
	void operator()(iterator_type a, iterator_type b, comp_type comp=std::less<value_type>() ) {
		progress.work_estimate = 0;
		progress.total_work_estimate = sortWork(b-a);
		if (progress.pi) progress.pi->init(progress.total_work_estimate);

		if (static_cast<size_t>(b - a) < min_size) {
			std::sort(a, b, comp);
			if (progress.pi) progress.pi->done();
			return;
		}

		qsort_job * master = new qsort_job(a, b, comp, 0, progress);
		master->enqueue();

		std::uint64_t prev_work_estimate = 0;
		std::unique_lock<std::mutex> lock(progress.mutex);
		while (progress.work_estimate < progress.total_work_estimate) {
			if (progress.pi && progress.work_estimate > prev_work_estimate) progress.pi->step(progress.work_estimate - prev_work_estimate);
			prev_work_estimate = progress.work_estimate;
			progress.cond.wait(lock);
		}
		lock.unlock();

		master->join();
		delete master;
		if (progress.pi) progress.pi->done();
	}
private:
	static const size_t max_job_count=256;
	progress_t progress;
	bool kill;
	size_t working;

	std::pair<iterator_type, iterator_type> jobs[max_job_count];
	size_t job_count;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort items in the range [a,b) using a parallel quick sort.
/// \param a Iterator to left boundary.
/// \param b Iterator to right boundary.
/// \param pi Progress tracker. No thread-safety required.
/// \param comp Comparator.
/// \sa parallel_sort_impl
///////////////////////////////////////////////////////////////////////////////
template <bool Progress, typename iterator_type, typename comp_type>
void parallel_sort(iterator_type a, 
				   iterator_type b, 
				   typename tpie::progress_types<Progress>::base & pi,
				   comp_type comp=std::less<typename boost::iterator_value<iterator_type>::type>()) {
#ifdef TPIE_PARALLEL_SORT
	parallel_sort_impl<iterator_type, comp_type, Progress> s(&pi);
	s(a,b,comp);
#else
	pi.init(1);
	std::sort(a,b,comp);
	pi.done();
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort items in the range [a,b) using a parallel quick sort.
/// \param a Iterator to left boundary.
/// \param b Iterator to right boundary.
/// \param comp Comparator.
/// \sa parallel_sort_impl
///////////////////////////////////////////////////////////////////////////////
template <typename iterator_type, typename comp_type>
void parallel_sort(iterator_type a, 
				   iterator_type b, 
				   comp_type comp=std::less<typename boost::iterator_value<iterator_type>::type>()) {
#ifdef TPIE_PARALLEL_SORT
	parallel_sort_impl<iterator_type, comp_type, false> s(0);
	s(a,b,comp);
#else
	std::sort(a, b, comp);
#endif
}


}
#endif //__TPIE_PARALLEL_SORT_H__
