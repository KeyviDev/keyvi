// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2015, The TPIE development team
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
#ifndef __TPIE_TINY_H__
#define __TPIE_TINY_H__

#include <algorithm>
#include <stdexcept>
#include <tpie/config.h>

namespace tpie {
namespace tiny {

/**
 * \brief Sorts the elements in the range [first,last) into ascending order.
 *
 * This uses insertion sort which has a complexity of O(n^2). Use this only 
 * to sort tiny amounts of data (<= 200)
 */
template <typename T, typename Comp>
void sort(T start, T end, Comp comp=Comp()) {
	for (T i = start; i != end; ++i)
		std::rotate(std::upper_bound(start, i, *i, comp), i, std::next(i));
}

/**
 * \brief Sorts the elements in the range [first,last) into ascending order.
 *
 * This uses insertion sort which has a complexity of O(n^2). Use this only 
 * to sort tiny amounts of data (<= 200)
 */
template <typename T>
void sort(T start, T end) {
	for (T i = start; i != end; ++i)
		std::rotate(std::upper_bound(start, i, *i), i, std::next(i));
}

namespace bits {
/**
 * \brief Use the identity function on elements to extract the key.
 */
class IdentityExtract {
public:
	template <typename T>
	const T & operator()(const T & x) const {return x;}
};

/**
 * \brief Extract the first element of a pair as the key.
 */
template <typename A, typename B>
class PairExtract {
public:
	template <typename T>
	const T & operator()(const T & x) const {return x;}
	const A & operator()(const std::pair<A, B> & x) const {return x.first;}
};

/**
 * \brief When inserting do not allow elements with equivalest keys.
 */
struct SingleInsertHelp {
	template <typename Inner>
	struct type {
		static const bool multi=false;
		typedef typename Inner::iterator iterator;
		typedef std::pair<iterator, bool> result;

		template <typename Comp>
		static result handle(Inner & inner, const Comp & comp) {
			iterator fend=std::prev(inner.end());
			iterator i=std::lower_bound(inner.begin(), fend, *fend, comp);
			if (i != fend && !comp(*fend, *i)) {
				inner.pop_back();
				return std::make_pair(i, false);
			}
			std::rotate(i, fend, std::next(fend));
			return std::make_pair(i, true);
		}
	};
};

/**
 * \brief When inserting allow elements with equivalest keys.
 */
struct MultiInsertHelp {
	template <typename Inner>
	struct type {
		static const bool multi=true;
		typedef typename Inner::iterator iterator;
		typedef iterator result;
		
		template <typename Comp>
		static result handle(Inner & inner, const Comp &) {
			iterator y=std::upper_bound(inner.begin(), inner.end()-1, inner.back());
			std::rotate(y, inner.end()-1, inner.end());
			return y;
		}
	};
};

} //namespace bits
/**
 * \brief class implementing a tiny::set, tiny::multiset, and
 * tiny::multimap. Also serve as a base class for the tiny::map.
 *
 * A tiny container is just a sorted vector of elements. When you have
 * less then 512 elements in a std::set using a tiny::set will often
 * be faster. Also much less code will be generated, so compiletime
 * will improve.
 *
 * Note that unlike in containers based on a red black tree, iterators
 * may be invalidated by insert. To avoid this use reserve, and check
 * the capacity.
 */
template <typename T,
		  typename Key,
		  typename KeyExtract,
		  typename Comp,
		  typename Alloc,
		  typename InsertHelp>
class set_impl {
protected:
	typedef std::vector<T, Alloc> Inner;
	typedef typename InsertHelp::template type<Inner> IH;
public:
	typedef T value_type;
	typedef Key key_type;
	typedef typename Inner::iterator iterator;
	typedef typename Inner::const_iterator const_iterator;
	typedef typename Inner::reverse_iterator reverse_iterator;
	typedef typename Inner::const_reverse_iterator const_reverse_iterator;
	typedef Comp key_compare;
	typedef Alloc allocator_type;
	typedef typename IH::result insert_result;
	
	struct value_compare {
		template <typename L, typename R>
		bool operator()(const L & l, const R & r) const {
			return comp(extract(l), extract(r));
		}
		Comp comp;
		KeyExtract extract;
		value_compare(Comp comp): comp(comp) {}
	};

	/**
	 * \brief Default constructor. Constructs empty container.
	 */
	set_impl(): comp(Comp()) {}
	
