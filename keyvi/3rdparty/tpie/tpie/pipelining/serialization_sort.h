// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#ifndef TPIE_PIPELINING_SERIALIZATION_SORT_H
#define TPIE_PIPELINING_SERIALIZATION_SORT_H

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_base.h>
#include <tpie/serialization_sorter.h>
#include <tpie/serialization.h>

namespace tpie {

namespace pipelining {

namespace serialization_bits {

template <typename T, typename pred_t>
class sorter_traits {
public:
	typedef T item_type;
	typedef pred_t pred_type;
	typedef serialization_sorter<item_type, pred_type> sorter_t;
	typedef std::shared_ptr<sorter_t> sorterptr;
};

template <typename Traits>
class sort_calc_t;

template <typename Traits>
class sort_input_t;

template <typename Traits>
class sort_output_base : public node {
	typedef typename Traits::pred_type pred_type;
public:
	/** Type of items sorted. */
	typedef typename Traits::item_type item_type;
	/** Type of the merge sort implementation used. */
	typedef typename Traits::sorter_t sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename Traits::sorterptr sorterptr;

	sorterptr get_sorter() const {
		return m_sorter;
	}

	void set_calc_node(node & calc) {
		add_memory_share_dependency(calc);
	}

	virtual void propagate() override {
		set_steps(m_sorter->item_count());
		forward("items", static_cast<stream_size_type>(m_sorter->item_count()));
		memory_size_type memory_usage = m_sorter->actual_memory_phase_3();
		set_minimum_memory(memory_usage);
		set_maximum_memory(memory_usage);
		set_memory_fraction(0);
		m_propagate_called = true;
	}

	void begin() override {
		m_sorter->set_owner(this);
	}

	void end() override {
		m_sorter->set_owner(nullptr);
		m_sorter.reset();
	}

	void add_calc_dependency(node_token tkn) {
		add_memory_share_dependency(tkn);
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) override {
		if (!m_propagate_called)
			m_sorter->set_phase_3_memory(availableMemory);
	}

	sort_output_base(pred_type pred)
		: m_sorter(new sorter_t(sizeof(item_type), pred))
		, m_propagate_called(false)
	{
	}

	sort_output_base(sorterptr p)
		: m_sorter(p)
		, m_propagate_called(false)
	{
	}

	sorterptr m_sorter;
	bool m_propagate_called;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter pull output node.
///////////////////////////////////////////////////////////////////////////////
template <typename Traits>
class sort_pull_output_t : public sort_output_base<Traits> {
public:
	typedef typename Traits::item_type item_type;
	typedef typename Traits::pred_type pred_type;
	typedef typename Traits::sorter_t sorter_t;
	typedef typename Traits::sorterptr sorterptr;

	sort_pull_output_t(sorterptr sorter)
		: sort_output_base<Traits>(sorter)
	{
		this->set_minimum_memory(sorter_t::minimum_memory_phase_3());
		this->set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
		this->set_memory_fraction(1.0);
	}

	inline bool can_pull() const {
		return this->m_sorter->can_pull();
	}

	inline item_type pull() {
		this->step();
		return this->m_sorter->pull();
	}

