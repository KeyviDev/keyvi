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

#ifndef __TPIE_PIPELINING_NODE_H__
#define __TPIE_PIPELINING_NODE_H__

#include <tpie/pipelining/exception.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/progress_indicator_base.h>
#include <tpie/progress_indicator_null.h>
#include <tpie/pipelining/priority_type.h>
#include <tpie/pipelining/predeclare.h>
#include <tpie/pipelining/node_name.h>
#include <tpie/pipelining/node_traits.h>
#include <tpie/flags.h>
#include <limits>
#include <tpie/resources.h>

namespace tpie {

namespace pipelining {

namespace bits {

class proxy_progress_indicator : public tpie::progress_indicator_base {
	node & m_node;

public:
	proxy_progress_indicator(node & s);

	virtual void refresh() override;
};

} // namespace bits

struct node_resource_parameters {
	memory_size_type minimum = 0;
	memory_size_type maximum = std::numeric_limits<memory_size_type>::max();
	double fraction = 0.0;

	memory_size_type available = 0;
};

struct node_parameters {
	node_resource_parameters resource_parameters[resource_type::TOTAL_RESOURCE_TYPES];

	std::string name;
	priority_type namePriority = PRIORITY_NO_NAME;

	std::string phaseName;
	priority_type phaseNamePriority = PRIORITY_NO_NAME;

