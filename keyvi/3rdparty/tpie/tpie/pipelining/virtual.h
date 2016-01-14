// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
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

///////////////////////////////////////////////////////////////////////////////
/// \file virtual.h  Virtual wrappers for nodes
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_VIRTUAL_H__
#define __TPIE_PIPELINING_VIRTUAL_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/pipeline.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/helpers.h>

namespace tpie {

namespace pipelining {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Virtual base class for extra data to go with virtual chunks.
/// When given to a virtual_chunk constructor, it will be automatically
/// std::deleted the virtual chunk is garbage collected.
///////////////////////////////////////////////////////////////////////////////
class virtual_container {
public:
	virtual ~virtual_container() {}
};

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief The maybe_add_const_ref helper struct adds const & to a type unless
/// the type is already const, reference or pointer type.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
struct maybe_add_const_ref {
	typedef const T & type;
};

template <typename T>
struct maybe_add_const_ref<const T &> {
	typedef const T & type;
};

template <typename T>
struct maybe_add_const_ref<const T *> {
	typedef const T * type;
};

template <typename T>
struct maybe_add_const_ref<T &> {
	typedef T & type;
};

template <typename T>
struct maybe_add_const_ref<T *> {
	typedef T * type;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual base node that is injected into the beginning of a
/// virtual chunk. For efficiency, the push method accepts a const reference
/// type unless the item type is already const/ref/pointer.
///////////////////////////////////////////////////////////////////////////////
template <typename Input>
class virtsrc : public node {
	typedef typename maybe_add_const_ref<Input>::type input_type;

public:
	virtual const node_token & get_token() = 0;
	virtual void push(input_type v) = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Concrete implementation of virtsrc.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t, typename T>
class virtsrc_impl : public virtsrc<T> {
public:
	typedef T item_type;

private:
	typedef typename maybe_add_const_ref<item_type>::type input_type;
	dest_t dest;

public:
	virtsrc_impl(dest_t dest)
		: dest(std::move(dest))
	{
		node::add_push_destination(this->dest);
		this->set_name("Virtual source", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(node::PLOT_BUFFERED | node::PLOT_SIMPLIFIED_HIDE);
	}

	const node_token & get_token() {
		return node::get_token();
	}

	void push(input_type v) {
		dest.push(v);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual node that is injected into the end of a virtual
/// chunk. May be dynamically connected to a virtsrc using the set_destination
/// method.
///////////////////////////////////////////////////////////////////////////////
template <typename Output>
class virtrecv : public node {
	virtrecv *& m_self;
	virtsrc<Output> * m_virtdest;

public:
	typedef Output item_type;

	virtrecv(virtrecv *& self)
		: m_self(self)
		, m_virtdest(0)
	{
		m_self = this;
		this->set_name("Virtual destination", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(node::PLOT_BUFFERED | node::PLOT_SIMPLIFIED_HIDE);
	}

	virtrecv(virtrecv && o)
		: node(std::move(o))
		, m_self(o.m_self)
		, m_virtdest(std::move(o.m_virtdest)) {
		m_self = this;
	}

	void begin() {
		node::begin();
		if (m_virtdest == 0) {
			throw tpie::exception("No virtual destination");
		}
	}

	void push(typename maybe_add_const_ref<Output>::type v) {
		m_virtdest->push(v);
	}

	void set_destination(virtsrc<Output> * dest) {
		if (m_virtdest != 0) {
			throw tpie::exception("Virtual destination set twice");
		}

		m_virtdest = dest;
		add_push_destination(dest->get_token());
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Ownership of nodes. This class can only be instantiated
/// through static methods that return a virt_node::ptr, providing reference
/// counting so that nodes are only instantiated once each and are
/// destroyed when the pipeline object goes out of scope.
///
/// This class either owns a node or two virt_nodes. The first case is
/// used in the virtual_chunk constructors that accept a single pipe_base. The
/// second case is used in the virtual_chunk pipe operators.
///////////////////////////////////////////////////////////////////////////////
class virt_node {
public:
	typedef std::shared_ptr<virt_node> ptr;

private:
	std::unique_ptr<node> m_node;
	std::unique_ptr<virtual_container> m_container;
	ptr m_left;
	ptr m_right;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Take std::new-ownership of given node.
	///////////////////////////////////////////////////////////////////////////
	static ptr take_own(node * pipe) {
		virt_node * n = new virt_node();
		n->m_node.reset(pipe);
		ptr res(n);
		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Aggregate ownership of virt_nodes.
	///////////////////////////////////////////////////////////////////////////
	static ptr combine(ptr left, ptr right) {
		virt_node * n = new virt_node();
		n->m_left = left;
		n->m_right = right;
		ptr res(n);
		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Set and/or reset the virtual_container assigned to this virtual
	/// node.
	///////////////////////////////////////////////////////////////////////////
	void set_container(virtual_container * ctr) {
		m_container.reset(ctr);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Helper class that throws an exception on behalf of virtual_chunks
/// that have not been assigned a pipe_middle. When the input and output are
/// different, a virtual_chunk_missing_middle is thrown.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename U, typename Result>
struct assert_types_equal_and_return {
	static Result go(...) {
		throw virtual_chunk_missing_middle();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Template partial specialization that just returns the parameter
/// given when the input and output types of a virtual chunk are the same
/// (implicit identity function).
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename Result>
struct assert_types_equal_and_return<T, T, Result> {
	static Result go(Result r) {
		return r;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class of virtual chunks. Owns a virt_node.
///////////////////////////////////////////////////////////////////////////////
class virtual_chunk_base : public pipeline_base {
	// pipeline_base has virtual dtor and shared_ptr to m_nodeMap
protected:
	virt_node::ptr m_node;
public:
	virtual_chunk_base() {}

	virt_node::ptr get_node() const { return m_node; }
	virtual_chunk_base(node_map::ptr nodeMap, virt_node::ptr ptr)
		: m_node(ptr)
	{
		this->m_nodeMap = nodeMap;
	}

	virtual_chunk_base(node_map::ptr nodeMap) {
		this->m_nodeMap = nodeMap;
	}

	void set_container(virtual_container * ctr) {
		m_node->set_container(ctr);
	}

	bool empty() const { return m_node.get() == 0; }
};

} // namespace bits

// Predeclare
template <typename Input>
class virtual_chunk_end;

// Predeclare
template <typename Input, typename Output>
class virtual_chunk;

// Predeclare
template <typename Output>
class virtual_chunk_begin;

namespace bits {

class access {
	template <typename>
	friend class pipelining::virtual_chunk_end;
	template <typename, typename>
	friend class pipelining::virtual_chunk;
	template <typename>
	friend class pipelining::virtual_chunk_begin;
	template <typename>
	friend class vfork_node;
	template <typename>
	friend class vpush_node;

	template <typename Input>
	static virtsrc<Input> * get_source(const virtual_chunk_end<Input> &);
	template <typename Input, typename Output>
	static virtsrc<Input> * get_source(const virtual_chunk<Input, Output> &);
	template <typename Input, typename Output>
	static virtrecv<Output> * get_destination(const virtual_chunk<Input, Output> &);
	template <typename Output>
	static virtrecv<Output> * get_destination(const virtual_chunk_begin<Output> &);
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual chunk that has no output (that is, virtual consumer).
///////////////////////////////////////////////////////////////////////////////
template <typename Input>
class virtual_chunk_end : public bits::virtual_chunk_base {
	friend class bits::access;
	typedef bits::access acc;
	typedef bits::virtsrc<Input> src_type;
	src_type * m_src;

	src_type * get_source() const { return m_src; }

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that leaves the virtual chunk unassigned.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_end()
		: m_src(0)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that recursively constructs a node and takes
	/// ownership of it.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_end(pipe_end<fact_t> && pipe, virtual_container * ctr = 0) {
		*this = std::forward<pipe_end<fact_t>>(pipe);
		set_container(ctr);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that combines two virtual chunks. Assumes that the
	/// virtual nodes are already connected. You should not use this
	/// constructor directly; instead, use the pipe operator.
	///////////////////////////////////////////////////////////////////////////
	template <typename Mid>
	virtual_chunk_end(const virtual_chunk<Input, Mid> & left,
					  const virtual_chunk_end<Mid> & right);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a node and assign it to this virtual chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_end & operator=(pipe_end<fact_t> && pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw tpie::exception("Virtual chunk assigned twice");
		}

		typedef typename fact_t::constructed_type constructed_type;
		m_src = new bits::virtsrc_impl<constructed_type, Input>(pipe.factory.construct());
		this->m_node = bits::virt_node::take_own(m_src);
		this->m_nodeMap = m_src->get_node_map();

		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual chunk that has input and output.
///////////////////////////////////////////////////////////////////////////////
template <typename Input, typename Output=Input>
class virtual_chunk : public bits::virtual_chunk_base {
	friend class bits::access;
	typedef bits::access acc;
	typedef bits::virtsrc<Input> src_type;
	typedef bits::virtrecv<Output> recv_type;
	src_type * m_src;
	recv_type * m_recv;
	src_type * get_source() const { return m_src; }
	recv_type * get_destination() const { return m_recv; }

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that leaves the virtual chunk unassigned.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk()
		: m_src(0)
		, m_recv(0)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that recursively constructs a node and takes
	/// ownership of it.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk(pipe_middle<fact_t> && pipe, virtual_container * ctr = 0) {
		*this = std::forward<pipe_middle<fact_t>>(pipe);
		set_container(ctr);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that combines two virtual chunks. Assumes that the
	/// virtual nodes are already connected. You should not use this
	/// constructor directly; instead, use the pipe operator.
	///////////////////////////////////////////////////////////////////////////
	template <typename Mid>
	virtual_chunk(const virtual_chunk<Input, Mid> & left,
				  const virtual_chunk<Mid, Output> & right)
		: virtual_chunk_base(left.get_node_map(), bits::virt_node::combine(left.get_node(), right.get_node()))
	{
		m_src = acc::get_source(left);
		m_recv = acc::get_destination(right);
	}

	virtual_chunk(const virtual_chunk_end<Input> & left,
				  const virtual_chunk_begin<Output> & right)
		: virtual_chunk_base(left.get_node_map(), bits::virt_node::combine(left.get_node(), right.get_node()))
	{
		m_src = acc::get_source(left);
		m_recv = acc::get_destination(right);
	}

	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a node and assign it to this virtual chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk & operator=(pipe_middle<fact_t> && pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw tpie::exception("Virtual chunk assigned twice");
		}
		typedef typename fact_t::template constructed<recv_type>::type constructed_type;
		recv_type temp(m_recv);
		this->m_nodeMap = temp.get_node_map();
		fact_t f = std::move(pipe.factory);
		f.set_destination_kind_push();
		m_src = new bits::virtsrc_impl<constructed_type, Input>(f.construct(std::move(temp)));
		this->m_node = bits::virt_node::take_own(m_src);

		return *this;
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename NextOutput>
	virtual_chunk<Input, NextOutput> operator|(virtual_chunk<Output, NextOutput> dest) {
		if (empty()) {
			return *bits::assert_types_equal_and_return<Input, Output, virtual_chunk<Input, NextOutput> *>
				::go(&dest);
		}
		bits::virtsrc<Output> * dst=acc::get_source(dest);
		if (dest.empty() || !dst) {
			return *bits::assert_types_equal_and_return<Output, NextOutput, virtual_chunk<Input, NextOutput> *>
				::go(this);
		}
		m_recv->set_destination(dst);
		return virtual_chunk<Input, NextOutput>(*this, dest);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_end<Input> operator|(virtual_chunk_end<Output> dest) {
		if (empty()) {
			return *bits::assert_types_equal_and_return<Input, Output, virtual_chunk_end<Input> *>
				::go(&dest);
		}
		m_recv->set_destination(acc::get_source(dest));
		return virtual_chunk_end<Input>(*this, dest);
	}
};

template <typename Input>
template <typename Mid>
virtual_chunk_end<Input>::virtual_chunk_end(const virtual_chunk<Input, Mid> & left,
											const virtual_chunk_end<Mid> & right)
	: virtual_chunk_base(left.get_node_map(),
						 bits::virt_node::combine(left.get_node(), right.get_node()))
{
	m_src = acc::get_source(left);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual chunk that has no input (that is, virtual producer).
///////////////////////////////////////////////////////////////////////////////
template <typename Output>
class virtual_chunk_begin : public bits::virtual_chunk_base {
	friend class bits::access;
	typedef bits::access acc;
	typedef bits::virtrecv<Output> recv_type;
	recv_type * m_recv;
	recv_type * get_destination() const { return m_recv; }

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that leaves the virtual chunk unassigned.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_begin()
		: m_recv(0)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that recursively constructs a node and takes
	/// ownership of it.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_begin(pipe_begin<fact_t> && pipe, virtual_container * ctr = 0) {
		*this = std::forward<pipe_begin<fact_t>>(pipe);
		set_container(ctr);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that combines two virtual chunks. Assumes that the
	/// virtual nodes are already connected. You should not use this
	/// constructor directly; instead, use the pipe operator.
	///////////////////////////////////////////////////////////////////////////
	template <typename Mid>
	virtual_chunk_begin(const virtual_chunk_begin<Mid> & left,
						const virtual_chunk<Mid, Output> & right)
		: virtual_chunk_base(left.get_node_map(),
							 bits::virt_node::combine(left.get_node(), right.get_node()))
	{
		m_recv = acc::get_destination(right);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a node and assign it to this virtual chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_begin & operator=(pipe_begin<fact_t> && pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw tpie::exception("Virtual chunk assigned twice");
		}
		typedef typename fact_t::template constructed<recv_type>::type constructed_type;
		recv_type temp(m_recv);
		this->m_nodeMap = m_recv->get_node_map();
		fact_t f = std::move(pipe.factory);
		f.set_destination_kind_push();
		this->m_node = bits::virt_node::take_own(new constructed_type(f.construct(std::move(temp))));
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename NextOutput>
	virtual_chunk_begin<NextOutput> operator|(virtual_chunk<Output, NextOutput> dest) {
		if (empty()) throw virtual_chunk_missing_begin();
		if (dest.empty()) {
			return *bits::assert_types_equal_and_return<Output, NextOutput, virtual_chunk_begin<NextOutput> *>
				::go(this);
		}
		m_recv->set_destination(acc::get_source(dest));
		return virtual_chunk_begin<NextOutput>(*this, dest);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_base operator|(virtual_chunk_end<Output> dest) {
		if (empty()) throw virtual_chunk_missing_begin();
		if (dest.empty()) throw virtual_chunk_missing_end();
		m_recv->set_destination(acc::get_source(dest));
		return virtual_chunk_base(this->m_nodeMap,
								  bits::virt_node::combine(get_node(), dest.get_node()));
	}
};

namespace bits {

template <typename Input>
virtsrc<Input> * access::get_source(const virtual_chunk_end<Input> & chunk) {
	return chunk.get_source();
}

template <typename Input, typename Output>
virtsrc<Input> * access::get_source(const virtual_chunk<Input, Output> & chunk) {
	return chunk.get_source();
}

template <typename Input, typename Output>
virtrecv<Output> * access::get_destination(const virtual_chunk<Input, Output> & chunk) {
	return chunk.get_destination();
}

template <typename Output>
virtrecv<Output> * access::get_destination(const virtual_chunk_begin<Output> & chunk) {
	return chunk.get_destination();
}

template <typename T>
class vfork_node {
public:
	template <typename dest_t>
	class type : public node {
	public:
		typedef T item_type;

		type(dest_t && dest, virtual_chunk_end<T> out)
			: vnode(out.get_node())
			, dest2(bits::access::get_source(out))
			, dest(std::move(dest))
		{
			add_push_destination(this->dest);
			if (dest2) add_push_destination(*dest2);
		}

		void push(T v) {
			dest.push(v);
			if (dest2) dest2->push(v);
		}

	private:
		// This counted reference ensures dest2 is not deleted prematurely.
		virt_node::ptr vnode;

		virtsrc<T> * dest2;

		dest_t dest;
	};
};


template <typename T>
class vpush_node : public node {
public:
	typedef T item_type;

	vpush_node(virtual_chunk_end<T> out)
		: vnode(out.get_node())
		, dest(bits::access::get_source(out))
	{
		if (dest) add_push_destination(*dest);
	}

	void push(T v) {
		if (dest) dest->push(v);
	}

private:
	// This counted reference ensures dest is not deleted prematurely.
	virt_node::ptr vnode;
	virtsrc<T> * dest;
};

} // namespace bits

template <typename T>
pipe_middle<tempfactory<bits::vfork_node<T>, virtual_chunk_end<T> > > fork_to_virtual(const virtual_chunk_end<T> & out) {
	return out;
}

template <typename T>
pipe_end<termfactory<bits::vpush_node<T>, virtual_chunk_end<T> > > push_to_virtual(const virtual_chunk_end<T> & out) {
	return out;
}
	
template <typename T>
virtual_chunk<T> vfork(const virtual_chunk_end<T> & out) {
	if (out.empty()) return virtual_chunk<T>();
	return fork_to_virtual(out);
}

template <typename T>
inline virtual_chunk<T> chunk_if(bool b, virtual_chunk<T> t) {
	if (b)
		return t;
	else
		return virtual_chunk<T>();
}

template <typename T>
virtual_chunk_end<T> chunk_end_if(bool b, virtual_chunk_end<T> t) {
	if (b)
		return t;
	else 
		return virtual_chunk_end<T>(null_sink<T>());
}

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_VIRTUAL_H__
