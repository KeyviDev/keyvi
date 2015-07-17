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

#ifndef _TPIE_BTREE_TREE_H_
#define _TPIE_BTREE_TREE_H_

#include <tpie/portability.h>
#include <tpie/btree/base.h>
#include <tpie/btree/node.h>

#include <cstddef>
#include <vector>

namespace tpie {

/**
 * \brief External or internal augmented btree
 *
 * S is the type of the storage object
 * C is the comparator used to compare keys
 * A is the functor used to computed an augmented value
 */
template <typename S,
		  typename C,
		  typename A>
class btree {
public:
	/**
	 * \brief The type of key used
	 */
	typedef typename S::key_type key_type;

	/**
	 * \brief Type of node wrapper
	 */
	typedef btree_node<S> node_type;

	/**
	 * \brief Type of value stored
	 */
	typedef typename S::value_type value_type;

	/**
	 * \brief Type of augmenter
	 */
	typedef A augmenter_type;


	/**
	 * \brief Type of the size
	 */
	typedef typename S::size_type size_type;

	/**
	 * \brief Iterator type
	 */
	typedef btree_iterator<S> iterator;
private:
	typedef typename S::leaf_type leaf_type;
	typedef typename S::internal_type internal_type;

	
	size_t count_child(internal_type node, size_t i, leaf_type) const {
		return m_store.count_child_leaf(node, i);
	}

	size_t count_child(internal_type node, size_t i, internal_type) const {
		return m_store.count_child_internal(node, i);
	}

	internal_type get_child(internal_type node, size_t i, internal_type) const {
		return m_store.get_child_internal(node, i);
	}

	leaf_type get_child(internal_type node, size_t i, leaf_type) const {
		return m_store.get_child_leaf(node, i);
	}

	leaf_type find_leaf(std::vector<internal_type> & path, key_type k) const {
		path.clear();
		if (m_store.height() == 1) return m_store.get_root_leaf();
		internal_type n = m_store.get_root_internal();
		for (size_t i=2;; ++i) {
			path.push_back(n);
			for (size_t j=0; ; ++j) {
 				if (j+1 == m_store.count(n) ||
					m_comp(k, m_store.min_key(n, j+1))) {
					if (i == m_store.height()) return m_store.get_child_leaf(n, j);
					n = m_store.get_child_internal(n, j);
					break;
				}
			}
		}
	}

	void augment(leaf_type l, internal_type p) {
		m_store.set_augment(l, p, m_augmenter(node_type(&m_store, l)));
	}
	
	void augment(value_type, leaf_type) {
	}


	void augment(internal_type l, internal_type p) {
		m_store.set_augment(l, p, m_augmenter(node_type(&m_store, l)));
	}

	size_t max_size(internal_type) const throw() {
		return m_store.max_internal_size();
	}

	size_t max_size(leaf_type) const throw() {
		return m_store.max_leaf_size();
	}

	size_t min_size(internal_type) const throw() {
		return m_store.min_internal_size();
	}

	size_t min_size(leaf_type) const throw() {
		return m_store.min_leaf_size();
	}

	template <typename N>
	N split(N left) {
		tp_assert(m_store.count(left) == max_size(left), "Node not full");
		size_t left_size = max_size(left)/2;
		size_t right_size = max_size(left)-left_size;
		N right = m_store.create(left);
		for (size_t i=0; i < right_size; ++i)
			m_store.move(left, left_size+i, right, i);
		m_store.set_count(left, left_size);
		m_store.set_count(right, right_size);
		return right;
	};

 	template <typename NT, typename VT>
 	void insert_part(NT n, VT v) {
		size_t z = m_store.count(n);
 		size_t i = z;
 		for (;i > 0 && m_comp(m_store.min_key(v), m_store.min_key(n, i-1)); --i)
 			m_store.move(n, i-1, n, i);
		m_store.set(n, i, v);
		m_store.set_count(n, z+1);
		augment(v, n);
 	}

	template <typename CT, typename NT>
	NT split_and_insert(CT c, NT p) {
		tp_assert(m_store.count(p) == max_size(p), "Node not full");
		NT p2=split(p);
		if (m_comp(m_store.min_key(c), m_store.min_key(p2)))
			insert_part(p, c);
		else
			insert_part(p2, c);
		return p2;
	}

	void augment_path(leaf_type) {
		//NOOP
	}

