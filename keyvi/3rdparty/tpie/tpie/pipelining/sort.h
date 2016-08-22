// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_SORT_H__
#define __TPIE_PIPELINING_SORT_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_base.h>
#include <tpie/pipelining/merge_sorter.h>
#include <tpie/parallel_sort.h>
#include <tpie/file_stream.h>
#include <tpie/tempname.h>
#include <tpie/memory.h>
#include <queue>
#include <memory>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename T, typename pred_t, typename store_t>
class sort_calc_t;

template <typename T, typename pred_t, typename store_t>
class sort_input_t;

template <typename T, typename pred_t, typename store_t>
class sort_output_base : public node {
	// node has virtual dtor
public:
	/** Type of items sorted. */
	typedef T item_type;
	
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<T, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	sorterptr get_sorter() const {
		return m_sorter;
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

	void add_calc_dependency(node_token tkn) {
		add_memory_share_dependency(tkn);
	}
		
protected:
	virtual void resource_available_changed(resource_type type, memory_size_type available) override {
		// TODO: Handle changing parameters of sorter after data structures has been frozen, i.e. after propagate
		if (m_propagate_called)
			return;

		if (type == MEMORY)
			m_sorter->set_phase_3_memory(available);
		else if (type == FILES) {
			m_sorter->set_phase_3_files(available);
		}
	}

	sort_output_base(sorterptr sorter)
		: m_sorter(sorter)
		, m_propagate_called(false)
	{
	}

	sorterptr m_sorter;
	bool m_propagate_called;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter pull output node.
/// \tparam pred_t   The less-than predicate.
/// \tparam dest_t   Destination node type.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class sort_pull_output_t : public sort_output_base<T, pred_t, store_t> {
public:
	/** Type of items sorted. */
	typedef T item_type;
	
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	sort_pull_output_t(sorterptr sorter)
		: sort_output_base<T, pred_t, store_t>(sorter)
	{
		this->set_minimum_resource_usage(FILES, sorter_t::minimumFilesPhase3);
		this->set_resource_fraction(FILES, 1.0);
		this->set_minimum_memory(sorter_t::minimum_memory_phase_3());
		this->set_maximum_memory(sorter_t::maximum_memory_phase_3());
		this->set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
		this->set_memory_fraction(1.0);
		this->set_plot_options(node::PLOT_BUFFERED);
	}
	
	void begin() override {
		this->m_sorter->set_owner(this);
	}

	bool can_pull() const {
		return this->m_sorter->can_pull();
	}

	item_type pull() {
		this->step();
		return this->m_sorter->pull();
	}

	void end() override {
		this->m_sorter.reset();
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
/// \tparam pred_t   The less-than predicate.
/// \tparam dest_t   Destination node type.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t, typename dest_t, typename store_t>
class sort_output_t : public sort_output_base<typename push_type<dest_t>::type, pred_t, store_t> {
public:
	/** Type of items sorted. */
	typedef typename push_type<dest_t>::type item_type;
	
	/** Base class */
	typedef sort_output_base<item_type, pred_t, store_t> p_t;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	inline sort_output_t(dest_t dest, sorterptr sorter)
		: p_t(sorter)
		, dest(std::move(dest))
	{
		this->add_push_destination(dest);
		this->set_minimum_resource_usage(FILES, sorter_t::minimumFilesPhase3);
		this->set_resource_fraction(FILES, 1.0);
		this->set_minimum_memory(sorter_t::minimum_memory_phase_3());
		this->set_maximum_memory(sorter_t::maximum_memory_phase_3());
		this->set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
		this->set_memory_fraction(1.0);
		this->set_plot_options(node::PLOT_BUFFERED);
	}

	void begin() override {
		this->m_sorter->set_owner(this);
	}
	
	virtual void go() override {
		while (this->m_sorter->can_pull()) {
			item_type && y=this->m_sorter->pull();
			dest.push(std::move(y));
			this->step();
		}
	}

	void end() override {
		this->m_sorter.reset();
	}

private:
	dest_t dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter middle node.
/// \tparam T        The type of items sorted
/// \tparam pred_t   The less-than predicate
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class sort_calc_t : public node {
public:
	/** Type of items sorted. */
	typedef T item_type;
	
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	typedef sort_output_base<T, pred_t, store_t> Output;

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
		set_minimum_resource_usage(FILES, sorter_t::minimumFilesPhase2);
		set_resource_fraction(FILES, 1.0);
		set_minimum_memory(sorter_t::minimum_memory_phase_2());
		set_name("Perform merge heap", PRIORITY_SIGNIFICANT);
		set_memory_fraction(1.0);
		set_plot_options(PLOT_BUFFERED | PLOT_SIMPLIFIED_HIDE);
		m_propagate_called = false;
	}

	virtual void propagate() override {
		set_steps(1000);
		m_propagate_called = true;
	}

	void begin() override {
		this->m_sorter->set_owner(this);
	}

	void end() override {
		m_weakSorter = m_sorter;
		m_sorter.reset();
	}

	virtual bool is_go_free() const override {return m_sorter->is_calc_free();}
	
	virtual void go() override {
		progress_indicator_base * pi = proxy_progress_indicator();
		m_sorter->calc(*pi);
	}

	virtual bool can_evacuate() override {
		return true;
	}

	virtual void evacuate() override {
		sorterptr sorter = m_weakSorter.lock();
		if (sorter) sorter->evacuate_before_reporting();
	}

	sorterptr get_sorter() const {
		return m_sorter;
	}

	void set_input_node(node & input) {
		add_memory_share_dependency(input);
	}

protected:
	virtual void resource_available_changed(resource_type type, memory_size_type available) override {
		// TODO: Handle changing parameters of sorter after data structures has been frozen, i.e. after propagate
		if (m_propagate_called)
			return;

		if (type == MEMORY)
			m_sorter->set_phase_2_memory(available);
		else if (type == FILES) {
			m_sorter->set_phase_2_files(available);
		}
	}

private:
	sorterptr m_sorter;
	std::weak_ptr<typename sorterptr::element_type> m_weakSorter;
	bool m_propagate_called;
	std::shared_ptr<Output> dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter input node.
/// \tparam T        The type of items sorted
/// \tparam pred_t   The less-than predicate
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class sort_input_t : public node {
public:
	/** Type of items sorted. */
	typedef T item_type;
	
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	inline sort_input_t(sort_calc_t<T, pred_t, store_t> dest)
		: m_sorter(dest.get_sorter())
		, m_propagate_called(false)
		, dest(std::move(dest))
	{
		this->dest.set_input_node(*this);
		set_name("Form input runs", PRIORITY_SIGNIFICANT);
		set_minimum_resource_usage(FILES, sorter_t::minimumFilesPhase1);
		set_resource_fraction(FILES, 0.0);
		set_minimum_memory(m_sorter->minimum_memory_phase_1());
		set_memory_fraction(1.0);
		set_plot_options(PLOT_BUFFERED | PLOT_SIMPLIFIED_HIDE);
	}

	virtual void propagate() override {
		if (this->can_fetch("items"))
			m_sorter->set_items(this->fetch<stream_size_type>("items"));
		m_propagate_called = true;
	}

	void push(item_type && item) {
		m_sorter->push(std::move(item));
	}

	void push(const item_type & item) {
		m_sorter->push(item);
	}

	void begin() override {
		m_sorter->begin();
		m_sorter->set_owner(this);
	}

	
	virtual void end() override {
		node::end();
		m_sorter->end();
		m_weakSorter = m_sorter;
		m_sorter.reset();
	}

	virtual bool can_evacuate() override {
		return true;
	}

	virtual void evacuate() override {
		sorterptr sorter = m_weakSorter.lock();
		if (sorter) sorter->evacuate_before_merging();
	}

protected:
	virtual void resource_available_changed(resource_type type, memory_size_type available) override {
		// TODO: Handle changing parameters of sorter after data structures has been frozen, i.e. after propagate
		if (m_propagate_called)
			return;

		if (type == MEMORY)
			m_sorter->set_phase_1_memory(available);
		else if (type == FILES) {
			m_sorter->set_phase_1_files(available);
		}
	}
private:
	sorterptr m_sorter;
	std::weak_ptr<typename sorterptr::element_type> m_weakSorter;
	bool m_propagate_called;
	sort_calc_t<T, pred_t, store_t> dest;
};

template <typename child_t, typename store_t>
class sort_factory_base : public factory_base {
	const child_t & self() const { return *static_cast<const child_t *>(this); }
public:
	template <typename dest_t>
	struct constructed {
	private:
		/** Type of items sorted. */
		typedef typename push_type<dest_t>::type item_type;
		typedef typename store_t::template element_type<item_type>::type element_type;
	public:
		typedef typename child_t::template predicate<element_type>::type pred_type;
		typedef sort_input_t<item_type, pred_type, store_t> type;
	};
	
	template <typename dest_t>
	typename constructed<dest_t>::type construct(dest_t dest) {
		typedef typename push_type<dest_t>::type item_type;
		typedef typename store_t::template element_type<item_type>::type element_type;
		typedef typename constructed<dest_t>::pred_type pred_type;

		sort_output_t<pred_type, dest_t, store_t> output(
			std::move(dest),
			std::make_shared<merge_sorter<item_type, true, pred_type, store_t> > (
				self().template get_pred<element_type>(), 
				m_store));
		this->init_sub_node(output);
		sort_calc_t<item_type, pred_type, store_t> calc(std::move(output));
		this->init_sub_node(calc);
		sort_input_t<item_type, pred_type, store_t> input(std::move(calc));
		this->init_sub_node(input);

		return std::move(input);
	}

	sort_factory_base(store_t store): m_store(store) {}
private:
	store_t m_store;

};

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort factory using std::less<T> as comparator.
///////////////////////////////////////////////////////////////////////////////
template <typename store_t>
class default_pred_sort_factory : public sort_factory_base<default_pred_sort_factory<store_t>, store_t> {
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

	default_pred_sort_factory(const store_t & store)
		: sort_factory_base<default_pred_sort_factory<store_t>, store_t>(store) 
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort factory using the given predicate as comparator.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t, typename store_t>
class sort_factory : public sort_factory_base<sort_factory<pred_t, store_t>, store_t> {
public:
	template <typename Dummy>
	class predicate {
	public:
		typedef pred_t type;
	};

	sort_factory(const pred_t & p, const store_t & store)
		: sort_factory_base<sort_factory<pred_t, store_t>, store_t>(store)
		, pred(p)
	{
	}

	template <typename T>
	pred_t get_pred() const {
		return pred;
	}
private:
	pred_t pred;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining sorter using std::less.
///////////////////////////////////////////////////////////////////////////////
inline pipe_middle<bits::default_pred_sort_factory<default_store> >
sort() {
	typedef bits::default_pred_sort_factory<default_store> fact;
	return pipe_middle<fact>(fact(default_store())).name("Sort");
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that sorts large elements indirectly by using 
/// a store and std::less.
///////////////////////////////////////////////////////////////////////////////
template <typename store_t>
inline pipe_middle<bits::default_pred_sort_factory<store_t> >
store_sort(store_t store=store_t()) {
	typedef bits::default_pred_sort_factory<store_t> fact;
	return pipe_middle<fact>(fact(store)).name("Sort");
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining sorter using the given predicate.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t>
inline pipe_middle<bits::sort_factory<pred_t, default_store> >
sort(const pred_t & p) {
	typedef bits::sort_factory<pred_t, default_store> fact;
	return pipe_middle<fact>(fact(p, default_store())).name("Sort");
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that sorts large elements indirectly by using 
/// a storeand a given predicate.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t, typename store_t>
inline pipe_middle<bits::sort_factory<pred_t, store_t> >
sort(const pred_t & p, store_t store) {
	typedef bits::sort_factory<pred_t, store_t> fact;
	return pipe_middle<fact>(fact(p, store)).name("Sort");
}

template <typename T, typename pred_t=std::less<T>, typename store_t=default_store>
class passive_sorter;

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief Factory for the passive sorter input node.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class passive_sorter_factory_input : public factory_base {
public:
	typedef sort_calc_t<T, pred_t, store_t> calc_t;
	typedef sort_input_t<T, pred_t, store_t> input_t;
	typedef input_t constructed_type;
	typedef merge_sorter<T, true, pred_t, store_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;
	
	passive_sorter_factory_input(sorterptr sorter, node_token calc_token)
		: m_sorter(sorter)
		, m_calc_token(calc_token) {}

	constructed_type construct() {
		calc_t calc(std::move(m_sorter), m_calc_token);
		this->init_node(calc);
		input_t input(std::move(calc));
		this->init_node(input);
		return input;
	}

private:
	sorterptr m_sorter;
	node_token m_calc_token;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Factory for the passive sorter output node.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class passive_sorter_factory_output : public factory_base {
public:
	typedef merge_sorter<T, true, pred_t, store_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;
	typedef bits::sort_pull_output_t<T, pred_t, store_t> constructed_type;
	
	passive_sorter_factory_output(sorterptr sorter, node_token calc_token)
		: m_sorter(sorter)
		, m_calc_token(calc_token)
		{}

	constructed_type construct() {
		constructed_type res(std::move(m_sorter));
		res.add_calc_dependency(m_calc_token);
		init_node(res);
		return res;
	}
private:
	sorterptr m_sorter;
	node_token m_calc_token;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelined sorter with push input and pull output.
/// Get the input pipe with \c input() and the output pullpipe with \c output().
/// \tparam T The type of item to sort
/// \tparam pred_t The predicate (e.g. std::less<T>) indicating the predicate
/// on which to order an item before another.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class passive_sorter {
public:
	/** Type of items sorted. */
	typedef T item_type;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;
	/** Type of pipe sorter output. */
	typedef bits::sort_pull_output_t<item_type, pred_t, store_t> output_t;

	passive_sorter(pred_t pred = pred_t(),
				   store_t store = store_t())
		: m_sorterInput(std::make_shared<sorter_t>(pred, store))
		, m_sorterOutput(m_sorterInput)
		{}

	passive_sorter(const passive_sorter &) = delete;
	passive_sorter & operator=(const passive_sorter &) = delete;
	passive_sorter(passive_sorter && ) = default;
	passive_sorter & operator=(passive_sorter &&) = default;
	
	typedef pipe_end<bits::passive_sorter_factory_input<item_type, pred_t, store_t> > input_pipe_t;
	typedef pullpipe_begin<bits::passive_sorter_factory_output<item_type, pred_t, store_t> > output_pipe_t;
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the input push node.
	///////////////////////////////////////////////////////////////////////////
	input_pipe_t input() {
		tp_assert(m_sorterInput, "input() called more than once");
		auto ret = bits::passive_sorter_factory_input<item_type, pred_t, store_t>(
			std::move(m_sorterInput), m_calc_token);
		return {std::move(ret)};
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the output pull node.
	///////////////////////////////////////////////////////////////////////////
	output_pipe_t output() {
		tp_assert(m_sorterOutput, "output() called more than once");
		auto ret =  bits::passive_sorter_factory_output<item_type, pred_t, store_t>(
			std::move(m_sorterOutput), m_calc_token);
		return {std::move(ret)};
	}
	
private:
	sorterptr m_sorterInput, m_sorterOutput;
	node_token m_calc_token;
};

} // namespace pipelining

} // namespace tpie

#endif
