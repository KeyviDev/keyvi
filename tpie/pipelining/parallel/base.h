// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_PARALLEL_BASE_H__
#define __TPIE_PIPELINING_PARALLEL_BASE_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/factory_base.h>
#include <tpie/array_view.h>
#include <memory>
#include <tpie/pipelining/maintain_order_type.h>
#include <tpie/pipelining/parallel/options.h>
#include <tpie/pipelining/parallel/worker_state.h>
#include <tpie/pipelining/parallel/aligned_array.h>

namespace tpie {

namespace pipelining {

namespace parallel_bits {

// predeclare
template <typename T>
class before;
template <typename dest_t>
class before_impl;
template <typename T>
class after;
template <typename T1, typename T2>
class state;

///////////////////////////////////////////////////////////////////////////////
/// \brief Class containing an array of node instances. We cannot use
/// tpie::array or similar, since we need to construct the elements in a
/// special way. This class is non-copyable since it resides in the refcounted
/// state class.
/// \tparam fact_t  Type of factory constructing the worker
/// \tparam Output  Type of output items
///////////////////////////////////////////////////////////////////////////////
template <typename Input, typename Output>
class threads {
	typedef before<Input> before_t;

protected:
	static const size_t alignment = 64;

	/** Progress indicator type */
	typedef progress_indicator_null pi_t;
	aligned_array<pi_t, alignment> m_progressIndicators;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Factory hook that sets the progress indicator of the
	/// nodes run in parallel to the null progress indicator.
	/// This way, we can collect the number of steps in the main thread.
	///////////////////////////////////////////////////////////////////////////
	class progress_indicator_hook : public factory_init_hook {
		threads * t;
	public:
		progress_indicator_hook(threads * t)
			: t(t)
			, index(0)
		{
		}

		virtual void init_node(node & r) override {
			r.set_progress_indicator(t->m_progressIndicators.get(index));
		}

		size_t index;
	};

	friend class progress_indicator_hook;

	std::vector<before_t *> m_dests;

public:
	before_t & operator[](size_t idx) {
		return *m_dests[idx];
	}

	stream_size_type sum_steps() {
		stream_size_type res = 0;
		for (size_t i = 0; i < m_progressIndicators.size(); ++i) {
			res += m_progressIndicators.get(i)->get_current();
		}
		return res;
	}

	virtual ~threads() {}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Subclass of threads instantiating and managing the pipelines.
///////////////////////////////////////////////////////////////////////////////
template <typename Input, typename Output, typename fact_t>
class threads_impl : public threads<Input, Output> {
private:
	typedef threads<Input, Output> p_t;

	/** Progress indicator type */
	typedef typename p_t::pi_t pi_t;

	typedef after<Output> after_t;
	typedef typename fact_t::template constructed<after_t>::type worker_t;
	typedef typename push_type<worker_t>::type T1;
	typedef Output T2;
	typedef before_impl<worker_t> before_t;
	static const size_t alignment = p_t::alignment;
	typedef aligned_array<before_t, alignment> aligned_before_t;

	/** Size of the m_dests array. */
	size_t numJobs;

	/** Allocated array buffer. */
	aligned_before_t m_data;

public:
	threads_impl(fact_t && fact,
						state<T1, T2> & st)
		: numJobs(st.opts.numJobs)
	{
		typename p_t::progress_indicator_hook hook(this);
		fact.hook_initialization(&hook);
		fact.set_destination_kind_push();
		// uninitialized allocation
		m_data.realloc(numJobs);
		this->m_progressIndicators.realloc(numJobs);
		this->m_dests.resize(numJobs);

		// construct elements manually
		for (size_t i = 0; i < numJobs; ++i) {
			// for debugging: check that pointer is aligned.
			if (((size_t) m_data.get(i)) % alignment != 0) {
				log_warning() << "Thread " << i << " is not aligned: Address "
					<< m_data.get(i) << " is off by " <<
					(((size_t) m_data.get(i)) % alignment) << " bytes"
					<< std::endl;
			}

			hook.index = i;
			new (this->m_progressIndicators.get(i)) pi_t();

			auto n = fact.construct_copy(after_t(st, i));
			if (i == 0)
				n.set_plot_options(node::PLOT_PARALLEL);
			else
				n.set_plot_options(node::PLOT_PARALLEL | node::PLOT_SIMPLIFIED_HIDE);
			this->m_dests[i] =
				new(m_data.get(i))
				before_t(st, i, std::move(n));
		}
	}

