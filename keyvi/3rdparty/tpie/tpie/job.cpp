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
/// \file job.cpp Job methods and job manager.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/job.h>
#include <tpie/array.h>
#include <tpie/internal_queue.h>
#include <tpie/exception.h>
#include <functional>
#include <thread>
namespace tpie {
 
///////////////////////////////////////////////////////////////////////////////
/// Job manager singleton.
///////////////////////////////////////////////////////////////////////////////
class job_manager * the_job_manager = 0;

class job_manager {

public:

	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor.
	///////////////////////////////////////////////////////////////////////////
	job_manager() : m_jobs(128), m_kill_job_pool(false) {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Initialize the thread pool.
	///////////////////////////////////////////////////////////////////////////
	void init_pool(size_t threads) {
		m_thread_pool.resize(threads);
		for (size_t i = 0; i < threads; ++i) {
			std::function<void()> f(worker);
			std::thread t(f);
			// thread is move-constructible
			m_thread_pool[i].swap(t);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Notify all waiting workers, wait for them to quit.
	///////////////////////////////////////////////////////////////////////////
	void shutdown_pool() {
		std::unique_lock<std::mutex> lock(jobs_mutex);
		m_kill_job_pool = true;
		m_has_data.notify_all();
		lock.unlock();
		for (size_t i = 0; i < m_thread_pool.size(); ++i) {
			m_thread_pool[i].join();
		}
	}

private:

	tpie::internal_queue<tpie::job *> m_jobs;
	tpie::array<std::thread> m_thread_pool;

	///////////////////////////////////////////////////////////////////////////
	/// \brief The only mutex we will ever need.
	///////////////////////////////////////////////////////////////////////////
	std::mutex jobs_mutex;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Notified when a job is added to the queue.
	///////////////////////////////////////////////////////////////////////////
	std::condition_variable m_has_data;

	///////////////////////////////////////////////////////////////////////////
	/// \brief True when the workers should quit ASAP.
	///////////////////////////////////////////////////////////////////////////
	bool m_kill_job_pool;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Worker thread entry point.
	///////////////////////////////////////////////////////////////////////////
	static void worker() {
		for (;;) {
			std::unique_lock<std::mutex> lock(the_job_manager->jobs_mutex);
			while (the_job_manager->m_jobs.empty() && !the_job_manager->m_kill_job_pool) the_job_manager->m_has_data.wait(lock);
			if (the_job_manager->m_kill_job_pool) break;
			tpie::job * j = the_job_manager->m_jobs.front();
			the_job_manager->m_jobs.pop();
			lock.unlock();
			j->run();
		}
	};

	friend class tpie::job;
};

memory_size_type default_worker_count() {
	memory_size_type workers = std::thread::hardware_concurrency();
	if (workers > 3) --workers; // spare a CPU for the UI
	return workers;
}

void init_job() {
	the_job_manager = tpie_new<job_manager>();
	memory_size_type workers = default_worker_count();
	the_job_manager->init_pool(workers);
}

void finish_job() {
	the_job_manager->shutdown_pool();
	tpie_delete(the_job_manager);
	the_job_manager = 0;
}

job::job()
	: m_dependencies(0)
	, m_parent(0)
	, m_state(job_idle)
{
}

void job::join() {
	std::unique_lock<std::mutex> lock(the_job_manager->jobs_mutex);
	while (m_dependencies) {
		m_done.wait(lock);
	}
}

bool job::is_done() {
	std::lock_guard<std::mutex> lock(the_job_manager->jobs_mutex);
	return !m_dependencies;
}

void job::enqueue(job * parent) {
	if (m_state != job_idle)
		throw tpie::exception("Bad job state");

	m_state = job_enqueued;

	std::unique_lock<std::mutex> lock(the_job_manager->jobs_mutex);
	if (the_job_manager->m_kill_job_pool) throw job_manager_exception();
	m_parent = parent;
	m_dependencies = 1;
	if (m_parent) ++m_parent->m_dependencies;
	if (the_job_manager->m_jobs.full()) {
		lock.unlock();
		run();
		return;
	}
	the_job_manager->m_jobs.push(this);
	the_job_manager->m_has_data.notify_one();
}

void job::run() {
	if (m_state != job_enqueued)
		throw tpie::exception("Bad job state");

	m_state = job_running;

	(*this)();
	std::lock_guard<std::mutex> lock(the_job_manager->jobs_mutex);
	done();
}

void job::done() {
	if (m_state != job_running)
		throw tpie::exception("Bad job state");

	--m_dependencies;
	if (m_dependencies) return;

	m_state = job_idle;

	if (m_parent) m_parent->done();
	m_done.notify_all();
	on_done();
}

} // namespace tpie

