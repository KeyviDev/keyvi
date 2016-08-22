// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2015 The TPIE development team
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

#ifndef _TPIE_BTREE_BTREE_BUILDER_H_
#define _TPIE_BTREE_BTREE_BUILDER_H_

#include <tpie/portability.h>
#include <tpie/btree/base.h>
#include <tpie/btree/node.h>

#include <cstddef>
#include <deque>
#include <vector>

namespace tpie {
namespace bbits {

template <typename T, typename O>
class builder {
private:
	typedef bbits::tree<T, O> tree_type;
	typedef bbits::tree_state<T, O> state_type;

	typedef typename state_type::key_type key_type;

	static const bool is_internal = state_type::is_internal;

	static const bool is_static = state_type::is_static;

	typedef T value_type;

	typedef typename O::C comp_type;

	typedef typename O::A augmenter_type;
	
    typedef typename tree_type::augment_type augment_type;

	typedef typename tree_type::size_type size_type;

	typedef typename state_type::combined_augment combined_augment;
	
	typedef typename state_type::store_type store_type;
	
	typedef typename state_type::store_type S;
	
    typedef typename S::leaf_type leaf_type;

    typedef typename S::internal_type internal_type;

    typedef btree_node<state_type> node_type;

    // Keeps the same information that the parent of a leaf keeps
    struct leaf_summary {
        leaf_type leaf;
        combined_augment augment;
    };

    // Keeps the same information that the parent of a node keeps
    struct internal_summary {
        internal_type internal;
        combined_augment augment;
    };

    // Construct a leaf from m
    void construct_leaf(size_t size) {
        leaf_summary leaf;
        leaf.leaf = m_state.store().create_leaf();

        m_state.store().set_count(leaf.leaf, size);

        for(size_t i = 0; i < size; ++i) {
            m_state.store().set(leaf.leaf, i, m_items.front());
            m_items.pop_front();
        }

		leaf.augment = m_state.m_augmenter(node_type(&m_state, leaf.leaf));
		m_state.store().flush();
        m_leaves.push_back(leaf);
    }

    void construct_internal_from_leaves(size_t size) {
        internal_summary internal;
        internal.internal = m_state.store().create_internal();

        m_state.store().set_count(internal.internal, size);

        for(size_t i = 0; i < size; ++i) {
            leaf_summary child = m_leaves.front();
            m_state.store().set(internal.internal, i, child.leaf);
            m_state.store().set_augment(child.leaf, internal.internal, child.augment);
            m_leaves.pop_front();
        }

        internal.augment = m_state.m_augmenter(node_type(&m_state, internal.internal));
		m_state.store().flush();
		
        // push the internal node to the deque of nodes
        if(m_internal_nodes.size() < 1) m_internal_nodes.push_back(std::deque<internal_summary>());
        m_internal_nodes[0].push_back(internal);
    }

    void construct_internal_from_internal(size_t size, size_t level) {
        // take nodes from internal_nodes[level] and construct a new node in internal_nodes[level+1]
        internal_summary internal;
        internal.internal = m_state.store().create_internal();
		m_state.store().set_count(internal.internal, size);
        for(size_t i = 0; i < size; ++i) {
            internal_summary child = m_internal_nodes[level].front();
            m_state.store().set(internal.internal, i, child.internal);
            m_state.store().set_augment(child.internal, internal.internal, child.augment);
            m_internal_nodes[level].pop_front();
        }
        internal.augment = m_state.m_augmenter(node_type(&m_state, internal.internal));
		m_state.store().flush();

        // push the internal node to the deque of nodes
        if(m_internal_nodes.size() < level+2) m_internal_nodes.push_back(std::deque<internal_summary>());
        m_internal_nodes[level+1].push_back(internal);
    }

	/**
	* \brief The desired number of children for each leaf node.
	*/
    static constexpr size_t desired_leaf_size() noexcept {
        return is_static
			? S::max_leaf_size()
			: ((S::min_leaf_size() + S::max_leaf_size()) / 2);
    }

	/**
	* \brief The maximum number of items to be kept in memory.
	*/
    static constexpr size_t leaf_tipping_point() noexcept {
        return desired_leaf_size() + S::min_leaf_size();
    }

	/**
	* \brief The desired number of children for each internal node.
	*/
    static constexpr size_t desired_internal_size() noexcept {
        return is_static
			? S::max_internal_size()
			: ((S::min_internal_size() + S::max_internal_size()) / 2);
    }

	/**
	* \brief The maximum number of children to be kept in memory at each level.
	*/
    static constexpr size_t internal_tipping_point() noexcept {
        return desired_internal_size() + S::min_internal_size();
    }