	virtual ~threads_impl() {
		for (size_t i = 0; i < numJobs; ++i) {
			m_data.get(i)->~before_t();
			this->m_progressIndicators.get(i)->~pi_t();
		}
		m_data.realloc(0);
		this->m_progressIndicators.realloc(0);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Non-templated virtual base class of after.
///////////////////////////////////////////////////////////////////////////////
class after_base : public node {
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Called by before::worker to initialize buffers.
	///////////////////////////////////////////////////////////////////////////
	virtual void worker_initialize() = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Called by before::worker after a batch of items has
	/// been pushed.
	/// \param  complete  Whether the entire input has been processed.
	///////////////////////////////////////////////////////////////////////////
	virtual void flush_buffer() = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  For internal use in order to construct the pipeline graph.
	///////////////////////////////////////////////////////////////////////////
	virtual void set_consumer(node *) = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Common state in parallel pipelining library.
/// This class is instantiated once and kept in a std::shared_ptr, and it is
/// not copy constructible.
///
/// Unless noted otherwise, a thread must own the state mutex to access other
/// parts of this instance.
///////////////////////////////////////////////////////////////////////////////
class state_base {
public:
	typedef std::mutex mutex_t;
	typedef std::condition_variable cond_t;
	typedef std::unique_lock<std::mutex> lock_t;

	const options opts;

	/** Single mutex. */
	mutex_t mutex;

	/** Condition variable.
	 *
	 * Who waits: The producer, with the single mutex (waits until at least one
	 * worker has state = IDLE or state = OUTPUTTING).
	 *
	 * Who signals: The par_after, when a worker is OUTPUTTING. */
	cond_t producerCond;

	/** Condition variable, one per worker.
	 *
	 * Who waits: The worker's par_before when waiting for input (wait for
	 * state = PROCESSING), the worker's par_after when waiting for output to
	 * be read (wait for state = IDLE). Waits with the single mutex.
	 *
	 * Who signals: par_producer, when input has been written (sets state to PROCESSING).
	 * par_consumer, when output has been read (sets state to IDLE).
	 */
	cond_t * workerCond;

	/** Shared state, must have mutex to write. */
	size_t runningWorkers;

	/// Must not be used concurrently.
	void set_input_ptr(size_t idx, node * v) {
		m_inputs[idx] = v;
	}

	/// Must not be used concurrently.
	void set_output_ptr(size_t idx, after_base * v) {
		m_outputs[idx] = v;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get the specified before instance.
	///
	/// Enables easy construction of the pipeline graph at runtime.
	///
	/// Shared state, must have mutex to use.
	///////////////////////////////////////////////////////////////////////////
	node & input(size_t idx) { return *m_inputs[idx]; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get the specified after instance.
	///
	/// Serves two purposes:
	/// First, it enables easy construction of the pipeline graph at runtime.
	/// Second, it is used by before to send batch signals to
	/// after.
	///
	/// Shared state, must have mutex to use.
	///////////////////////////////////////////////////////////////////////////
	after_base & output(size_t idx) { return *m_outputs[idx]; }

	/// Shared state, must have mutex to use.
	worker_state get_state(size_t idx) {
		return m_states[idx];
	}

	/// Shared state, must have mutex to use.
	void transition_state(size_t idx, worker_state from, worker_state to) {
		if (m_states[idx] != from) {
			std::stringstream ss;
			ss << idx << " Invalid state transition " << from << " -> " << to << "; current state is " << m_states[idx];
			log_error() << ss.str() << std::endl;
			throw exception(ss.str());
		}
		m_states[idx] = to;
	}

protected:
	std::vector<node *> m_inputs;
	std::vector<after_base *> m_outputs;
	std::vector<worker_state> m_states;

	state_base(const options opts)
		: opts(opts)
		, runningWorkers(0)
		, m_inputs(opts.numJobs, 0)
		, m_outputs(opts.numJobs, 0)
		, m_states(opts.numJobs, INITIALIZING)
	{
		workerCond = new cond_t[opts.numJobs];
	}

	virtual ~state_base() {
		delete[] workerCond;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Instantiated in each thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class parallel_input_buffer {
	memory_size_type m_inputSize;
	array<T> m_inputBuffer;

public:
	array_view<T> get_input() {
		return array_view<T>(&m_inputBuffer[0], m_inputSize);
	}

	void set_input(array_view<T> input) {
		if (input.size() > m_inputBuffer.size())
			throw tpie::exception(m_inputBuffer.size() ? "Input too large" : "Input buffer not initialized");

		memory_size_type items =
			std::copy(input.begin(), input.end(), m_inputBuffer.begin())
			-m_inputBuffer.begin();

		m_inputSize = items;
	}

	parallel_input_buffer(const options & opts)
		: m_inputSize(0)
		, m_inputBuffer(opts.bufSize)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Instantiated in each thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class parallel_output_buffer {
	memory_size_type m_outputSize;
	array<T> m_outputBuffer;
	friend class after<T>;

public:
	array_view<T> get_output() {
		return array_view<T>(&m_outputBuffer[0], m_outputSize);
	}

	parallel_output_buffer(const options & opts)
		: m_outputSize(0)
		, m_outputBuffer(opts.bufSize)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Node running in main thread, accepting an output buffer
/// from the managing producer and forwards them down the pipe. The overhead
/// concerned with switching threads dominates the overhead of a virtual method
/// call, so this class only depends on the output type and leaves the pushing
/// of items to a virtual subclass.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class consumer : public node {
public:
	typedef T item_type;

	virtual void consume(array_view<T>) = 0;
	// node has virtual dtor
};

///////////////////////////////////////////////////////////////////////////////
/// \brief State subclass containing the item type specific state, i.e. the
/// input/output buffers and the concrete pipes.
///////////////////////////////////////////////////////////////////////////////
template <typename T1, typename T2>
class state : public state_base {
public:
	typedef std::shared_ptr<state> ptr;
	typedef state_base::mutex_t mutex_t;
	typedef state_base::cond_t cond_t;
	typedef state_base::lock_t lock_t;

	array<parallel_input_buffer<T1> *> m_inputBuffers;
	array<parallel_output_buffer<T2> *> m_outputBuffers;

	consumer<T2> * m_cons;

	std::unique_ptr<threads<T1, T2> > pipes;

	template <typename fact_t>
	state(const options opts, fact_t && fact)
		: state_base(opts)
		, m_inputBuffers(opts.numJobs)
		, m_outputBuffers(opts.numJobs)
		, m_cons(0)
	{
		typedef threads_impl<T1, T2, fact_t> pipes_impl_t;
		pipes.reset(new pipes_impl_t(std::move(fact), *this));
	}

	void set_consumer_ptr(consumer<T2> * cons) {
		m_cons = cons;
	}

	consumer<T2> * const * get_consumer_ptr_ptr() const {
		return &m_cons;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Accepts output items and sends them to the main thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class after : public after_base {
protected:
	state_base & st;
	size_t parId;
	std::unique_ptr<parallel_output_buffer<T> > m_buffer;
	array<parallel_output_buffer<T> *> & m_outputBuffers;
	typedef state_base::lock_t lock_t;
	consumer<T> * const * m_cons;

public:
	typedef T item_type;

	template <typename Input>
	after(state<Input, T> & state,
				   size_t parId)
		: st(state)
		, parId(parId)
		, m_outputBuffers(state.m_outputBuffers)
		, m_cons(state.get_consumer_ptr_ptr())
	{
		state.set_output_ptr(parId, this);
		set_name("Parallel after", PRIORITY_INSIGNIFICANT);
		set_plot_options(PLOT_PARALLEL | PLOT_SIMPLIFIED_HIDE);
		if (m_cons == 0) throw tpie::exception("Unexpected nullptr");
		if (*m_cons != 0) throw tpie::exception("Expected nullptr");
	}

	virtual void set_consumer(node * cons) override {
		this->add_push_destination(*cons);
	}

	after(after && other)
		: after_base(std::move(other))
		, st(other.st)
		, parId(std::move(other.parId))
		, m_outputBuffers(other.m_outputBuffers)
		, m_cons(std::move(other.m_cons)) {
		st.set_output_ptr(parId, this);
		if (m_cons == 0) throw tpie::exception("Unexpected nullptr in move");
		if (*m_cons != 0) throw tpie::exception("Expected nullptr in move");
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push to thread-local buffer; flush it when full.
	///////////////////////////////////////////////////////////////////////////
	void push(const T & item) {
		if (m_buffer->m_outputSize >= m_buffer->m_outputBuffer.size())
			flush_buffer_impl(false);

		m_buffer->m_outputBuffer[m_buffer->m_outputSize++] = item;
	}

	virtual void end() override {
		flush_buffer_impl(true);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Invoked by before::worker (in worker thread context).
	///////////////////////////////////////////////////////////////////////////
	virtual void worker_initialize() override {
		m_buffer.reset(new parallel_output_buffer<T>(st.opts));
		m_outputBuffers[parId] = m_buffer.get();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Invoked by before::push_all when all input items have been
	/// pushed.
	///////////////////////////////////////////////////////////////////////////
	virtual void flush_buffer() override {
		flush_buffer_impl(true);
	}

private:
	bool is_done() const {
		switch (st.get_state(parId)) {
			case INITIALIZING:
				throw tpie::exception("INITIALIZING not expected in after::is_done");
			case IDLE:
				return true;
			case PROCESSING:
				// The main thread may transition us from Outputting to Idle to
				// Processing without us noticing, or it may transition us from
				// Partial_Output to Processing. In either case, we are done
				// flushing the buffer.
				return true;
			case PARTIAL_OUTPUT:
			case OUTPUTTING:
				return false;
			case DONE:
				return true;
		}
		throw tpie::exception("Unknown state");
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Send a signal to the main thread that the output buffer must
	/// be emptied.
	///
	/// If this is in response to a full output buffer before the input buffer
	/// has been processed, `complete == false`. In this case, we transition
	/// to PARTIAL_OUTPUT and the main thread will transition us to PROCESSING
	/// after consuming our output buffer.
	///
	/// If this is in response to an empty input buffer, `complete == true`,
	/// and we transition to OUTPUTTING and the main thread will transition us
	/// to IDLE.
	///
	/// \param  complete  Whether the entire input has been processed.
	///////////////////////////////////////////////////////////////////////////
	void flush_buffer_impl(bool complete) {
		// At this point, we could check if the output buffer is empty and
		// short-circuit when it is without acquiring the lock; however, we
		// must do a full PROCESSING -> OUTPUTTING -> IDLE transition in this
		// case to let the main thread know that we are done processing the
		// input.

		lock_t lock(st.mutex);
		if (st.get_state(parId) == DONE) {
			if (*m_cons == 0) throw tpie::exception("Unexpected nullptr in flush_buffer");
			array_view<T> out = m_buffer->get_output();
			(*m_cons)->consume(out);
		} else {
			st.transition_state(parId, PROCESSING, complete ? OUTPUTTING : PARTIAL_OUTPUT);
			// notify producer that output is ready
			st.producerCond.notify_one();
			while (!is_done()) {
				st.workerCond[parId].wait(lock);
			}
		}
		m_buffer->m_outputSize = 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Accepts input items from the main thread and sends them down the
/// pipeline. This class contains the bulk of the code that is run in each
/// worker thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class before : public node {
protected:
	state_base & st;
	size_t parId;
	std::unique_ptr<parallel_input_buffer<T> > m_buffer;
	array<parallel_input_buffer<T> *> & m_inputBuffers;
	std::thread m_worker;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Overridden in subclass to push a buffer of items.
	///////////////////////////////////////////////////////////////////////////
	virtual void push_all(array_view<T> items) = 0;

	template <typename Output>
	before(state<T, Output> & st, size_t parId)
		: st(st)
		, parId(parId)
		, m_inputBuffers(st.m_inputBuffers)
	{
		set_name("Parallel before", PRIORITY_INSIGNIFICANT);
		set_plot_options(PLOT_PARALLEL | PLOT_SIMPLIFIED_HIDE);
	}
	// virtual dtor in node

	before(const before & other)
		: st(other.st)
		, parId(other.parId)
		, m_inputBuffers(other.m_inputBuffers)
	{
	}

	~before() {
		m_worker.join();
	}

public:
	typedef T item_type;

	virtual void begin() override {
		node::begin();
		std::thread t(run_worker, this);
		m_worker.swap(t);
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if we are ready to process a batch of input.
	///////////////////////////////////////////////////////////////////////////
	bool ready() {
		switch (st.get_state(parId)) {
			case INITIALIZING:
				throw tpie::exception("INITIALIZING not expected in before::ready");
			case IDLE:
				return false;
			case PROCESSING:
				return true;
			case PARTIAL_OUTPUT:
				throw tpie::exception("State 'partial_output' was not expected in before::ready");
			case OUTPUTTING:
				throw tpie::exception("State 'outputting' was not expected in before::ready");
			case DONE:
				return false;
		}
		throw tpie::exception("Unknown state");
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Class providing RAII-style bookkeeping of number of workers.
	///////////////////////////////////////////////////////////////////////////
	class running_signal {
		typedef state_base::cond_t cond_t;
		memory_size_type & sig;
		cond_t & producerCond;
	public:
		running_signal(memory_size_type & sig, cond_t & producerCond)
			: sig(sig)
			, producerCond(producerCond)
		{
			++sig;
			producerCond.notify_one();
		}

		~running_signal() {
			--sig;
			producerCond.notify_one();
		}
	};

	static void run_worker(before * self) {
		self->worker();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Worker thread entry point.
	///////////////////////////////////////////////////////////////////////////
	void worker() {
		state_base::lock_t lock(st.mutex);

		m_buffer.reset(new parallel_input_buffer<T>(st.opts));
		m_inputBuffers[parId] = m_buffer.get();

		// virtual invocation
		st.output(parId).worker_initialize();

		st.transition_state(parId, INITIALIZING, IDLE);
		running_signal _(st.runningWorkers, st.producerCond);
		while (true) {
			// wait for transition IDLE -> PROCESSING
			while (!ready()) {
				if (st.get_state(parId) == DONE) {
					return;
				}
				st.workerCond[parId].wait(lock);
			}
			lock.unlock();

			// virtual invocation
			push_all(m_buffer->get_input());

			lock.lock();
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Concrete before class.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class before_impl : public before<typename push_type<dest_t>::type> {
	typedef typename push_type<dest_t>::type item_type;

	dest_t dest;

public:
	template <typename Output>
	before_impl(state<item_type, Output> & st,
						 size_t parId,
						 dest_t dest)
		: before<item_type>(st, parId)
		, dest(std::move(dest))
	{
		this->add_push_destination(dest);
		st.set_input_ptr(parId, this);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push all items from buffer and flush output buffer afterwards.
	///
	/// If pipeline is one-to-one, that is, one item output for each item
	/// input, then the flush at the end is not needed.
	///////////////////////////////////////////////////////////////////////////
	virtual void push_all(array_view<item_type> items) {
		for (size_t i = 0; i < items.size(); ++i) {
			dest.push(items[i]);
		}

		// virtual invocation
		this->st.output(this->parId).flush_buffer();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Concrete consumer implementation.
///////////////////////////////////////////////////////////////////////////////
template <typename Input, typename Output, typename dest_t>
class consumer_impl : public consumer<typename push_type<dest_t>::type> {
	typedef state<Input, Output> state_t;
	typedef typename state_t::ptr stateptr;
	dest_t dest;
	stateptr st;
public:
	typedef typename push_type<dest_t>::type item_type;

	consumer_impl(dest_t dest, stateptr st)
		: dest(std::move(dest))
		, st(st)
	{
		this->add_push_destination(dest);
		this->set_name("Parallel output", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(node::PLOT_PARALLEL | node::PLOT_SIMPLIFIED_HIDE);
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			st->output(i).set_consumer(this);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push all items from output buffer to the rest of the pipeline.
	///////////////////////////////////////////////////////////////////////////
	virtual void consume(array_view<item_type> a) override {
		for (size_t i = 0; i < a.size(); ++i) {
			dest.push(a[i]);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Producer, running in main thread, managing the parallel execution.
///
/// This class contains the bulk of the code that is run in the main thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T1, typename T2>
class producer : public node {
public:
	typedef T1 item_type;

private:
	typedef state<T1, T2> state_t;
	typedef typename state_t::ptr stateptr;
	stateptr st;
	array<T1> inputBuffer;
	size_t written;
	size_t readyIdx;
	std::shared_ptr<consumer<T2> > cons;
	internal_queue<memory_size_type> m_outputOrder;
	stream_size_type m_steps;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if a worker is waiting for the main thread.
	///
	/// A worker may wait for output to be fetched, or it may wait for input to
	/// be sent. If there is a worker waiting, this function returns true and
	/// sets the index of the waiting worker in this->readyIdx.
	///////////////////////////////////////////////////////////////////////////
	bool has_ready_pipe() {
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			switch (st->get_state(i)) {
				case INITIALIZING:
				case PROCESSING:
					break;
				case PARTIAL_OUTPUT:
				case OUTPUTTING:
					// If we have to maintain order of items, the only
					// outputting worker we consider to be waiting is the
					// "front worker".
					if (st->opts.maintainOrder && m_outputOrder.front() != i)
						break;
					// fallthrough
				case IDLE:
					readyIdx = i;
					return true;
				case DONE:
					throw tpie::exception("State DONE not expected in has_ready_pipe().");
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if a worker is waiting for the main thread to process its
	/// output.
	///
	/// This is used in end() instead of has_ready_pipe, since we do not care
	/// about workers waiting for input when we don't have any input to send.
	///
	/// Like has_ready_pipe, this function sets this->readyIdx if and only if
	/// it returns true.
	///////////////////////////////////////////////////////////////////////////
	bool has_outputting_pipe() {
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			switch (st->get_state(i)) {
				case INITIALIZING:
				case IDLE:
				case PROCESSING:
					break;
				case PARTIAL_OUTPUT:
				case OUTPUTTING:
					if (st->opts.maintainOrder && m_outputOrder.front() != i)
						break;
					readyIdx = i;
					return true;
				case DONE:
					throw tpie::exception("State DONE not expected in has_outputting_pipe().");
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if a worker is waiting for the main thread to process its
	/// output.
	///
	/// This is used in end() when we are waiting for workers to finish up.
	/// When no worker is outputting and no worker is processing, all items
	/// have been processed.
	///
	/// Does not modify this->readyIdx.
	///////////////////////////////////////////////////////////////////////////
	bool has_processing_pipe() {
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			switch (st->get_state(i)) {
				case INITIALIZING:
				case IDLE:
				case PARTIAL_OUTPUT:
				case OUTPUTTING:
					break;
				case PROCESSING:
					return true;
				case DONE:
					throw tpie::exception("State DONE not expected in has_processing_pipe().");
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Propagate progress information.
	///////////////////////////////////////////////////////////////////////////
	void flush_steps() {
		// The number of items has been forwarded along unchanged to all
		// the workers (it is still a valid upper bound).
		//
		// This means the workers each expect to handle all the items,
		// which means the number of steps reported in total is scaled up
		// by the number of workers.
		//
		// Therefore, we similarly scale up the number of times we call step.
		// In effect, every time step() is called once in a single worker,
		// we process this as if all workers called step().

		stream_size_type steps = st->pipes->sum_steps();
		if (steps != m_steps) {
			this->get_progress_indicator()->step(st->opts.numJobs*(steps - m_steps));
			m_steps = steps;
		}
	}

public:
	template <typename consumer_t>
	producer(stateptr st, consumer_t cons)
		: st(st)
		, written(0)
		, cons(new consumer_t(std::move(cons)))
		, m_steps(0)
	{
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			this->add_push_destination(st->input(i));
		}
		this->set_name("Parallel input", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(PLOT_PARALLEL | PLOT_SIMPLIFIED_HIDE);

		memory_size_type usage =
			st->opts.numJobs * st->opts.bufSize * (sizeof(T1) + sizeof(T2)) // workers
			+ st->opts.bufSize * sizeof(item_type) // our buffer
			;
		this->set_minimum_memory(usage);

		if (st->opts.maintainOrder) {
			m_outputOrder.resize(st->opts.numJobs);
		}
	}

	virtual void begin() override {
		inputBuffer.resize(st->opts.bufSize);

		state_base::lock_t lock(st->mutex);
		while (st->runningWorkers != st->opts.numJobs) {
			st->producerCond.wait(lock);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Accumulate input buffer and send off to workers.
	///
	/// Since the parallel producer and parallel consumer run single-threaded
	/// in the main thread, producer::push is our only opportunity to have the
	/// consumer call push on its destination. Thus, when we accumulate an
	/// input buffer, before sending it off to a worker, we might want to have
	/// the consumer consume an output buffer to free up a parallel worker.
	///////////////////////////////////////////////////////////////////////////
	void push(item_type item) {
		inputBuffer[written++] = item;
		if (written < st->opts.bufSize) {
			// Wait for more items before doing anything expensive such as
			// locking.
			return;
		}
		state_base::lock_t lock(st->mutex);

		flush_steps();

		empty_input_buffer(lock);
	}

private:
	void empty_input_buffer(state_base::lock_t & lock) {
		while (written > 0) {
			while (!has_ready_pipe()) {
				st->producerCond.wait(lock);
			}
			switch (st->get_state(readyIdx)) {
				case INITIALIZING:
					throw tpie::exception("State 'INITIALIZING' not expected at this point");
				case IDLE:
				{
					// Send buffer to ready worker
					item_type * first = &inputBuffer[0];
					item_type * last = first + written;
					parallel_input_buffer<T1> & dest = *st->m_inputBuffers[readyIdx];
					dest.set_input(array_view<T1>(first, last));
					st->transition_state(readyIdx, IDLE, PROCESSING);
					st->workerCond[readyIdx].notify_one();
					written = 0;
					if (st->opts.maintainOrder)
						m_outputOrder.push(readyIdx);
					break;
				}
				case PROCESSING:
					throw tpie::exception("State 'processing' not expected at this point");
				case PARTIAL_OUTPUT:
					// Receive buffer (virtual invocation)
					cons->consume(st->m_outputBuffers[readyIdx]->get_output());
					st->transition_state(readyIdx, PARTIAL_OUTPUT, PROCESSING);
					st->workerCond[readyIdx].notify_one();
					break;
				case OUTPUTTING:
					// Receive buffer (virtual invocation)
					cons->consume(st->m_outputBuffers[readyIdx]->get_output());

					st->transition_state(readyIdx, OUTPUTTING, IDLE);
					st->workerCond[readyIdx].notify_one();
					if (st->opts.maintainOrder) {
						if (m_outputOrder.front() != readyIdx) {
							log_error() << "Producer: Expected " << readyIdx << " in front; got "
								<< m_outputOrder.front() << std::endl;
							throw tpie::exception("Producer got wrong entry from has_ready_pipe");
						}
						m_outputOrder.pop();
					}
					break;
				case DONE:
					throw tpie::exception("State 'DONE' not expected at this point");
			}
		}
	}

public:
	virtual void end() override {
		state_base::lock_t lock(st->mutex);

		flush_steps();

		empty_input_buffer(lock);

		inputBuffer.resize(0);

		st->set_consumer_ptr(cons.get());

		bool done = false;
		while (!done) {
			while (!has_outputting_pipe()) {
				if (!has_processing_pipe()) {
					done = true;
					break;
				}
				// All items pushed; wait for processors to complete
				st->producerCond.wait(lock);
			}
			if (done) break;

			// virtual invocation
			cons->consume(st->m_outputBuffers[readyIdx]->get_output());

			if (st->get_state(readyIdx) == PARTIAL_OUTPUT) {
				st->transition_state(readyIdx, PARTIAL_OUTPUT, PROCESSING);
				st->workerCond[readyIdx].notify_one();
				continue;
			}
			st->transition_state(readyIdx, OUTPUTTING, IDLE);
			if (st->opts.maintainOrder) {
				if (m_outputOrder.front() != readyIdx) {
					log_error() << "Producer: Expected " << readyIdx << " in front; got "
						<< m_outputOrder.front() << std::endl;
					throw tpie::exception("Producer got wrong entry from has_ready_pipe");
				}
				m_outputOrder.pop();
			}
		}
		// Notify all workers that all processing is done
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			st->transition_state(i, IDLE, DONE);
			st->workerCond[i].notify_one();
		}
		while (st->runningWorkers > 0) {
			st->producerCond.wait(lock);
		}
		// All workers terminated

		flush_steps();
	}
};

} // namespace parallel_bits

} // namespace pipelining

} // namespace tpie

#endif
