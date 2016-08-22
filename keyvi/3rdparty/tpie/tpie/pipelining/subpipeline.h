// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2016, The TPIE development team
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
#ifndef __TPIE_PIPELINING_SUBPIPELINE_H__
#define __TPIE_PIPELINING_SUBPIPELINE_H__

#include <tpie/pipelining/pipeline.h>
#include <tpie/pipelining/runtime.h>

namespace tpie {
namespace pipelining {
namespace bits {

class subpipeline_base: public pipeline_base_base {
public:
	void begin(stream_size_type items, progress_indicator_base & pi,
			memory_size_type filesAvailable, memory_size_type mem,
			   const char * file, const char * function);
	void begin(stream_size_type items, progress_indicator_base & pi,
			   memory_size_type mem,
			   const char * file, const char * function) {
		begin(items, pi, get_file_manager().available(), mem, file, function);
	}
	void end();
protected:
	node * frontNode;
private:
	gocontext_ptr gc;
	std::unique_ptr<runtime> rt;
};

template <typename item_type>
struct subpipeline_virt: public subpipeline_base {
	progress_indicator_null pi;

	virtual void push(const item_type &) = 0;
};

template <typename item_type, typename fact_t>
struct subpipeline_impl: public subpipeline_virt<item_type> {
	typename fact_t::constructed_type front;
	
	subpipeline_impl(fact_t fact): front(fact.construct()) {
		this->m_nodeMap = front.get_node_map();
		this->frontNode = &front;
	}
	
	subpipeline_impl(const subpipeline_impl &) = delete;
	subpipeline_impl & operator=(const subpipeline_impl &) = delete;
	subpipeline_impl(subpipeline_impl &&) = delete;
	subpipeline_impl & operator=(subpipeline_impl &&) = delete;
	
	void push(const item_type & item) override {
		front.push(item);
	};
	
};
} //namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline
///
/// Container class for a subpipeline
///////////////////////////////////////////////////////////////////////////////
template <typename item_type>
struct subpipeline {
	subpipeline() {}
	subpipeline(subpipeline &&) = default;
	subpipeline(const subpipeline &) = default;
	subpipeline & operator=(subpipeline &&) = default;
	subpipeline & operator=(const subpipeline &) = default;
	subpipeline(const std::shared_ptr<bits::subpipeline_virt<item_type>> & p): p(p) {}
	
	template <typename T>
	subpipeline(T from) {
		*this = std::move(from);
	}

	template <typename T>
	subpipeline & operator=(T from) {
		p.reset(new bits::subpipeline_impl<item_type, T>(std::move(from)));
		return *this;
	}
	
	void push(const item_type & item) {p->push(item);}

	void begin(size_t filesAvailable, size_t memory) {
		p->begin(1, p->pi, filesAvailable, memory, nullptr, nullptr);
	}

	void begin(size_t memory) {
		begin(get_file_manager().available(), memory);
	}

	void begin(stream_size_type items, progress_indicator_base & pi,
			memory_size_type filesAvailable, memory_size_type mem,
			   const char * file, const char * function) {
		p->begin(items, pi, filesAvailable, mem, file, function);
	}

	void begin(stream_size_type items, progress_indicator_base & pi,
			   memory_size_type mem,
			   const char * file, const char * function) {
		begin(items, pi, get_file_manager().available(), mem, file, function);
	}

	void end() {p->end();}

	void plot(std::ostream & os = std::cout) {
		p->plot(os);
	}

	void plot_full(std::ostream & os = std::cout) {
		p->plot_full(os);
	}

	bits::node_map::ptr get_node_map() const {
		return p->get_node_map();
	}

	bool can_fetch(std::string key) {
		return p->can_fetch(key);
	}

	any_noncopyable fetch_any(std::string key) {
		return p->fetch_any(key);
	}

	template <typename T>
	T fetch(std::string key) {
		any_noncopyable a = fetch_any(key);
		return *any_cast<T>(&a);
	}

	void forward_any(std::string key, const any_noncopyable & value) {
		return p->forward_any(key, value);
	}

	template <typename T>
	void forward(std::string key, T value) {
		forward_any(key, any_noncopyable(value));
	}

	void output_memory(std::ostream & o) const {p->output_memory(o);}
private:
	std::shared_ptr<bits::subpipeline_virt<item_type>> p;
};

} //namespace pipelining
} //namespace tpie

#endif //__TPIE_PIPELINING_SUBPIPELINE_H__