	/**
	* \brief Constructs a leaf. If possible, also constructs internal nodes.
	*/
	void extract_nodes() {
        construct_leaf(desired_leaf_size());

        if(m_leaves.size() < internal_tipping_point()) return;
        construct_internal_from_leaves(desired_internal_size());

        // Traverse the levels of the tree and try to construct internal nodes from other internal nodes
        for(size_t i = 0; i < m_internal_nodes.size(); ++i) {
            // if it is not possible to construct a node at this level, it is not possible at higher levels either
            if(m_internal_nodes[i].size() < internal_tipping_point()) return;
            // we have enough nodes to construct a new node.
            construct_internal_from_internal(desired_internal_size(), i);
        }
	}
public:
	/**
	* \brief Construct a btree builder with the given storage
	*/
	template <typename X=enab>
	explicit builder(std::string path, comp_type comp=comp_type(), augmenter_type augmenter=augmenter_type(), enable<X, !is_internal> =enab() )
        : m_state(store_type(path, true), std::move(augmenter), typename state_type::keyextract_type())
        , m_comp(comp)
    {}

	template <typename X=enab>
	explicit builder(comp_type comp=comp_type(), augmenter_type augmenter=augmenter_type(), enable<X, is_internal> =enab() )
		: m_state(store_type(), std::move(augmenter), typename state_type::keyextract_type())
        , m_comp(comp)
    {}

	/**
	* \brief Push a value to the builder. Values are expected to be received in order
	* \param v The value to be pushed
	*/
    void push(value_type v) {
        m_items.push_back(v);
        m_state.store().set_size(m_state.store().size() + 1); //TODO we should just set the size in the end

        // try to construct nodes from items if possible
        if(m_items.size() < leaf_tipping_point()) return;
		extract_nodes();
    }
    
	/**
	* \brief Constructs and returns a btree from the value that was pushed to the builder. The btree builder should not be used again after this point.
	*/
    tree_type build(const std::string & metadata = std::string()) {
        // finish building the tree by traversing all levels and constructing leaves/nodes

        // construct one or two leaves if neccesary
        if(m_items.size() > 0) {
            if(m_items.size() > S::max_leaf_size()) // construct two leaves if necessary
                construct_leaf(m_items.size()/2);
            construct_leaf(m_items.size()); // construct a leaf with the remaining items
        }

        // if there already exists internal nodes and there are leaves left: construct a new internal node(since there is guaranteed to be atleast S::min_internal_size leaves)
        // if there do not exist internal nodes, then only construct an internal node if there is more than one leaf
        if((m_internal_nodes.size() == 0 && m_leaves.size() > 1) || (m_internal_nodes.size() > 0 && m_leaves.size() > 0)) {
            if(m_leaves.size() > 2*S::max_internal_size() ) // construct two nodes if necessary
                construct_internal_from_leaves(m_leaves.size()/3);
            if(m_leaves.size() > S::max_internal_size()) // construct two nodes if necessary
                construct_internal_from_leaves(m_leaves.size()/2);
            construct_internal_from_leaves(m_leaves.size()); // construct a node from the remaining leaves
        }

        for(size_t i = 0; i < m_internal_nodes.size(); ++i) {
            // if there already exists internal nodes at a higher level and there are nodes at this level left: construct a new internal node at the higher level.
            // if there do not exist internal nodes at a higher level, then only construct an internal nodes at that level if there are more than one node at this level.
            if((m_internal_nodes.size() == i+1 && m_internal_nodes[i].size() > 1)
                || (m_internal_nodes.size() > i+1 && m_internal_nodes[i].size() > 0)) {
                if(m_internal_nodes[i].size() > 2*S::max_internal_size())
                    construct_internal_from_internal(m_internal_nodes[i].size()/3, i);
                if(m_internal_nodes[i].size() > S::max_internal_size())
                    construct_internal_from_internal(m_internal_nodes[i].size()/2, i);
                construct_internal_from_internal(m_internal_nodes[i].size(), i);
            }
        }

        // find the root and set it as such

        if(m_internal_nodes.size() == 0 && m_leaves.size() == 0) // no items were pushed
            m_state.store().set_height(0);
        else {
            m_state.store().set_height(m_internal_nodes.size() + 1);
            if(m_leaves.size() == 1)
                m_state.store().set_root(m_leaves.front().leaf);
            else
                m_state.store().set_root(m_internal_nodes.back().front().internal);
        }
		if (metadata.size()) {
			m_state.store().flush();
			m_state.store().set_metadata(metadata);
		}
		m_state.store().finalize_build();
        return tree_type(std::move(m_state), std::move(m_comp));
    }

private:
    std::deque<value_type> m_items;
    std::deque<leaf_summary> m_leaves;
    std::vector<std::deque<internal_summary>> m_internal_nodes;

	state_type m_state;
    comp_type m_comp;
};

} //namespace bbits

template <typename T, typename ... Opts>
using btree_builder = bbits::builder<T, typename bbits::OptComp<Opts...>::type>;

} //namespace tpie
#endif /*_TPIE_BTREE_BTREE_BUILDER_H_*/