	/**
	 * \brief Default constructor. Constructs empty container.
	 */
	explicit set_impl(const Comp & comp, const Alloc & alloc=Alloc()): comp(comp), inner(alloc) {}

	/**
	 * \brief Default constructor. Constructs empty container.
	 */
	explicit set_impl(const Alloc & alloc): inner(alloc) {}

	/**
	 * \brief Copy constructor. Constructs the container with the copy of the contents of other. 
	 */
	set_impl(const set_impl & other): comp(other.comp), inner(other.inner) {}

	/**
	 * \brief Copy constructor. Constructs the container with the copy of the contents of other. 
	 */
	set_impl(const set_impl & other, const Alloc & alloc): comp(other.comp), inner(other.inner, alloc) {}

	/**
	 * \brief Move constructor. Constructs the container with the contents of other using move semantics.
	 */
	set_impl(set_impl && other): comp(std::move(other.comp)), inner(std::move(other.inner)) {}

	/**
	 * \brief Move constructor. Constructs the container with the contents of other using move semantics.
	 */
	set_impl(set_impl && other, const Alloc & alloc): comp(std::move(other.comp)), inner(std::move(other.inner), alloc) {}

	/**
	 * \brief Constructs the container with the contents of the range [first, last).
	 */
	template< class InputIterator >
	set_impl(InputIterator first, InputIterator last,
			 const Comp& comp = Comp(),
			 const Alloc& alloc = Alloc()): comp(comp), inner(alloc) {
		insert(first, last);
	}

	/**
	 * \brief Constructs the container with the contents of the initializer list init.
	 */
	set_impl( std::initializer_list<value_type> init,
					const Comp& comp = Comp(),
					const Alloc& alloc = Alloc() ): comp(comp), inner(alloc) {
		insert(init);
	}
	
	/**
	 * \brief Returns an iterator to the first element of the container.
	 */
	iterator begin() noexcept {return inner.begin();}

	/**
	 * \brief Returns an iterator to the first element of the container.
	 */
	const_iterator begin() const noexcept {return inner.cbegin();}

	/**
	 * \brief Returns an iterator to the first element of the container.
	 */
	const_iterator cbegin() const noexcept {return inner.cbegin();}

	/**
	 * \brief Returns a reverse iterator to the first element of the reversed container.
	 */
	reverse_iterator rbegin() noexcept {return inner.rbegin();}

	/**
	 * \brief Returns a reverse iterator to the first element of the reversed container.
	 */
	const_reverse_iterator rbegin() const noexcept {return inner.rbegin();}

	/**
	 * \brief Returns a reverse iterator to the first element of the reversed container.
	 */
	const_reverse_iterator crbegin() const noexcept {return inner.rbegin();}

	/**
	 * \brief Returns an iterator to the element following the last element of the container.
	 */
	iterator end() noexcept {return inner.end();}

	/**
	 * \brief Returns an iterator to the element following the last element of the container.
	 */
	const_iterator end() const noexcept {return inner.cend();}

	/** 
	 * \brief Returns an iterator to the element following the last element of the container.
	 */
	const_iterator cend() const noexcept {return inner.cend();}

	/**
	 * \brief Returns a reverse iterator to the element following the
	 * last element of the reversed container.
	 */
	reverse_iterator rend() noexcept {return inner.rend();}

	/**
	 * \brief Returns a reverse iterator to the element following the
	 * last element of the reversed container.
	 */
	const_reverse_iterator rend() const noexcept {return inner.rend();}

	/**
	 * \brief Returns a reverse iterator to the element following the
	 * last element of the reversed container.
	 */
	const_reverse_iterator crend() const noexcept {return inner.rend();}

	/**
	 * \brief Checks if the container has no elements, i.e. whether
	 * begin() == end().
	 */
	bool empty() const noexcept {return inner.empty();}

	/**
	 * \brief Returns the number of elements in the container, i.e. std::distance(begin(), end())
	 */
	size_t size() const noexcept {return inner.size();}

	/**
	 * \brief Returns the maximum number of elements the container is
	 * able to hold due to system or library implementation
	 * limitations.
	 */
	size_t max_size() const noexcept {return inner.max_size();}

	/**
	 * \brief Removes all elements from the container. 
	 */
	void clear() {inner.clear(); }

	/**
	 * \brief Removes the element at pos.
	 * \return Iterator following the last removed element.
	 */
	iterator erase(iterator pos) {
		std::rotate(pos, std::next(pos), inner.end());
		inner.pop_back();
		return pos;
	}

