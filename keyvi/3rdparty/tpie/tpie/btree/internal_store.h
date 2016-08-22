// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The TPIE development team
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

#ifndef _TPIE_BTREE_INTERNAL_STORE_H_
#define _TPIE_BTREE_INTERNAL_STORE_H_

#include <tpie/portability.h>
#include <tpie/btree/base.h>
#include <tpie/tpie_assert.h>
#include <cstddef>

namespace tpie {
namespace bbits {

/**
 * \brief Storage used for an internal btree. Note that a user of a btree should
 * not call the store directly
 *
 * \tparam T the type of value stored
 * \tparam A the type of augmentation
 * \tparam a the minimum fanout of a node
 * \tparam b the maximum fanout of a node
 */
template <typename T,
		  typename A,
		  std::size_t a_,
		  std::size_t b_
		  >
class internal_store {
public:
	static const size_t a = a_?a_:2;
	static const size_t b = b_?b_:4;
	
	/**
	 * \brief Type of value of items stored
	 */
	typedef T value_type;

	/**
	 * \brief Type of augmentation stored
	 */
	typedef A augment_type;


	typedef size_t size_type;


	internal_store(const internal_store & o) = delete;
	internal_store & operator=(const internal_store & o) = delete;

	internal_store(internal_store && o)
		: m_root(o.m_root)
		, m_height(o.m_height)
		, m_size(o.m_size) {
		o.m_root = nullptr;
		o.m_size = 0;
		o.m_height = 0;
	}
		
	internal_store & operator=(internal_store && o) {
		this->~internal_store();
		new (this) internal_store(o);
		return this;
	}
	
private:	
	/**
	 * \brief Construct a new empty btree storage
	 */
	explicit internal_store(): 
		m_root(nullptr), m_height(0), m_size(0) {}

	
	struct internal_content {
		void * ptr;
		A augment;
	};

	struct internal {
		size_t count;
		internal_content values[b];
	};
	
	struct leaf {
		size_t count;
		T values[b];
	};

	typedef internal * internal_type;
	typedef leaf * leaf_type;
	
	void dispose(void * node, size_t depth) {
		if (depth + 1 == m_height) {
			delete static_cast<leaf *>(node);
			return;
		}
		internal * in = static_cast<internal*>(node);
		for (size_t i=0; i != in->count; ++i)
			dispose(in->values[i].ptr, depth + 1);
		delete in;
	}
	
	~internal_store() {
		if (!m_root || !m_height) return;
		dispose(m_root, 0);
		m_root = nullptr;
	}
	
	static constexpr size_t min_internal_size() {return a;}
	static constexpr size_t max_internal_size() {return b;}

	static constexpr size_t min_leaf_size() {return a;}
	static constexpr size_t max_leaf_size() {return b;}
	
	void move(internal_type src, size_t src_i,
			  internal_type dst, size_t dst_i) {
		dst->values[dst_i] = src->values[src_i];
	}

	void move(leaf_type src, size_t src_i,
			  leaf_type dst, size_t dst_i) {
		dst->values[dst_i] = src->values[src_i];
	}

	void set(leaf_type dst, size_t dst_i, T c) {
		dst->values[dst_i] = c;
	}
		
	void set(internal_type node, size_t i, internal_type c) {
		node->values[i].ptr = c;
	}

	void set(internal_type node, size_t i, leaf_type c) {
		node->values[i].ptr = c;
	}

	const T & get(leaf_type l, size_t i) const {
		return l->values[i];
	}

	size_t count(internal_type node) const {
		return node->count;
	}

	size_t count_child_leaf(internal_type node, size_t i) const {
		return static_cast<leaf_type>(node->values[i].ptr)->count;
	}

	size_t count_child_internal(internal_type node, size_t i) const {
		return static_cast<internal_type>(node->values[i].ptr)->count;
	}

	size_t count(leaf_type node) const {
		return node->count;
	}

	void set_count(internal_type node, size_t i) {
		node->count = i;
	}

	void set_count(leaf_type node, size_t i) {
		node->count = i;
	}

	leaf_type create_leaf() {return new leaf();}
	leaf_type create(leaf_type) {return create_leaf();}
	internal_type create_internal() {return new internal();}
	internal_type create(internal_type) {return create_internal();}

	void destroy(internal_type node) {delete node;}
	void destroy(leaf_type node) {delete node;}

	void set_root(internal_type node) {m_root = node;}
	void set_root(leaf_type node) {m_root = node;}

	internal_type get_root_internal() const {
		return static_cast<internal_type>(m_root);
	}

	leaf_type get_root_leaf() const {
		return static_cast<leaf_type>(m_root);
	}

	internal_type get_child_internal(internal_type node, size_t i) const {
		return static_cast<internal_type>(node->values[i].ptr);
	}

	leaf_type get_child_leaf(internal_type node, size_t i) const {
		return static_cast<leaf_type>(node->values[i].ptr);
	}

	size_t index(void * child, internal_type node) const {
		for (size_t i=0; i < node->count; ++i)
			if (node->values[i].ptr == child) return i;
		tp_assert(false, "Not found");
		tpie_unreachable();
	}

	void set_augment(void * l, internal_type p, augment_type ag) {
		size_t idx=index(l, p);
		p->values[idx].augment = ag;
	}

	const augment_type & augment(internal_type p, size_t i) const {
		return p->values[i].augment;
	}
	
	size_t height() const throw() {
		return m_height;
	}

	void set_height(size_t height) throw() {
		m_height = height;
	}

	size_t size() const throw() {
		return m_size;
	}

	void set_size(size_t size) throw() {
		m_size = size;
	}
	
	void flush() {}
	void finalize_build() {}

	void set_metadata(const std::string & data) {
		metadata = data;
	}
	
	std::string get_metadata() {
		return metadata;
	}
	
	void * m_root;
	size_t m_height;
	size_t m_size;
	std::string metadata;

	template <typename>
	friend class ::tpie::btree_node;

	template <typename>
	friend class ::tpie::btree_iterator;

	template <typename, typename>
	friend class bbits::tree;

	template <typename, typename>
	friend class bbits::tree_state;

    template<typename, typename>
    friend class bbits::builder;

	
};

} //namespace bbits
} //namespace tpie
#endif /*_TPIE_BTREE_INTERNAL_STORE_H_*/