	// Despite this go() implementation, a sort_pull_output_t CANNOT be used as
	// an initiator node. Normally, it is a type error to have a phase without
	// an initiator, but with a passive_sorter you can circumvent this
	// mechanism. Thus we customize the error message printed (but throw the
	// same type of exception.)
	virtual void go() override {
		log_warning() << "Passive sorter used without an initiator in the final merge and output phase.\n"
			<< "Define an initiator and pair it up with the pipe from passive_sorter::output()." << std::endl;
		throw not_initiator_node();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter push output node.
///////////////////////////////////////////////////////////////////////////////
template <typename Traits, typename dest_t>
class sort_output_t : public sort_output_base<Traits> {
	typedef typename Traits::pred_type pred_type;
public:
	typedef typename Traits::item_type item_type;
	typedef sort_output_base<Traits> p_t;
	typedef typename Traits::sorter_t sorter_t;
	typedef typename Traits::sorterptr sorterptr;

	sort_output_t(dest_t dest, pred_type pred)
		: p_t(pred)
		, dest(std::move(dest))
	{
		this->add_push_destination(dest);
		this->set_minimum_memory(sorter_t::minimum_memory_phase_3());
		this->set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
		this->set_memory_fraction(1.0);
	}

	virtual void go() override {
		while (this->m_sorter->can_pull()) {
			dest.push(this->m_sorter->pull());
			this->step();
		}
	}
private:
	dest_t dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter middle node.
/// \tparam T        The type of items sorted
/// \tparam pred_t   The less-than predicate
///////////////////////////////////////////////////////////////////////////////
template <typename Traits>
class sort_calc_t : public node {
public:
	typedef typename Traits::item_type item_type;
	typedef typename Traits::sorter_t sorter_t;
	typedef typename Traits::sorterptr sorterptr;

	typedef sort_output_base<Traits> Output;

	sort_calc_t(sort_calc_t && other) = default;

	template <typename dest_t>
	sort_calc_t(dest_t dest)
		: dest(new dest_t(std::move(dest)))
		{
		m_sorter = this->dest->get_sorter();
		this->dest->add_calc_dependency(this->get_token());
		init();
	}

	sort_calc_t(sorterptr sorter, node_token tkn)
		: node(tkn), m_sorter(sorter)
	{
		init();
	}

	void init() {
		set_minimum_memory(sorter_t::minimum_memory_phase_2());
		set_name("Perform merge heap", PRIORITY_SIGNIFICANT);
		set_memory_fraction(1.0);
		m_propagate_called = false;
	}

	virtual void propagate() override {
		set_steps(1000);
		m_propagate_called = true;
	}

	void begin() override {
		m_sorter->set_owner(this);
	}

	virtual bool is_go_free() const override {return m_sorter->is_merge_runs_free();}

	virtual void go() override {
		progress_indicator_base * pi = proxy_progress_indicator();
		log_debug() << "TODO: Progress information during merging." << std::endl;
		m_sorter->merge_runs();
		pi->init(1);
		pi->step();
		pi->done();
	}

	void end() override {
		m_weak_sorter = m_sorter;
		m_sorter.reset();
	}

	virtual bool can_evacuate() override {
		return true;
	}

	virtual void evacuate() override {
		auto p = m_weak_sorter.lock();
		if (p) p->evacuate();
	}

	sorterptr get_sorter() const {
		return m_sorter;
	}

	void set_input_node(node & input) {
		add_memory_share_dependency(input);
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) override {
		if (!m_propagate_called)
			m_sorter->set_phase_2_memory(availableMemory);
	}

private:
	sorterptr m_sorter;
	std::weak_ptr<typename sorterptr::element_type> m_weak_sorter;
	bool m_propagate_called;
	std::shared_ptr<Output> dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter input node.
/// \tparam T        The type of items sorted
/// \tparam pred_t   The less-than predicate
///////////////////////////////////////////////////////////////////////////////
template <typename Traits>
class sort_input_t : public node {
	typedef typename Traits::pred_type pred_type;
public:
	typedef typename Traits::item_type item_type;
	typedef typename Traits::sorter_t sorter_t;
	typedef typename Traits::sorterptr sorterptr;

	sort_input_t(sort_calc_t<Traits> dest)
		: m_sorter(dest.get_sorter())
		, dest(std::move(dest))
	{
		this->dest.set_input_node(*this);
		set_minimum_memory(sorter_t::minimum_memory_phase_1());
		set_name("Form input runs", PRIORITY_SIGNIFICANT);
		set_memory_fraction(1.0);
		m_propagate_called = false;
	}

	virtual void propagate() override {
		m_propagate_called = true;
	}

	virtual void begin() override {
		m_sorter->set_owner(this);
		m_sorter->begin();
	}

	void push(const item_type & item) {
		m_sorter->push(item);
	}

	virtual void end() override {
		m_sorter->end();
		m_weak_sorter = m_sorter;
		m_sorter.reset();
	}

	virtual bool can_evacuate() override {
		return true;
	}

	virtual void evacuate() override {
		auto p = m_weak_sorter.lock();
		if (p) p->evacuate();
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) override {
		if (!m_propagate_called)
			m_sorter->set_phase_1_memory(availableMemory);
	}

private:
	sorterptr m_sorter;
	std::weak_ptr<typename sorterptr::element_type> m_weak_sorter;
	sort_calc_t<Traits> dest;
	bool m_propagate_called;
};

template <typename child_t>
class sort_factory_base : public factory_base {
	const child_t & self() const { return *static_cast<const child_t *>(this); }
public:
	template <typename dest_t>
	struct constructed {
	private:
		/** Type of items sorted. */
		typedef typename push_type<dest_t>::type item_type;
	public:
		typedef typename child_t::template predicate<item_type>::type pred_type;
		typedef sorter_traits<item_type, pred_type> Traits;
		typedef sort_input_t<Traits> type;
	};

	template <typename dest_t>
	typename constructed<dest_t>::type construct(dest_t dest) {
		typedef typename push_type<dest_t>::type item_type;
		typedef typename constructed<dest_t>::Traits Traits;

		sort_output_t<Traits, dest_t> output(std::move(dest), self().template get_pred<item_type>());
		this->init_sub_node(output);
		sort_calc_t<Traits> calc(std::move(output));
		this->init_sub_node(calc);
		sort_input_t<Traits> input(std::move(calc));
		this->init_sub_node(input);

		return input;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort factory using std::less<T> as comparator.
///////////////////////////////////////////////////////////////////////////////
class default_pred_sort_factory : public sort_factory_base<default_pred_sort_factory> {
public:
	template <typename item_type>
	class predicate {
	public:
		typedef std::less<item_type> type;
	};

	template <typename T>
	std::less<T> get_pred() const {
		return std::less<T>();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort factory using the given predicate as comparator.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t>
class sort_factory : public sort_factory_base<sort_factory<pred_t> > {
public:
	template <typename Dummy>
	class predicate {
	public:
		typedef pred_t type;
	};

	sort_factory(const pred_t & p)
		: pred(p)
	{
	}

	template <typename T>
	pred_t get_pred() const {
		return pred;
	}

private:
	pred_t pred;
};

} // namespace serialization_bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining sorter using std::less.
///////////////////////////////////////////////////////////////////////////////
inline pipe_middle<serialization_bits::default_pred_sort_factory>
serialization_sort() {
	typedef serialization_bits::default_pred_sort_factory fact;
	return pipe_middle<fact>(fact()).name("Sort");
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining sorter using the given predicate.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t>
pipe_middle<serialization_bits::sort_factory<pred_t> >
serialization_sort(const pred_t & p) {
	typedef serialization_bits::sort_factory<pred_t> fact;
	return pipe_middle<fact>(fact(p)).name("Sort");
}

template <typename T, typename pred_t=std::less<T> >
class serialization_passive_sorter;

namespace serialization_bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief Factory for the passive sorter input node.
///////////////////////////////////////////////////////////////////////////////
template <typename Traits>
class passive_sorter_factory_input : public factory_base {
public:
	typedef sort_pull_output_t<Traits> output_t;
	typedef sort_calc_t<Traits> calc_t;
	typedef sort_input_t<Traits> input_t;
	typedef input_t constructed_type;
	typedef typename Traits::sorter_t sorter_t;
	typedef typename Traits::sorterptr sorterptr;

	passive_sorter_factory_input(sorterptr sorter, node_token calc_token)
		: m_sorter(sorter)
		, m_calc_token(calc_token) {}

	constructed_type construct() {
		calc_t calc(std::move(m_sorter), m_calc_token);
		this->init_node(calc);
		input_t input(std::move(calc));
		this->init_node(input);
		return std::move(input);
	}

private:
	sorterptr m_sorter;
	node_token m_calc_token;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Factory for the passive sorter output node.
///////////////////////////////////////////////////////////////////////////////
template <typename Traits>
class passive_sorter_factory_output : public factory_base {
public:
	typedef sort_pull_output_t<Traits> output_t;
	typedef typename Traits::sorterptr sorterptr;
	typedef output_t constructed_type;

	passive_sorter_factory_output(sorterptr sorter, node_token calc_token)
		: m_sorter(sorter)
		, m_calc_token(calc_token)
		{}

	constructed_type construct() {
		constructed_type res(std::move(m_sorter));
		res.add_calc_dependency(m_calc_token);
		init_node(res);
		return std::move(res);
	}

private:
	sorterptr m_sorter;
	node_token m_calc_token;
};

} // namespace serialization_bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelined sorter with push input and pull output.
/// Get the input pipe with \c input() and the output pullpipe with \c output().
/// input() must not be called after output().
/// \tparam T The type of item to sort
/// \tparam pred_t The predicate (e.g. std::less<T>) indicating the predicate
/// on which to order an item before another.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t>
class serialization_passive_sorter {
	typedef serialization_bits::sorter_traits<T, pred_t> Traits;
public:
	/** Type of items sorted. */
	typedef T item_type;
	/** Type of the merge sort implementation used. */
	typedef typename Traits::sorter_t sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename Traits::sorterptr sorterptr;
	/** Type of pipe sorter output. */
	typedef serialization_bits::sort_pull_output_t<Traits> output_t;

	typedef pipe_end<serialization_bits::passive_sorter_factory_input<Traits> > input_pipe_t;
	typedef pullpipe_begin<serialization_bits::passive_sorter_factory_output<Traits> > output_pipe_t;

	serialization_passive_sorter(pred_t pred = pred_t())
		: m_sorter_input(new sorter_t(sizeof(T), pred))
		, m_sorter_output(m_sorter_input)
	{
	}

	serialization_passive_sorter(const serialization_passive_sorter &) = delete;
	serialization_passive_sorter & operator=(const serialization_passive_sorter &) = delete;
	serialization_passive_sorter(serialization_passive_sorter && ) = default;
	serialization_passive_sorter & operator=(serialization_passive_sorter &&) = default;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the input push node.
	///////////////////////////////////////////////////////////////////////////
	input_pipe_t input() {
		assert(m_sorter_input);
		return input_pipe_t(std::move(m_sorter_input), m_calc_token);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the output pull node.
	///////////////////////////////////////////////////////////////////////////
	output_pipe_t output() {
		assert(m_sorter_output);
		return output_pipe_t(std::move(m_sorter_output), m_calc_token);
	}

private:
	sorterptr m_sorter_input;
	sorterptr m_sorter_output;
	node_token m_calc_token;
};

} // namespace pipelining

} // namespace tpie

#endif // TPIE_PIPELINING_SERIALIZATION_SORT_H