	/**
	 * \brief Removes the elements in the range [first; last), which
	 * must be a valid range in *this.  \return Iterator following the
	 * last removed element.
	 */
	iterator erase(iterator first, iterator last) {
		std::rotate(first, last, inner.end());
		inner.resize(size() - (last-first));
		return begin();
	}

	/**
	 * \brief Removes all elements with the key value key.
	 * \return Number of elements removed.
	 */
	size_t erase(const Key & key) {
		auto x=equal_range(key);
		erase(x.first, x.second);
		return x.second - x.first;
	}

	/**
	 * \brief Exchanges the contents of the container with those of
	 * other. Does not invoke any move, copy, or swap operations on
	 * individual elements.
	*/
	void swap(set_impl & o) {
		std::swap(comp, o.comp);
		std::swap(inner, o.inner);
	}

	/**
	 * \brief Finds an element with key equivalent to key. If no such
	 * element is found, past-the-end iterator is returned.
	 */
	iterator find(const Key & key) noexcept {
		iterator x=lower_bound(key);
		if (x == end() || comp(key, *x)) return end();
		return x;
	}

	/**
	 * \brief Finds an element with key equivalent to key. If no such
	 * element is found, past-the-end iterator is returned.
	 */
	const_iterator find(const Key & key) const noexcept {
		const_iterator x=lower_bound(key);
		if (x == end() || comp(key, *x)) return end();
		return x;
	}

	/**
	 * \brief Returns a range containing all elements with the given
	 * key in the container. The range is defined by two iterators,
	 * one pointing to the first element that is not less than key and
	 * another pointing to the first element greater than key.
	 */
	std::pair<iterator, iterator> equal_range(const Key & key) noexcept {
		return std::equal_range(inner.begin(), inner.end(), key, comp);
	}

	/**
	 * \brief Returns a range containing all elements with the given
	 * key in the container. The range is defined by two iterators,
	 * one pointing to the first element that is not less than key and
	 * another pointing to the first element greater than key.
	 */
	std::pair<const_iterator, const_iterator> equal_range(const Key & key) const noexcept {
		return std::equal_range(inner.begin(), inner.end(), key, comp);
	}

	/**
	 * \brief Returns an iterator pointing to the first element that
	 * is not less than key.
	 */
	iterator lower_bound(const Key & key) noexcept {
		return std::lower_bound(inner.begin(), inner.end(), key, comp);
	}

	/**
	 * \brief Returns an iterator pointing to the first element that
	 * is not less than key.
	 */
	const_iterator lower_bound(const Key & key) const noexcept {
		return std::lower_bound(inner.begin(), inner.end(), key, comp);
	}

	/**
	 * \brief Returns an iterator pointing to the first element that
	 * is greater than key.
	 */
	iterator upper_bound(const Key & key) noexcept {
		return std::upper_bound(inner.begin(), inner.end(), key, comp);
	}

	/**
	 * \brief Returns an iterator pointing to the first element that
	 * is greater than key.
	 */
	const_iterator upper_bound(const Key & key) const noexcept {
		return std::upper_bound(inner.begin(), inner.end(), key, comp);
	}

	/**
	 * \brief Returns the function object that compares the keys,
	 * which is a copy of this container's constructor argument comp.
	 */
	key_compare key_comp() const noexcept {
		return comp.comp;
	}

	/**
	 * \brief Returns a function object that compares objects of type
	 * std::map::value_type (key-value pairs) by using key_comp to
	 * compare the first components of the pairs.
	 */
	value_compare value_comp() const noexcept {
		return comp;
	}

	/**
	 * \brief Returns the allocator associated with the container.
	 */
	allocator_type get_allocator() const {
		return inner.get_allocator();
	}

	/**
	 * \brief Copy assignment operator. Replaces the contents with a copy of the contents of other.
	 */
	set_impl & operator=(const set_impl& other ) {
		inner = other.inner;
		comp = other.comp;
		return *this;
	}

	/**
	 * \brief Move assignment operator. Replaces the contents with
	 * those of other using move semantics (i.e. the data in other is
	 * moved from other into this container)
	 */
	set_impl & operator=(set_impl&& other ) {
		inner = std::move(other.inner);
		comp = std::move(other.comp);
		return *this;
	}

	/**
	 * \brief Replaces the contents with those identified by initializer list ilist.
	 */
	set_impl& operator=(std::initializer_list<value_type> ilist) {
		clear();
		insert(ilist);
		return *this;
	}
	