	stream_size_type stepsTotal = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// Base class of all nodes. A node should inherit from the node class,
/// have a single template parameter dest_t if it is not a terminus node,
/// and implement methods begin(), push() and end(), if it is not a source
/// node.
///////////////////////////////////////////////////////////////////////////////
class node {
public:
	typedef boost::optional<any_noncopyable &> maybeany_t;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Options for how to plot this node
	//////////////////////////////////////////////////////////////////////////
	enum PLOT {
		PLOT_SIMPLIFIED_HIDE=1,
		PLOT_BUFFERED=2,
		PLOT_PARALLEL=4
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used internally to check order of method calls.
	///////////////////////////////////////////////////////////////////////////
	enum STATE {
		STATE_FRESH,
		STATE_IN_PREPARE,
		STATE_AFTER_PREPARE,
		STATE_IN_PROPAGATE,
		STATE_AFTER_PROPAGATE,
		STATE_IN_BEGIN,
		STATE_AFTER_BEGIN,
		STATE_IN_GO,
		STATE_IN_END,
		STATE_AFTER_END
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~node() {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the minimum amount of the resource declared by this node.
	/// Defaults to zero when no minimum has been set.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_minimum_resource_usage(resource_type type) const {
		return m_parameters.resource_parameters[type].minimum;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the maximum amount of the resource declared by this node.
	/// Defaults to maxint when no maximum has been set.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_maximum_resource_usage(resource_type type) const {
		return m_parameters.resource_parameters[type].maximum;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the priority for the specific resource of this node.
	///////////////////////////////////////////////////////////////////////////
	inline double get_resource_fraction(resource_type type) const {
		return m_parameters.resource_parameters[type].fraction;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the amount of the specific resource assigned to this node.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_available_of_resource(resource_type type) const {
		return m_parameters.resource_parameters[type].available;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare minimum resource requirements.
	///////////////////////////////////////////////////////////////////////////
	void set_minimum_resource_usage(resource_type type, memory_size_type usage);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare maximum resource requirements.
	///
	/// To signal that you don't want to use this resource,
	/// set minimum resource usage and the resource fraction to zero.
	///////////////////////////////////////////////////////////////////////////
	void set_maximum_resource_usage(resource_type type, memory_size_type usage);

	///////////////////////////////////////////////////////////////////////////
	/// \Brief Set the resource priority of this node. Resources are
	/// distributed proportionally to the priorities of the nodes in the given
	/// phase.
	///////////////////////////////////////////////////////////////////////////
	void set_resource_fraction(resource_type type, double f);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by the resource manager to notify the node's available
	/// amount of resource has changed.
	///////////////////////////////////////////////////////////////////////////
	virtual void resource_available_changed(resource_type, memory_size_type) {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally to assign the available resource to the node.
	///////////////////////////////////////////////////////////////////////////
	void _internal_set_available_of_resource(resource_type type, memory_size_type available);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the minimum amount of memory declared by this node.
	/// Defaults to zero when no minimum has been set.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_minimum_memory() const {
		return get_minimum_resource_usage(MEMORY);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the maximum amount of memory declared by this node.
	/// Defaults to maxint when no maximum has been set.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_maximum_memory() const {
		return get_maximum_resource_usage(MEMORY);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the memory priority of this node.
	///////////////////////////////////////////////////////////////////////////
	inline double get_memory_fraction() const {
		return get_resource_fraction(MEMORY);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the amount of memory assigned to this node.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_available_memory() const {
		return get_available_of_resource(MEMORY);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare minimum memory requirements.
	///////////////////////////////////////////////////////////////////////////
	void set_minimum_memory(memory_size_type minimumMemory) {
		set_minimum_resource_usage(MEMORY, minimumMemory);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare maximum memory requirements.
	///
	/// To signal that you don't want any memory, set minimum memory and the
	/// memory fraction to zero.
	///////////////////////////////////////////////////////////////////////////
	void set_maximum_memory(memory_size_type maximumMemory) {
		set_maximum_resource_usage(MEMORY, maximumMemory);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \Brief Set the memory priority of this node. Memory is distributed
	/// proportionally to the priorities of the nodes in the given phase.
	///////////////////////////////////////////////////////////////////////////
	void set_memory_fraction(double f) {
		set_resource_fraction(MEMORY, f);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by the memory manager to set the amount of memory
	/// assigned to this node.
	///////////////////////////////////////////////////////////////////////////
	virtual void set_available_memory(memory_size_type availableMemory) {
		unused(availableMemory);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the amount of memory currently used by this node.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_used_memory() const {
		memory_size_type ans=0;
		for (const auto & p: m_buckets)
			if (p) ans += p->count;
		return ans;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the local node map, mapping node IDs to node
	/// pointers for all the nodes reachable from this one.
	///////////////////////////////////////////////////////////////////////////
	inline bits::node_map::ptr get_node_map() const {
		return token.get_map();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the internal node ID of this node (mainly
	/// for debugging purposes).
	///////////////////////////////////////////////////////////////////////////
	inline node_token::id_t get_id() const {
		return token.id();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called before memory assignment but after depending phases have
	/// executed and ended. The implementer may use fetch and forward in this
	/// phase. The implementer does not have to call the super prepare-method;
	/// its default implementation is empty.
	///////////////////////////////////////////////////////////////////////////
	virtual void prepare() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Propagate stream metadata.
	///
	/// The implementation may fetch() and forward() metadata such as number of
	/// items or the size of a single item.
	///
	/// The pipelining framework calls propagate() on the nodes in the
	/// item flow graph in a topological order.
	///
	/// The default implementation does nothing.
	///////////////////////////////////////////////////////////////////////////
	virtual void propagate() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Begin pipeline processing phase.
	///
	/// The implementation may pull() from a pull destination in begin(),
	/// and it may push() to a push destination.
	///
	/// The pipelining framework calls begin() on the nodes in the
	/// actor graph in a reverse topological order. The framework calls
	/// node::begin() on a node after calling begin() on its pull and push
	/// destinations.
	///
	/// The default implementation does nothing.
	///////////////////////////////////////////////////////////////////////////
	virtual void begin() {
	}

	virtual bool is_go_free() const {return false;}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief For initiator nodes, execute this phase by pushing all items
	/// to be pushed. For non-initiator nodes, the default implementation
	/// throws a not_initiator_node exception.
	///////////////////////////////////////////////////////////////////////////
	virtual void go() {
		log_warning() << "node subclass " << typeid(*this).name()
			<< " is not an initiator node" << std::endl;
		throw not_initiator_node();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief End pipeline processing phase.
	///
	/// The implementation may pull() from a pull destination in end(),
	/// and it may push() to a push destination.
	///
	/// The pipelining framework calls end() on the nodes in the
	/// pipeline graph in a topological order. The framework calls
	/// node::end() on a node before its pull and push
	/// destinations.
	///
	/// The default implementation does nothing, so it does not matter if the
	/// implementation calls the parent end().
	///////////////////////////////////////////////////////////////////////////
	virtual void end() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Overridden by nodes that have data to evacuate.
	///////////////////////////////////////////////////////////////////////////
	virtual bool can_evacuate() {
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Overridden by nodes that have data to evacuate.
	///////////////////////////////////////////////////////////////////////////
	virtual void evacuate() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the priority of this node's name. For purposes of
	/// pipeline debugging and phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	inline priority_type get_name_priority() {
		return m_parameters.namePriority;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get this node's name. For purposes of pipeline debugging and
	/// phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	const std::string & get_name();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Set this node's name. For purposes of pipeline debugging and
	/// phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	void set_name(const std::string & name, priority_type priority = PRIORITY_USER);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the priority of this node's pdane name. For purposes of
	/// pipeline debugging and phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	inline priority_type get_phase_name_priority() {
		return m_parameters.phaseNamePriority;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get this node's phase name. For purposes of pipeline debugging and
	/// phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	const std::string & get_phase_name();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Set this node's phase name. For purposes of pipeline debugging and
	/// phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	void set_phase_name(const std::string & name, priority_type priority = PRIORITY_USER);

	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally when a pair_factory has a name set.
	///////////////////////////////////////////////////////////////////////////
	inline void set_breadcrumb(const std::string & breadcrumb) {
		m_parameters.name = m_parameters.name.empty() ? breadcrumb : (breadcrumb + " | " + m_parameters.name);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally for progress indication. Get the number of times
	/// the node expects to call step() at most.
	///////////////////////////////////////////////////////////////////////////
	inline stream_size_type get_steps() {
		return m_parameters.stepsTotal;
	}

	stream_size_type get_steps_left() {
		return m_stepsLeft;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally. Set the progress indicator to use.
	///////////////////////////////////////////////////////////////////////////
	inline void set_progress_indicator(progress_indicator_base * pi) {
		m_pi = pi;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally. Get the progress indicator used.
	///////////////////////////////////////////////////////////////////////////
	progress_indicator_base * get_progress_indicator() {
		return m_pi;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used internally to check order of method calls.
	///////////////////////////////////////////////////////////////////////////
	STATE get_state() const {
		return m_state;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used internally to check order of method calls.
	///////////////////////////////////////////////////////////////////////////
	void set_state(STATE s) {
		m_state = s;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used internally to check order of method calls.
	///////////////////////////////////////////////////////////////////////////
	resource_type get_resource_being_assigned() const {
		return m_resourceBeingAssigned;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used internally to check order of method calls.
	///////////////////////////////////////////////////////////////////////////
	void set_resource_being_assigned(resource_type type) {
		m_resourceBeingAssigned = type;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get options specified for plot(), as a combination of
	/// \c node::PLOT values.
	///////////////////////////////////////////////////////////////////////////
	flags<PLOT> get_plot_options() const {
		return m_plotOptions;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Set options specified for plot(), as a combination of
	/// \c node::PLOT values.
	///////////////////////////////////////////////////////////////////////////
	void set_plot_options(flags<PLOT> options) {
		m_plotOptions = options;
	}
protected:
#ifdef _WIN32
	// Disable warning C4355: 'this' : used in base member initializer list
	// node_token does not access members of the `node *`,
	// it merely uses it as a value in the node map.
	// Only after this node object is completely constructed are node members accessed.
#pragma warning( push )
#pragma warning( disable : 4355 )
#endif // _WIN32
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor, using a new node_token.
	///////////////////////////////////////////////////////////////////////////
	node();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Copy constructor. We need to define this explicitly since the
	/// node_token needs to know its new owner.
	///////////////////////////////////////////////////////////////////////////
	node(const node & other) = delete;
	node & operator=(const node & other) = delete;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Move constructor. We need to define this explicitly since the
	/// node_token needs to know its new owner.
	///////////////////////////////////////////////////////////////////////////
	node(node && other);
	node & operator=(node && other);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor using a given fresh node_token.
	///////////////////////////////////////////////////////////////////////////
	node(const node_token & token);
#ifdef _WIN32
#pragma warning( pop )
#endif // _WIN32
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a push destination.
	///////////////////////////////////////////////////////////////////////////
	void add_push_destination(const node_token & dest);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a push destination.
	///////////////////////////////////////////////////////////////////////////
	void add_push_destination(const node & dest);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a pull source.
	///////////////////////////////////////////////////////////////////////////
	void add_pull_source(const node_token & dest);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a pull source.
	///////////////////////////////////////////////////////////////////////////
	void add_pull_source(const node & dest);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a node dependency, that is,
	/// a requirement that another node has end() called before the begin()
	/// of this node.
	///////////////////////////////////////////////////////////////////////////
	void add_dependency(const node_token & dest);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a node dependency, that is,
	/// a requirement that another node has end() called before the begin()
	/// of this node.
	///////////////////////////////////////////////////////////////////////////
	void add_dependency(const node & dest);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a node memory share
	/// dependency, that is, a requirement that another node has end() called
	/// before the begin() of this node, and memory shared between end() and
	/// begin() unless evacuate() is called
	///////////////////////////////////////////////////////////////////////////
	void add_memory_share_dependency(const node_token & dest);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a node memory share
	/// dependency, that is, a requirement that another node has end() called
	/// before the begin() of this node, and memory shared between end() and
	/// begin() unless evacuate() is called
	///////////////////////////////////////////////////////////////////////////
	void add_memory_share_dependency(const node & dest);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to forward auxiliary data to successors.
	/// If explicitForward is false, the data will not override data forwarded
	/// with explicitForward == true.
	/// \param key The key of forwarded data
	/// \param value The value of forwarded data
	/// \param k The maximum distance to forward the distance. If there are
	/// more than k nodes between the forwarding nodes and another node b.
	/// b will not be able to fetch the data. Defaults to infinity.
	///////////////////////////////////////////////////////////////////////////
	// Implementation note: If the type of the `value` parameter is changed
	// from `T` to `const T &`, this will yield linker errors if an application
	// attempts to pass a const reference to a static data member inside a
	// templated class.
	// See http://stackoverflow.com/a/5392050
	///////////////////////////////////////////////////////////////////////////
	template <typename T>
	void forward(std::string key, T value, memory_size_type k = std::numeric_limits<memory_size_type>::max()) {
		forward_any(key, any_noncopyable(std::move(value)), k);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief See \ref node::forward.
	///////////////////////////////////////////////////////////////////////////
	void forward_any(std::string key, any_noncopyable value, memory_size_type k = std::numeric_limits<memory_size_type>::max());

private:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by forward_any to add forwarded data.
	///////////////////////////////////////////////////////////////////////////
	void add_forwarded_data(std::string key, node_token::id_t from_node);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by fetch_any to get data forwarded from this node.
	///////////////////////////////////////////////////////////////////////////
	maybeany_t get_forwarded_data_maybe(std::string key);

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Find out if there is a piece of auxiliary data forwarded with a
	/// given name.
	///////////////////////////////////////////////////////////////////////////
	bool can_fetch(std::string key) {
		return bool(fetch_maybe(key));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Fetch piece of auxiliary data as any_noncopyable (the internal
	/// representation) wrapped in a boost::optional which is unitialized
	/// if the key is not found.
	///////////////////////////////////////////////////////////////////////////
	maybeany_t fetch_maybe(std::string key);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Fetch piece of auxiliary data as any_noncopyable (the internal
	/// representation).
	///////////////////////////////////////////////////////////////////////////
	any_noncopyable & fetch_any(std::string key);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Fetch piece of auxiliary data, expecting a given value type.
	///////////////////////////////////////////////////////////////////////////
	template <typename T>
	inline T & fetch(std::string key) {
		any_noncopyable &item = fetch_any(key);
		try {
			return any_cast<T>(item);
		} catch (bad_any_noncopyable_cast m) {
			std::stringstream ss;
			ss << "Trying to fetch key '" << key << "' of type "
			   << typeid(T).name() << " but forwarded data was of type "
			   << item.type().name() << ". Message was: " << m.what();
			throw invalid_argument_exception(ss.str());
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the node_token that maps this node's ID to a pointer
	/// to this.
	///////////////////////////////////////////////////////////////////////////
	const node_token & get_token() const {
		return token;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers that intend to call step().
	/// \param steps  The number of times step() is called at most.
	///////////////////////////////////////////////////////////////////////////
	void set_steps(stream_size_type steps);


private:
	///////////////////////////////////////////////////////////////////////////////
	/// \brief used by step() when too many steps are taken
	///////////////////////////////////////////////////////////////////////////////
	void step_overflow();
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Step the progress indicator.
	/// \param steps  How many steps to step.
	///////////////////////////////////////////////////////////////////////////
	void step(stream_size_type steps = 1) {
		assert(get_state() == STATE_IN_END ||
			   get_state() == STATE_IN_BEGIN ||
			   get_state() == STATE_AFTER_BEGIN ||
			   get_state() == STATE_IN_END ||
			   get_state() == STATE_IN_GO);
		if (m_stepsLeft < steps)
			step_overflow();
		else
			m_stepsLeft -= steps;
		m_pi->step(steps);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get a non-initialized progress indicator for use with external
	/// implementations. When step() is called on a proxying progress
	/// indicator, step() is called on the node according to the number
	/// of steps declared in progress_indicator_base::init() and in
	/// node::set_steps().
	///////////////////////////////////////////////////////////////////////////
	progress_indicator_base * proxy_progress_indicator();

#ifdef DOXYGEN
	///////////////////////////////////////////////////////////////////////////
	/// \brief For pull nodes, return true if there are more items to be
	/// pulled.
	///////////////////////////////////////////////////////////////////////////
	inline bool can_pull() const;

	///////////////////////////////////////////////////////////////////////////
	/// \brief For pull nodes, pull the next item from this node.
	///////////////////////////////////////////////////////////////////////////
	inline item_type pull();

	///////////////////////////////////////////////////////////////////////////
	/// \brief For push nodes, push the next item to this node.
	///////////////////////////////////////////////////////////////////////////
	inline void push(const item_type & item);
#endif

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Registers a datastructure
	/// \param name the name of the datastructure
	/// \param priority the priority that should be given to this datastructure
	/// when assigning memory
	///////////////////////////////////////////////////////////////////////////////
	void register_datastructure_usage(const std::string & name, double priority=1);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Assign memory to a registered datastructure
	/// \param name the name of the datastructure
	/// \param min the minimum amount of memory required by the datastructure
	/// \param max the maximum amount of memory used by the datastructure
	///////////////////////////////////////////////////////////////////////////////
	void set_datastructure_memory_limits(const std::string & name, memory_size_type min, memory_size_type max=std::numeric_limits<memory_size_type>::max());

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns the memory assigned to a datastructure
	/// \param name the name of the datastructure
	///////////////////////////////////////////////////////////////////////////////
	memory_size_type get_datastructure_memory(const std::string & name);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns a previously declared datastructure
	/// \param name the name of the datastructure
	/// \param datastructure the datastructure itself
	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	void set_datastructure(const std::string & name, T datastructure) {
		bits::node_map::datastructuremap_t & structures = get_node_map()->find_authority()->get_datastructures();
		bits::node_map::datastructuremap_t::iterator i = structures.find(name);

		if(i == structures.end())
			throw tpie::exception("attempted to set non-registered datastructure");

		i->second.second = move_if_movable<T>(datastructure);
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns a previously declared datastructure
	/// \param name the name of the datastructure
	/// \tparam the type of the datastructure
	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	T & get_datastructure(const std::string & name) {
		bits::node_map::datastructuremap_t & structures = get_node_map()->find_authority()->get_datastructures();
		bits::node_map::datastructuremap_t::iterator i = structures.find(name);

		if(i == structures.end())
			throw tpie::exception("attempted to get non-registered datastructure");

		return any_cast<T>(i->second.second);
	}

	void unset_datastructure(const std::string & name) {
		bits::node_map::datastructuremap_t & structures = get_node_map()->find_authority()->get_datastructures();
		bits::node_map::datastructuremap_t::iterator i = structures.find(name);

		if(i == structures.end()) return;
		i->second.second.reset();
	}
	
private:
	struct datastructure_info_t {
		datastructure_info_t() : min(0), max(std::numeric_limits<memory_size_type>::max()) {}
		memory_size_type min;
		memory_size_type max;
		double priority;
	};

	typedef std::map<std::string, datastructure_info_t> datastructuremap_t;

	const datastructuremap_t & get_datastructures() const {
		return m_datastructures;
	}

public:
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns the flush priority of this node
	///////////////////////////////////////////////////////////////////////////////
	memory_size_type get_flush_priority() {
		return m_flushPriority;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Sets the flush priority of this node
	/// \param flushPriority the flush priority
	///////////////////////////////////////////////////////////////////////////////
	void set_flush_priority(memory_size_type flushPriority) {
		m_flushPriority = flushPriority;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Count the number of memory buckets
	///////////////////////////////////////////////////////////////////////////////	
	size_t buckets() const {return m_buckets.size();}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Access a memory bucket
	///////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<memory_bucket> & bucket(size_t i) {
		if (m_buckets.size() <= i) m_buckets.resize(i+1);
		if (!m_buckets[i]) m_buckets[i].reset(new memory_bucket());
		return m_buckets[i];
	}
		
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Return an allocator that counts memory usage within the node
	///////////////////////////////////////////////////////////////////////////////
	tpie::memory_bucket_ref allocator(size_t i=0) {
		return tpie::memory_bucket_ref(bucket(i).get());
	}
	
	friend class bits::memory_runtime;

	friend class bits::datastructure_runtime;

	friend class factory_base;

	friend class bits::pipeline_base;
private:
	node_token token;

	node_parameters m_parameters;
	std::vector<std::unique_ptr<memory_bucket> > m_buckets;
	
	std::map<std::string, any_noncopyable> m_forwardedFromHere;
	std::map<std::string, node_token::id_t> m_forwardedToHere;

	datastructuremap_t m_datastructures;
	memory_size_type m_flushPriority;
	stream_size_type m_stepsLeft;
	progress_indicator_base * m_pi;
	STATE m_state;
	resource_type m_resourceBeingAssigned = NO_RESOURCE;
	std::unique_ptr<progress_indicator_base> m_piProxy;
	flags<PLOT> m_plotOptions;

	friend class bits::proxy_progress_indicator;
};


TPIE_DECLARE_OPERATORS_FOR_FLAGS(node::PLOT);


} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_NODE_H__
