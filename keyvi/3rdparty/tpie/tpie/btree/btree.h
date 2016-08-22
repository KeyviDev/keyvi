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
namespace bbits {

/**
 * \brief External or internal augmented btree
 *
 */
template <typename T, typename O>
class tree {
public:
	typedef tree_state<T, O> state_type;

	static const bool is_internal = state_type::is_internal;
	static const bool is_static = state_type::is_static;
	static const bool is_ordered = state_type::is_ordered;
	
	typedef typename state_type::augmenter_type augmenter_type;

	/**
	 * \brief Type of value stored
	 */
	typedef typename state_type::value_type value_type;
	typedef typename state_type::keyextract_type keyextract_type;

	typedef typename state_type::augment_type augment_type;


	/**
	 * \brief The type of key used
	 */
	typedef typename state_type::key_type key_type;

	typedef typename O::C comp_type;

	typedef typename state_type::store_type store_type;


	/**
	 * \brief Type of node wrapper
	 */
	typedef btree_node<state_type> node_type;

	/**
	 * \brief Type of the size
	 */
	typedef typename store_type::size_type size_type;

	/**
	 * \brief Iterator type
	 */
	typedef btree_iterator<state_type> iterator;
private:
	typedef typename store_type::leaf_type leaf_type;
	typedef typename store_type::internal_type internal_type;

	
	size_t count_child(internal_type node, size_t i, leaf_type) const {
		return m_state.store().count_child_leaf(node, i);
	}

	size_t count_child(internal_type node, size_t i, internal_type) const {
		return m_state.store().count_child_internal(node, i);
	}

	internal_type get_child(internal_type node, size_t i, internal_type) const {
		return m_state.store().get_child_internal(node, i);
	}

	leaf_type get_child(internal_type node, size_t i, leaf_type) const {
		return m_state.store().get_child_leaf(node, i);
	}

	template <bool upper_bound = false, typename K>
	leaf_type find_leaf(std::vector<internal_type> & path, K k) const {
		path.clear();
		if (m_state.store().height() == 1) return m_state.store().get_root_leaf();
		internal_type n = m_state.store().get_root_internal();
		for (size_t i=2;; ++i) {
			path.push_back(n);
			for (size_t j=0; ; ++j) {
 				if (j+1 == m_state.store().count(n) ||
					(upper_bound
					 ? m_comp(k, m_state.min_key(n, j+1))
					 : !m_comp(m_state.min_key(n, j+1), k))) {
					if (i == m_state.store().height()) return m_state.store().get_child_leaf(n, j);
					n = m_state.store().get_child_internal(n, j);
					break;
				}
			}
		}
	}

	void augment(leaf_type l, internal_type p) {
		m_state.store().set_augment(l, p, m_state.m_augmenter(node_type(&m_state, l)));
	}
	
	void augment(value_type, leaf_type) {
	}


	void augment(internal_type l, internal_type p) {
		m_state.store().set_augment(l, p, m_state.m_augmenter(node_type(&m_state, l)));
	}

	size_t max_size(internal_type) const throw() {
		return m_state.store().max_internal_size();
	}

	size_t max_size(leaf_type) const throw() {
		return m_state.store().max_leaf_size();
	}

	size_t min_size(internal_type) const throw() {
		return m_state.store().min_internal_size();
	}

	size_t min_size(leaf_type) const throw() {
		return m_state.store().min_leaf_size();
	}

	template <typename N>
	N split(N left) {
		tp_assert(m_state.store().count(left) == max_size(left), "Node not full");
		size_t left_size = max_size(left)/2;
		size_t right_size = max_size(left)-left_size;
		N right = m_state.store().create(left);
		for (size_t i=0; i < right_size; ++i)
			m_state.store().move(left, left_size+i, right, i);
		m_state.store().set_count(left, left_size);
		m_state.store().set_count(right, right_size);
		return right;
	};

 	template <typename NT, typename VT>
 	void insert_part(NT n, VT v) {
		size_t z = m_state.store().count(n);
 		size_t i = z;
 		for (;i > 0 && m_comp(m_state.min_key(v), m_state.min_key(n, i-1)); --i)
 			m_state.store().move(n, i-1, n, i);
		m_state.store().set(n, i, v);
		m_state.store().set_count(n, z+1);
		augment(v, n);
 	}