	/**
	 * \brief true if the contents of the containers are equal, false otherwise.
	 */
	friend bool operator==(const set_impl & lhs, const set_impl & rhs) {
		return lhs.inner == rhs.inner;
	}

	/**
	 * \brief true if the contents of the containers are not equal, false otherwise.
	 */
	friend bool operator!=(const set_impl & lhs, const set_impl & rhs) {
		return lhs.inner != rhs.inner;
	}

	/**
	 * \brief true if the contents of the lhs are lexicographically
	 * less than the contents of rhs, false otherwise.
	 */
	friend bool operator<(const set_impl & lhs, const set_impl & rhs) {
		return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), lhs.comp);
	}

	/**
	 * \brief true if the contents of the lhs are lexicographically
	 * less than or equal the contents of rhs, false otherwise.
	 */
	friend bool operator<=(const set_impl & lhs, const set_impl & rhs) {
		return !std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
		
	}

	/**
	 * \brief true if the contents of the lhs are lexicographically
	 * greater than the contents of rhs, false otherwise.
	 */
	friend bool operator>(const set_impl & lhs, const set_impl & rhs) {
		return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
	}

	/**
	 * \brief true if the contents of the lhs are lexicographically
	 * greater than or equal the contents of rhs, false otherwise.
	 */
	friend bool operator>=(const set_impl & lhs, const set_impl & rhs) {
		return !std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), lhs.comp);
	}

	/**
	 * \brief Specializes the std::swap algorithm using adl. Swaps the
	 * contents of lhs and rhs. Calls lhs.swap(rhs).
	 */
	friend void swap(set_impl & lhs, set_impl & rhs) {lhs.swap(rhs);}

	
	/**
	 * \brief Returns the number of elements with key k.
	 */
	size_t count(const Key k) const noexcept {
		if (IH::multi) {
			auto x=equal_range(k);
			return x.second - x.first;
		} else {
			auto x=lower_bound(k);
			if (x == end() || comp(k, *x)) return 0;
			return 1;
		}
	}

	/**
	 * \brief Inserts element(s) into the container, if the container
	 * doesn't already contain an element with an equivalent key.
	 *
	 * \return Returns a pair consisting of an iterator to the
	 * inserted element (or to the element that prevented the
	 * insertion) and a bool denoting whether the insertion took
	 * place.
	 */
	insert_result insert(const T & t) {
		inner.push_back(t);
		return IH::handle(inner, comp);
	}

	/**
	 * \brief Inserts element(s) into the container, if the container
	 * doesn't already contain an element with an equivalent key.
	 *
	 * \return Returns a pair consisting of an iterator to the
	 * inserted element (or to the element that prevented the
	 * insertion) and a bool denoting whether the insertion took
	 * place.
	 */
	template <typename TT>
	insert_result insert(TT && t) {
		return emplace(std::forward<TT>(t));
	}

	/**
	 * \brief Does the same as normal insert, we ignore the hint.
	 */
	insert_result insert(const_iterator /*hint*/, const T & t) {
		return insert(t);
	}

	/**
	 * \brief Does the same as normal insert, we ignore the hint.
	 */
	template <typename TT>
	insert_result insert(const_iterator /*hint*/, TT && t) {
		return insert(std::forward<TT>(t));
	}

	/**
	 * \brief Inserts elements from range [first, last).
	 */
	template <class InputIt>
	void insert(InputIt first, InputIt last) {
		inner.reserve(size() + last-first);
		for (InputIt i=first; i != last; ++i)
			insert(*i);
	}

	/**
	 * \brief Inserts elements from initializer list ilist.
	 */
	void insert(std::initializer_list<value_type> list) {
		insert(list.begin(), list.end());
	}
				
	/**
	 * \brief Inserts a new element into the container by constructing
	 * it in-place with the given args if there is no element with the
	 * key in the container.  
	 *
	 * \return Returns a pair consisting of an iterator to the
	 * inserted element, or the already-existing element if no
	 * insertion happened, and a bool denoting whether the insertion
	 * took place.
	 */
	template <class... Args>
	insert_result emplace(Args &&... args) {
		inner.emplace_back(std::forward<Args>(args)...);
		return IH::handle(inner, comp);
	}

	/**
	 * \brief Do the same as emplace, and ignore the hint.
	 */
	template <class... Args>
	insert_result emplace_hint(const_iterator /*hint*/, Args &&... args) {
		return emplace(std::forward<Args>(args)...);
	}

	/**
	 * \brief Increase the capacity of the container to a value that's
	 * greater or equal to new_cap. If new_cap is greater than the
	 * current capacity(), new storage is allocated, otherwise the
	 * method does nothing.
	 */
	void reserve(size_t new_cap) {inner.reserve(new_cap);}

	/**
	 * \brief Requests the removal of unused capacity.
	 */
	void shrink_to_fit() {inner.shrink_to_fit();}

	/**
	 * \brief Returns the number of elements that the container has
	 * currently allocated space for.
	 */
	size_t capacity() const noexcept {return inner.capacity();}