	void augment_path(std::vector<internal_type> & path) {
		while (path.size() >= 2) {
			internal_type c=path.back();
			path.pop_back();
			augment(c, path.back());
		}
		path.pop_back();
	}
	
	template <typename CT, typename PT>
	bool remove_fixup_round(CT c, PT p) {
		size_t z=m_store.count(c);
		size_t i=m_store.index(c, p);
		if (i != 0 && count_child(p, i-1, c) > min_size(c)) {
			//We can steel a value from left
			CT left = get_child(p, i-1, c);
			size_t left_size = m_store.count(left);
			for (size_t j=0; j < z; ++j)
				m_store.move(c, z-j-1, c, z-j);
			m_store.move(left, left_size-1, c, 0);
			m_store.set_count(left, left_size-1);
			m_store.set_count(c, z+1);
			augment(c, p);
			augment(left, p);
			return true;
		}
		

		if (i +1 != m_store.count(p) &&
			count_child(p, i+1, c) > min_size(c)) {
			// We can steel from right
			CT right = get_child(p, i+1, c);
			size_t right_size = m_store.count(right);
			m_store.move(right, 0, c, z);
			for (size_t i=0; i+1 < right_size ; ++i)
				m_store.move(right, i+1, right, i);
			m_store.set_count(right, right_size-1);
			m_store.set_count(c, z+1);
			augment(c, p);
			augment(right, p);
			return true;
		}

		CT c1;
		CT c2;
		if (i == 0) {
			c1 = c;
			c2 = get_child(p, i+1, c);
		} else {
			c1 = get_child(p, i-1, c);
			c2 = c;
		}

		//Merge l2 into l1
		size_t z1 = m_store.count(c1);
		size_t z2 = m_store.count(c2);
		for (size_t i=0; i < z2; ++i)
			m_store.move(c2, i, c1, i+z1);
		m_store.set_count(c1, z1+z2);
		m_store.set_count(c2, 0);

		// And remove c2 from p
		size_t id = m_store.index(c2, p);
		size_t z_ = m_store.count(p) -1;
		for (; id != z_; ++id)
			m_store.move(p, id+1, p, id);
		m_store.set_count(p, z_);
		m_store.destroy(c2);

		augment(c1, p);
		return z_ >= min_size(p);
	}

public:
	/**
	 * \brief Returns an iterator pointing to the beginning of the tree
	 */
	iterator begin() const {
		iterator i(&m_store);
		i.goto_begin();
		return i;
	}

	/**
	 * \brief Returns an iterator pointing to the end of the tree
	 */
	iterator end() const {
		iterator i(&m_store);
		i.goto_end();
		return i;
	}

	/**
	 * \brief Insert given value into the btree
	 */
	void insert(value_type v) {
		// Handle the special case of the empty tree
		m_store.set_size(m_store.size() + 1);
		
		if (m_store.height() == 0) {
			leaf_type n = m_store.create_leaf();
			m_store.set_count(n, 1);
			m_store.set(n, 0, v);
			m_store.set_height(1);
			m_store.set_root(n);
			augment_path(n);
			return;
		}

		std::vector<internal_type> path;

		// Find the leaf contaning the value
		leaf_type l = find_leaf(path, m_store.min_key(v));
		//If there is room in the leaf
		if (m_store.count(l) != m_store.max_leaf_size()) {
			insert_part(l, v);
			if (!path.empty()) augment(l, path.back());
			augment_path(path);
			return;
		}

		// We split the leaf
		leaf_type l2 = split_and_insert(v, l);
		
		// If the leaf was a root leef we create a new root
		if (path.empty()) {
			internal_type i=m_store.create_internal();
			m_store.set_count(i, 2);
			m_store.set(i, 0, l);
			m_store.set(i, 1, l2);
			m_store.set_root(i);
			m_store.set_height(m_store.height()+1);
			augment(l, i);
			augment(l2, i);
			path.push_back(i);
			augment_path(path);
			return;
		}

		internal_type p = path.back();
		augment(l, p);
		
		//If there is room in the parent to insert the extra leave
		if (m_store.count(p) != m_store.max_internal_size()) {
			insert_part(p, l2);
			augment_path(path);
			return;
		}

		path.pop_back();
		internal_type n2 = split_and_insert(l2, p);
		internal_type n1 = p;
		
		while (!path.empty()) {
			internal_type p = path.back();
			augment(n1, p);
			if (m_store.count(p) != m_store.max_internal_size()) {
				insert_part(p, n2);
				augment_path(path);
				return;
			}
			path.pop_back();
			n2 = split_and_insert(n2, p);
			n1 = p;

		}
		
		//We need a new root
		internal_type i=m_store.create_internal();
		m_store.set_count(i, 2);
		m_store.set(i, 0, n1);
		m_store.set(i, 1, n2);
		m_store.set_root(i);
		m_store.set_height(m_store.height()+1);
		augment(n1, i);
		augment(n2, i);
		path.push_back(i);
		augment_path(path);
	}

