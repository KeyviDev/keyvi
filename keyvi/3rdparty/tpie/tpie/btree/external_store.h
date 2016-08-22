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

#ifndef _TPIE_BTREE_EXTERNAL_STORE_H_
#define _TPIE_BTREE_EXTERNAL_STORE_H_

#include <tpie/portability.h>
#include <tpie/btree/base.h>
#include <tpie/tpie_assert.h>
#include <tpie/blocks/block_collection_cache.h>
#include <tpie/btree/external_store_base.h>
#include <memory>

#include <cstddef>

namespace tpie {
namespace bbits {

/**
 * \brief Storage used for an external btree. Note that a user of a btree should
 * not call the store directly.
 * 
 * \tparam T the type of value stored
 * \tparam A the type of augmentation
 */
template <typename T,
		  typename A,
		  std::size_t fanout_a,
		  std::size_t fanout_b>
class external_store : public external_store_base {
public:
	/**
	 * \brief Type of value of items stored
	 */
	typedef T value_type;

	/**
	 * \brief Type of augmentation stored
	 */
	typedef A augment_type;

	typedef size_t size_type;

	static constexpr memory_size_type cacheSize() {return 32;}
	static constexpr memory_size_type blockSize() {return 7000;}
	
	struct internal_content {
		blocks::block_handle handle;
		A augment;
	};

	struct internal {
		internal(blocks::block * b) {
			count = reinterpret_cast<memory_size_type *>(b->get());
			values = reinterpret_cast<internal_content *>(b->get() + sizeof(memory_size_type));
		}

		memory_size_type * count;
		internal_content * values;
	};
	
	struct leaf {
		leaf(blocks::block * b) {
			count = reinterpret_cast<memory_size_type *>(b->get());
			values = reinterpret_cast<T *>(b->get() + sizeof(memory_size_type));
		}

		memory_size_type * count;
		T * values;
	};

	struct internal_type {
		internal_type() {}
		internal_type(blocks::block_handle handle) : handle(handle) {}

		blocks::block_handle handle;

		bool operator==(const internal_type & other) const {
			return handle == other.handle;
		}
	};

	struct leaf_type {
		leaf_type() {}
		leaf_type(blocks::block_handle handle) : handle(handle) {}

		blocks::block_handle handle;

		bool operator==(const leaf_type & other) const {
			return handle == other.handle;
		}
	};

	/**
	 * \brief Construct a new empty btree storage
	 */
	explicit external_store(const std::string & path, bool /*write_only*/=false) //TODO maybe use this?
	: external_store_base(path)
	{
		m_collection = std::make_shared<blocks::block_collection_cache>(
			path, blockSize(), cacheSize(), true);
	}

	~external_store() {
		m_collection.reset();
	}

	static constexpr size_t min_internal_size() {
		return fanout_a?fanout_a:(max_internal_size() + 3) / 4;
	}

	static constexpr size_t max_internal_size() {
		return fanout_b?fanout_b:(blockSize() - sizeof(memory_size_type)) / sizeof(internal_content);
	}

	static constexpr size_t min_leaf_size() {
		return fanout_a?fanout_a:(max_leaf_size() + 3) / 4;
	}

	static constexpr size_t max_leaf_size() {
		return fanout_b?fanout_b:(blockSize() - sizeof(memory_size_type)) / sizeof(T);
	}
	
	void move(internal_type src, size_t src_i,
			  internal_type dst, size_t dst_i) {
		blocks::block * srcBlock = m_collection->read_block(src.handle);
		blocks::block * dstBlock = m_collection->read_block(dst.handle);

		internal srcInter(srcBlock);
		internal dstInter(dstBlock);

		dstInter.values[dst_i] = srcInter.values[src_i];

		m_collection->write_block(src.handle);
		m_collection->write_block(dst.handle);
	}

	void move(leaf_type src, size_t src_i,
			  leaf_type dst, size_t dst_i) {
		blocks::block * srcBlock = m_collection->read_block(src.handle);
		blocks::block * dstBlock = m_collection->read_block(dst.handle);

		leaf srcInter(srcBlock);
		leaf dstInter(dstBlock);

		dstInter.values[dst_i] = srcInter.values[src_i];

		m_collection->write_block(src.handle);
		m_collection->write_block(dst.handle);
	}

	void set(leaf_type dst, size_t dst_i, T c) {
		blocks::block * dstBlock = m_collection->read_block(dst.handle);
		leaf dstInter(dstBlock);

		dstInter.values[dst_i] = c;

		m_collection->write_block(dst.handle);
	}
		
	void set(internal_type node, size_t i, internal_type c) {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal nodeInter(nodeBlock);

		nodeInter.values[i].handle = c.handle;

		m_collection->write_block(node.handle);
	}