protected:	
	value_compare comp;
	std::vector<value_type, Alloc> inner;
};

/**
 * \brief A std::set compatible set, useful when we do not have many elements (less then 512)
 */
template <typename T,
		  typename Comp=std::less<T>,
		  typename Alloc=std::allocator<T> >
using set = set_impl<T, T, bits::IdentityExtract, Comp, Alloc, bits::SingleInsertHelp>;

/**
 * \brief A std::multiset compatible multi set, useful when we do not have many elements (less then 512)
 */
template <typename T,
		  typename Comp=std::less<T>,
		  typename Alloc=std::allocator<T> >
using multiset = set_impl<T, T, bits::IdentityExtract, Comp, Alloc, bits::MultiInsertHelp>;

/**
 * \brief A std::multimap compatible multi map, useful when we do not have many elements (less then 512)
 */
template <typename Key,
		  typename T,
		  typename Comp=std::less<Key>,
		  typename Alloc=std::allocator<std::pair<Key, T> > >
using multimap = set_impl<std::pair<Key, T>, T, bits::PairExtract<Key, T>, Comp, Alloc, bits::MultiInsertHelp>;

/**
 * \brief A std::map compatible map, useful when we do not have many elements (less then 512)
 */
template <typename Key,
		  typename T,
		  typename Comp=std::less<Key>,
		  typename Alloc=std::allocator<std::pair<Key, T> >
		  > class map: public set_impl<std::pair<Key, T>, T, bits::PairExtract<Key, T>, Comp, Alloc, bits::SingleInsertHelp> {
private:
	typedef set_impl<std::pair<Key, T>, T, bits::PairExtract<Key, T>, Comp, Alloc, bits::SingleInsertHelp> P;
public:

	map() {}
	explicit map(const Comp & comp, const Alloc & alloc = Alloc()) : P(comp, alloc) {}
	explicit map(const Alloc & alloc) : P(alloc) {}
	map(const map & other) : P(other) {}
	map(const map & other, const Alloc & alloc) : P(other, alloc) {}
	map(map && other) : P(other) {}
	map(map && other, const Alloc & alloc) : P(std::move(other), alloc) {}
	template< class InputIterator >
	map(InputIterator first, InputIterator last,
		const Comp& comp = Comp(),
		const Alloc& alloc = Alloc()) : P(first, last, comp, alloc) {}
	map(std::initializer_list<typename P::value_type> init,
		const Comp& comp = Comp(),
		const Alloc& alloc = Alloc()) : P(init, comp, alloc) {}

	//using P::P; we must explicity copy the constructors until VS supports this
		
	/**
	 * \brief Returns a reference to the value that is mapped to a key
	 * equivalent to key, performing an insertion if such key does not
	 * already exist.
	 */
	T & operator[](const Key & key) {
		return this->emplace(key, T()).first->second;
	}

	/**
	 * \brief Returns a reference to the value that is mapped to a key
	 * equivalent to key, performing an insertion if such key does not
	 * already exist.
	 */
	T & operator[](Key && key) {
		return this->emplace(std::move(key), T()).first->second;
	}

	/**
	 * \brief Returns a reference to the mapped value of the element
	 * with key equivalent to key. If no such element exists, an
	 * exception of type std::out_of_range is thrown.
	 */
	T& at( const Key& key ) {
		typename P::iterator x=P::lower_bound(key);
		if (x == P::end() || P::comp(key, *x)) throw std::out_of_range("tiny::map::at");
		return x->second;
	}

	/**
	 * \brief Returns a reference to the mapped value of the element
	 * with key equivalent to key. If no such element exists, an
	 * exception of type std::out_of_range is thrown.
	 */
	const T& at( const Key& key ) const {
		typename P::const_iterator x=P::lower_bound(key);
		if (x == P::end() || P::comp(key, *x)) throw std::out_of_range("tiny::map::at");
		return x->second;
	}
};

} //namespace tiny
} //namespace tpie;

#endif //__TPIE_TINY_H__