	/**
	 * \brief Return an iterator to the first item with the given key
	 */
	iterator find(key_type v) const {
		iterator itr(&m_store);

		if(m_store.height() == 0) {
			itr.goto_end();
			return itr;
		}

		std::vector<internal_type> path;
		leaf_type l = find_leaf(path, v);

		size_t i=0;
		size_t z = m_store.count(l);
		while (true) {
			if (i == z) {
				itr.goto_end();
				return itr;
			}
			if (!m_comp(m_store.min_key(l, i), v) &&
				!m_comp(v, m_store.min_key(l, i))) break;
			++i;
		}
		itr.goto_item(path, l, i);
		return itr;
	}
	
	/**
	 * \brief remove item at iterator
	 */
	void erase(const iterator & itr) {
		std::vector<internal_type> path=itr.m_path;
		leaf_type l = itr.m_leaf;

		size_t z = m_store.count(l);
		size_t i = itr.m_index;

		m_store.set_size(m_store.size()-1);
		--z;
		for (; i != z; ++i)
			m_store.move(l, i+1, l, i);
		m_store.set_count(l, z);

		// If we still have a large enough size
		if (z >= m_store.min_leaf_size()) {
			// We are the lone root
			if (path.empty()) {
				augment_path(l);
				return;
			}
			augment(l, path.back());
			augment_path(path);
			return;
		}

		// We are too small but the root
		if (path.empty()) {
			// We are now the empty tree
			if (z == 0) {
				m_store.destroy(l);
				m_store.set_height(0);
				return;
			}
			augment_path(l);
			return;
		}

		// Steal or merge
		if (remove_fixup_round(l, path.back())) {
			augment_path(path);
			return;
		}
		
		// If l is now the only child of the root, make l the root
		if (path.size() == 1) {
			if (m_store.count(path.back()) == 1) {
				l = m_store.get_child_leaf(path.back(), 0);
				m_store.set_count(path.back(), 0);
				m_store.destroy(path.back());
				m_store.set_height(1);
				m_store.set_root(l);
				augment_path(l);
				return;
			}
			augment_path(path);
			return;
		}
		
		while (true) {
			// We need to handle the parent
			internal_type p = path.back();
			path.pop_back();
			if (remove_fixup_round(p, path.back())) {
				augment_path(path);
				return;
			}
			
			if (path.size() == 1) {
				if (m_store.count(path.back()) == 1) {
					p = m_store.get_child_internal(path.back(), 0);
					m_store.set_count(path.back(), 0);
					m_store.destroy(path.back());
					m_store.set_height(m_store.height()-1);
					m_store.set_root(p);
					path.clear();
					path.push_back(p);
					augment_path(path);
					return;
				}
				augment_path(path);
				return;
			}
		}
	}
		
	/**
	 * \brief remove all items with given key
	 */
	size_type erase(key_type v) {
		size_type count = 0;
		iterator i = find(v);
		while(i != end()) {
			erase(i);
			++count;
			i = find(v);
		}

		return count;
	}

	/**
	 * \brief Return the root node
	 * \pre !empty()
	 */
	node_type root() const {
		if (m_store.height() == 1) return node_type(&m_store, m_store.get_root_leaf());
		return node_type(&m_store, m_store.get_root_internal());
	}
	
	/**
	 * \brief Return the number of elements in the tree	
	 */
	size_type size() const throw() {
		return m_store.size();
	}

	/**
	 * \brief Check if the tree is empty
	 */
	bool empty() const throw() {
		return m_store.size() == 0;
	}

	/**
	 * Construct a btree with the given storage
	 */
	btree(S store=S(), C comp=C(), A augmenter=A()): 
		m_store(store), 
		m_comp(comp), 
		m_augmenter(augmenter) {}

private:
	S m_store;
	C m_comp;
	A m_augmenter;
};

} //namespace tpie
#endif /*_TPIE_BTREE_TREE_H_*/