	template <typename CT, typename NT>
	NT split_and_insert(CT c, NT p) {
		tp_assert(m_state.store().count(p) == max_size(p), "Node not full");
		NT p2=split(p);
		if (m_comp(m_state.min_key(c), m_state.min_key(p2)))
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
		size_t z=m_state.store().count(c);
		size_t i=m_state.store().index(c, p);
		if (i != 0 && count_child(p, i-1, c) > min_size(c)) {
			//We can steel a value from left
			CT left = get_child(p, i-1, c);
			size_t left_size = m_state.store().count(left);
			for (size_t j=0; j < z; ++j)
				m_state.store().move(c, z-j-1, c, z-j);
			m_state.store().move(left, left_size-1, c, 0);
			m_state.store().set_count(left, left_size-1);
			m_state.store().set_count(c, z+1);
			augment(c, p);
			augment(left, p);
			return true;
		}
		

		if (i +1 != m_state.store().count(p) &&
			count_child(p, i+1, c) > min_size(c)) {
			// We can steel from right
			CT right = get_child(p, i+1, c);
			size_t right_size = m_state.store().count(right);
			m_state.store().move(right, 0, c, z);
			for (size_t i=0; i+1 < right_size ; ++i)
				m_state.store().move(right, i+1, right, i);
			m_state.store().set_count(right, right_size-1);
			m_state.store().set_count(c, z+1);
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
		size_t z1 = m_state.store().count(c1);
		size_t z2 = m_state.store().count(c2);
		for (size_t i=0; i < z2; ++i)
			m_state.store().move(c2, i, c1, i+z1);
		m_state.store().set_count(c1, z1+z2);
		m_state.store().set_count(c2, 0);

		// And remove c2 from p
		size_t id = m_state.store().index(c2, p);
		size_t z_ = m_state.store().count(p) -1;
		for (; id != z_; ++id)
			m_state.store().move(p, id+1, p, id);
		m_state.store().set_count(p, z_);
		m_state.store().destroy(c2);

		augment(c1, p);
		return z_ >= min_size(p);
	}

public:
	/**
	 * \brief Returns an iterator pointing to the beginning of the tree
	 */
	iterator begin() const {
		iterator i(&m_state);
		i.goto_begin();
		return i;
	}

	/**
	 * \brief Returns an iterator pointing to the end of the tree
	 */
	iterator end() const {
		iterator i(&m_state);
		i.goto_end();
		return i;
	}

	/**
	 * \brief Insert given value into the btree
	 */
	template <typename X=enab>
	void insert(value_type v, enable<X, !is_static> =enab()) {
		m_state.store().set_size(m_state.store().size() + 1);

		// Handle the special case of the empty tree
		if (m_state.store().height() == 0) {
			leaf_type n = m_state.store().create_leaf();
			m_state.store().set_count(n, 1);
			m_state.store().set(n, 0, v);
			m_state.store().set_height(1);
			m_state.store().set_root(n);
			augment_path(n);
			return;
		}

		std::vector<internal_type> path;

		// Find the leaf contaning the value
		leaf_type l = find_leaf(path, m_state.min_key(v));
		//If there is room in the leaf
		if (m_state.store().count(l) != m_state.store().max_leaf_size()) {
			insert_part(l, v);
			if (!path.empty()) augment(l, path.back());
			augment_path(path);
			return;
		}

		// We split the leaf
		leaf_type l2 = split_and_insert(v, l);
		
		// If the leaf was a root leef we create a new root
		if (path.empty()) {
			internal_type i=m_state.store().create_internal();
			m_state.store().set_count(i, 2);
			m_state.store().set(i, 0, l);
			m_state.store().set(i, 1, l2);
			m_state.store().set_root(i);
			m_state.store().set_height(m_state.store().height()+1);
			augment(l, i);
			augment(l2, i);
			path.push_back(i);
			augment_path(path);
			return;
		}

		internal_type p = path.back();
		augment(l, p);
		
		//If there is room in the parent to insert the extra leave
		if (m_state.store().count(p) != m_state.store().max_internal_size()) {
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
			if (m_state.store().count(p) != m_state.store().max_internal_size()) {
				insert_part(p, n2);
				augment_path(path);
				return;
			}
			path.pop_back();
			n2 = split_and_insert(n2, p);
			n1 = p;

		}
		
		//We need a new root
		internal_type i=m_state.store().create_internal();
		m_state.store().set_count(i, 2);
		m_state.store().set(i, 0, n1);
		m_state.store().set(i, 1, n2);
		m_state.store().set_root(i);
		m_state.store().set_height(m_state.store().height()+1);
		augment(n1, i);
		augment(n2, i);
		path.push_back(i);
		augment_path(path);
	}

	/**
	 * \brief Return an iterator to the first item with the given key
	 */
	template <typename K, typename X=enab>
	iterator find(K v, enable<X, is_ordered> =enab()) const {
		iterator itr(&m_state);

		if(m_state.store().height() == 0) {
			itr.goto_end();
			return itr;
		}

		std::vector<internal_type> path;
		leaf_type l = find_leaf<true>(path, v);
	
		size_t i=0;
		size_t z = m_state.store().count(l);
		while (true) {
			if (i == z) {
				itr.goto_end();
				return itr;
			}
			if (!m_comp(m_state.min_key(l, i), v) &&
				!m_comp(v, m_state.min_key(l, i))) break;
			++i;
		}
		itr.goto_item(path, l, i);
		return itr;
	}

	/**
	 * \brief Return an interator to the first element that is "not less" than
	 * the given key
	 */
	template <typename K, typename X=enab>
	iterator lower_bound(K v, enable<X, is_ordered> =enab()) const {
		iterator itr(&m_state);
		if (m_state.store().height() == 0) {
			itr.goto_end();
			return itr;
		}

		std::vector<internal_type> path;
		leaf_type l = find_leaf(path, v);
		
		const size_t z = m_state.store().count(l);
		for (size_t i = 0 ; i < z ; ++i) {
			if (!m_comp(m_state.min_key(l, i), v)) {
				itr.goto_item(path, l, i);
				return itr;
			}
		}
		itr.goto_item(path, l, z-1);
		return ++itr;
	}
	
	/**
	 * \brief Return an interator to the first element that is "greater" than
	 * the given key
	 */
	template <typename K, typename X=enab>
	iterator upper_bound(K v, enable<X, is_ordered> =enab()) const {
		iterator itr(&m_state);
		if (m_state.store().height() == 0) {
			itr.goto_end();
			return itr;
		}

		std::vector<internal_type> path;
		leaf_type l = find_leaf<true>(path, v);
		
		const size_t z = m_state.store().count(l);
		for (size_t i = 0 ; i < z ; ++i) {
			if (m_comp(v, m_state.min_key(l, i))) {
				itr.goto_item(path, l, i);
				return itr;
			}
		}
		itr.goto_item(path, l, z-1);
		return ++itr;
	}

	/**
	 * \brief remove item at iterator
	 */
	template <typename X=enab>
	void erase(const iterator & itr, enable<X, !is_static> =enab()) {
		std::vector<internal_type> path=itr.m_path;
		leaf_type l = itr.m_leaf;

		size_t z = m_state.store().count(l);
		size_t i = itr.m_index;

		m_state.store().set_size(m_state.store().size()-1);
		--z;
		for (; i != z; ++i)
			m_state.store().move(l, i+1, l, i);
		m_state.store().set_count(l, z);

		// If we still have a large enough size
		if (z >= m_state.store().min_leaf_size()) {
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
				m_state.store().destroy(l);
				m_state.store().set_height(0);
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
			if (m_state.store().count(path.back()) == 1) {
				l = m_state.store().get_child_leaf(path.back(), 0);
				m_state.store().set_count(path.back(), 0);
				m_state.store().destroy(path.back());
				m_state.store().set_height(1);
				m_state.store().set_root(l);
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
				if (m_state.store().count(path.back()) == 1) {
					p = m_state.store().get_child_internal(path.back(), 0);
					m_state.store().set_count(path.back(), 0);
					m_state.store().destroy(path.back());
					m_state.store().set_height(m_state.store().height()-1);
					m_state.store().set_root(p);
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
	template <typename X=enab>
	size_type erase(key_type v, enable<X, !is_static && is_ordered> =enab()) {
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
		if (m_state.store().height() == 1) return node_type(&m_state, m_state.store().get_root_leaf());
		return node_type(&m_state, m_state.store().get_root_internal());
	}
	
	/**
	 * \brief Return the number of elements in the tree	
	 */
	size_type size() const throw() {
		return m_state.store().size();
	}

	/**
	 * \brief Check if the tree is empty
	 */
	bool empty() const throw() {
		return m_state.store().size() == 0;
	}
	
	void set_metadata(const std::string & data) {
		m_state.store().set_metadata(data);
	}
	
	std::string get_metadata() {
		return m_state.store().get_metadata();
	}
	
	/**
	 * Construct a btree with the given storage
	 */
	template <typename X=enab>
	explicit tree(std::string path, comp_type comp=comp_type(), augmenter_type augmenter=augmenter_type(), enable<X, !is_internal> =enab() ): 
		m_state(store_type(path), std::move(augmenter), keyextract_type()),
		m_comp(comp) {}

	/**
	 * Construct a btree with the given storage
	 */
	template <typename X=enab>
	explicit tree(comp_type comp=comp_type(), augmenter_type augmenter=augmenter_type(), enable<X, is_internal> =enab() ): 
		m_state(store_type(), std::move(augmenter), keyextract_type()),
		m_comp(comp) {}
	
	friend class bbits::builder<T, O>;
	
private:
	explicit tree(state_type state, comp_type comp):
		m_state(std::move(state)),
		m_comp(comp) {}

	state_type m_state;
	comp_type m_comp;
};

} //namespace bbits

template <typename T, typename ... Opts>
using btree = bbits::tree<T, typename bbits::OptComp<Opts...>::type>;

} //namespace tpie
#endif /*_TPIE_BTREE_TREE_H_*/