	void set(internal_type node, size_t i, leaf_type c) {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal nodeInter(nodeBlock);

		nodeInter.values[i].handle = c.handle;

		m_collection->write_block(node.handle);
	}

	const T & get(leaf_type node, size_t i) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		leaf nodeInter(nodeBlock);

		return nodeInter.values[i];
	}

	size_t count(internal_type node) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal nodeInter(nodeBlock);

		return *(nodeInter.count);
	}

	size_t count(leaf_type node) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		leaf nodeInter(nodeBlock);

		return *(nodeInter.count);
	}

	size_t count_child_leaf(internal_type node, size_t i) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal nodeInter(nodeBlock);

		leaf_type wrap(nodeInter.values[i].handle);
		return count(wrap);	
	}

	size_t count_child_internal(internal_type node, size_t i) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal nodeInter(nodeBlock);

		internal_type wrap(nodeInter.values[i].handle);
		return count(wrap);
	}

	void set_count(internal_type node, size_t i) {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal nodeInter(nodeBlock);

		*(nodeInter.count) = i;

		m_collection->write_block(node.handle);
	}

	void set_count(leaf_type node, size_t i) {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		leaf nodeInter(nodeBlock);

		*(nodeInter.count) = i;

		m_collection->write_block(node.handle);
	}

	leaf_type create_leaf() {
		blocks::block_handle h = m_collection->get_free_block();
		blocks::block * b = m_collection->read_block(h);
		leaf l(b);
		(*l.count) = 0;
		m_collection->write_block(h);
		return leaf_type(h);
	}

	leaf_type create(leaf_type) {
		return create_leaf();
	}

	internal_type create_internal() {
		blocks::block_handle h = m_collection->get_free_block();
		blocks::block * b = m_collection->read_block(h);
		internal i(b);
		(*i.count) = 0;
		m_collection->write_block(h);
		return internal_type(h);
	}

	internal_type create(internal_type) {
		return create_internal();
	}

	void destroy(internal_type node) {
		m_collection->free_block(node.handle);
	}

	void destroy(leaf_type node) {
		m_collection->free_block(node.handle);
	}

	void set_root(internal_type node) {
		m_root = node.handle;
	}

	void set_root(leaf_type node) {
		m_root = node.handle;
	}

	internal_type get_root_internal() const throw() {
		return internal_type(m_root);
	}

	leaf_type get_root_leaf() const throw() {
		return leaf_type(m_root);
	}

	internal_type get_child_internal(internal_type node, size_t i) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal dstInter(nodeBlock);

		return internal_type(dstInter.values[i].handle);
	}

	leaf_type get_child_leaf(internal_type node, size_t i) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal dstInter(nodeBlock);

		return leaf_type(dstInter.values[i].handle);
	}

	size_t index(leaf_type child, internal_type node) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal dstInter(nodeBlock);

		for (size_t i=0; i < *(dstInter.count); ++i)
			if (dstInter.values[i].handle == child.handle) return i;
		tp_assert(false, "Leaf not found");
		tpie_unreachable();
	}

	size_t index(internal_type child, internal_type node) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal dstInter(nodeBlock);

		for (size_t i=0; i < *(dstInter.count); ++i)
			if (dstInter.values[i].handle == child.handle) return i;
		tp_assert(false, "Node not found");
		tpie_unreachable();
	}
	
	void set_augment(blocks::block_handle child, internal_type node, augment_type augment) {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal nodeInter(nodeBlock);

		for (size_t i=0; i < *(nodeInter.count); ++i)
		{
			if (nodeInter.values[i].handle == child) {
				nodeInter.values[i].augment = augment;
				m_collection->write_block(node.handle);
				return;
			}
		}

		tp_assert(false, "Not found");
		tpie_unreachable();
	}
	

	void set_augment(leaf_type child, internal_type node, augment_type augment) {
		set_augment(child.handle, node, augment);
	}

	void set_augment(internal_type child, internal_type node, augment_type augment) {
		set_augment(child.handle, node, augment);
	}

	const augment_type & augment(internal_type node, size_t i) const {
		blocks::block * nodeBlock = m_collection->read_block(node.handle);
		internal nodeInter(nodeBlock);

		return nodeInter.values[i].augment;
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
		throw exception("Not yet implemnted.");
	}
	
	std::string get_metadata() {
		throw exception("Not yet implemnted.");
	}

	std::shared_ptr<blocks::block_collection_cache> m_collection;

	template <typename>
	friend class btree_node;

	template <typename>
	friend class btree_iterator;

	template <typename, typename>
	friend class bbits::tree_state;
	
	template <typename, typename>
	friend class bbits::tree;

    template <typename, typename>
    friend class bbits::builder;
};

} //namespace bbits
} //namespace tpie
#endif /*_TPIE_BTREE_EXTERNAL_STORE_H_*/
