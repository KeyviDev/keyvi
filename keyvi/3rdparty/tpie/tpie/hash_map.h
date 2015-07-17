// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, 2011, 2012 The TPIE development team
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
#ifndef __TPIE_HASHMAP_H__
#define __TPIE_HASHMAP_H__

///////////////////////////////////////////////////////////////////////////////
/// \file hash_map.h Internal hash map with guaranteed memory requirements.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/array.h>
#include <tpie/unused.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <tpie/prime.h>
#include <tpie/hash.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief Hash table handling hash collisions by chaining.
/// \tparam value_t Value to store.
/// \tparam hash_t Hash function to use.
/// \tparam equal_t Equality predicate.
/// \tparam index_t Index type into bucket array. Always size_t.
///////////////////////////////////////////////////////////////////////////////
template <typename value_t, typename hash_t, typename equal_t, typename index_t>
class chaining_hash_table {
private:
 	static const float sc;
	
#pragma pack(push, 1)
	///////////////////////////////////////////////////////////////////////////
	/// \brief Type of hash table buckets.
	///////////////////////////////////////////////////////////////////////////
 	struct bucket_t {
 		value_t value;
 		index_t next;
 	};
#pragma pack(pop)
	
	size_t first_free;
	array<index_t> list;
	array<bucket_t> buckets;

  	hash_t h;
 	equal_t e;
public:
	/** \brief Number of buckets in hash table. */
 	size_t size;

	/** \brief Special constant indicating an unused table entry. */
  	value_t unused;

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return array<index_t>::memory_coefficient() *sc + array<bucket_t>::memory_coefficient();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {
		return array<index_t>::memory_coefficient() * 100.0 
			+ array<index_t>::memory_overhead() + sizeof(chaining_hash_table) 
			+ array<bucket_t>::memory_overhead()
			- sizeof(array<index_t>)
			- sizeof(array<bucket_t>);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get contents of a bucket by its index.
	/// \param idx The index of the bucket to fetch.
	///////////////////////////////////////////////////////////////////////////
	inline value_t & get(size_t idx) {return buckets[idx].value;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get contents of a bucket by its index.
	/// \param idx The index of the bucket to fetch.
	///////////////////////////////////////////////////////////////////////////
	inline const value_t & get(size_t idx) const {return buckets[idx].value;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Clear contents of hash table.
	///////////////////////////////////////////////////////////////////////////
	void clear() {
 		first_free = 0;
 		for (size_t i=0; i < buckets.size(); ++i) {
			buckets[i].value = unused;
			buckets[i].next = static_cast<index_t>(i+1);
		}
		for (typename array<index_t>::iterator i=list.begin(); i != list.end(); ++i)
			*i = std::numeric_limits<index_t>::max();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Resize table to given number of buckets and clear contents.
	/// \param z New size of hash table.
	///////////////////////////////////////////////////////////////////////////
 	void resize(size_t z) {
 		buckets.resize(z);
		size_t x=static_cast<size_t>(99+static_cast<double>(z)*sc)|1;
		while (!is_prime(x)) x -= 2;
 		list.resize(x);
		clear();
 	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a hash table.
	/// \param ee Number of buckets in initial hash table.
	/// \param u Special value to be used to indicate unused entries in table.
	/// \param hash Hashing function.
	/// \param equal Equality predicate.
	///////////////////////////////////////////////////////////////////////////
	chaining_hash_table(size_t ee,
						value_t u,
						const hash_t & hash,
						const equal_t & equal):  h(hash), e(equal), size(0), unused(u) {resize(ee);};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return first bucket entry in use.
	///////////////////////////////////////////////////////////////////////////
	inline size_t begin() {
		if (size == 0) return buckets.size();
		for(size_t i=0; true; ++i)
			if (buckets[i].value != unused) return i;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return index greater than any buckets in use.
	///////////////////////////////////////////////////////////////////////////
	inline size_t end() const {return buckets.size();}	

	///////////////////////////////////////////////////////////////////////////
	/// \brief Find bucket entry containing given value, or end() if not found.
	/// \param value Sought value.
	///////////////////////////////////////////////////////////////////////////
 	inline size_t find(const value_t & value) const {
 		size_t v = list[h(value) % list.size()];
		while (v != std::numeric_limits<index_t>::max()) {
			if (e(buckets[v].value, value)) return v;
			v = buckets[v].next;
		}
 		return buckets.size();
 	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Insert value into hash table.
	/// \param val Value to insert.
	/// \returns (index, new)-pair, where index contains the index of the
	/// entry, and new is true if the entry wasn't already in the table.
	///////////////////////////////////////////////////////////////////////////
	inline std::pair<size_t, bool> insert(const value_t & val) {
		// First, look for the value in table.
		size_t hv = h(val) % list.size();
 		size_t v = list[hv];
		while (v != std::numeric_limits<index_t>::max()) {
			if (e(buckets[v].value, val)) return std::make_pair(v, false);
			v = buckets[v].next;
		}
		// It wasn't found. Insert into free bucket.
		v = first_free;
		first_free = buckets[v].next;
		buckets[v].value = val;
		buckets[v].next = list[hv];
		list[hv] = v;
 		++size;
		return std::make_pair(v, true);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Erase value from table.
	/// \param val Value to erase.
	///////////////////////////////////////////////////////////////////////////
 	inline void erase(const value_t & val) {
		size_t hv = h(val) % list.size();
		size_t cur = list[hv];
		size_t prev = std::numeric_limits<size_t>::max();

		while (!e(buckets[cur].value, val)) {
			prev = cur;
			cur = buckets[cur].next;
		}
		
		if (prev == std::numeric_limits<size_t>::max())
			list[hv] = buckets[cur].next;
		else
			buckets[prev].next = buckets[cur].next;

		buckets[cur].next = first_free;
		buckets[cur].value = unused;
		first_free = cur;
 		--size;
 	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Hash table handling hash collisions by linear probing.
/// \tparam value_t Value to store.
/// \tparam hash_t Hash function to use.
/// \tparam equal_t Equality predicate.
/// \tparam index_t Index type into bucket array. Always size_t.
///////////////////////////////////////////////////////////////////////////////
template <typename value_t, typename hash_t, typename equal_t, typename index_t>
class linear_probing_hash_table {
private:
	static const float sc;
 	array<value_t> elements;
  	hash_t h;
 	equal_t e;
public:
	/** \brief Number of buckets in hash table. */
 	size_t size;

	/** \brief Special constant indicating an unused table entry. */
  	value_t unused;

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return array<value_t>::memory_coefficient() *sc;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {
		return array<value_t>::memory_coefficient() * 100.0 
			+ array<value_t>::memory_overhead() + sizeof(linear_probing_hash_table) - sizeof(array<value_t>);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::clear()
	/// \copydetails chaining_hash_table::clear()
	///////////////////////////////////////////////////////////////////////////
	void clear() {
		for (typename array<value_t>::iterator i=elements.begin(); i != elements.end(); ++i)
			*i = unused;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::resize(size_t)
	/// \copydetails chaining_hash_table::resize(size_t)
	///////////////////////////////////////////////////////////////////////////
	void resize(size_t element_count) {
		size_t x=(99+static_cast<size_t>(static_cast<float>(element_count)*sc))|1;
		while (!is_prime(x)) x -= 2;
		elements.resize(x, unused);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::chaining_hash_table
	/// \copydetails chaining_hash_table::chaining_hash_table
	///////////////////////////////////////////////////////////////////////////
	linear_probing_hash_table(size_t ee, value_t u,
							  const hash_t & hash, const equal_t & equal):
		h(hash), e(equal), size(0), unused(u) {resize(ee);}
	
	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::find
	/// \copydetails chaining_hash_table::find
	///////////////////////////////////////////////////////////////////////////
 	inline size_t find(const value_t & value) const {
 		size_t v = h(value) % elements.size();
 		while (elements[v] != unused) {
 			if (e(elements[v], value)) return v;
 			v = (v + 1) % elements.size();
 		}
 		return elements.size();
 	}


	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::end()
	/// \copydetails chaining_hash_table::end()
	///////////////////////////////////////////////////////////////////////////
	inline size_t end() const {return elements.size();}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::begin()
	/// \copydetails chaining_hash_table::begin()
	///////////////////////////////////////////////////////////////////////////
	inline size_t begin() const {
		if (size == 0) return elements.size();
		for(size_t i=0; true; ++i)
			if (elements[i] != unused) return i;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::get(size_t)
	/// \copydetails chaining_hash_table::get(size_t)
	///////////////////////////////////////////////////////////////////////////
	value_t & get(size_t idx) {return elements[idx];}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::get(size_t)
	/// \copydetails chaining_hash_table::get(size_t)
	///////////////////////////////////////////////////////////////////////////
	const value_t & get(size_t idx) const {return elements[idx];}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::insert
	/// \copydetails chaining_hash_table::insert
	///////////////////////////////////////////////////////////////////////////
	inline std::pair<size_t, bool> insert(const value_t & val) {
 		size_t v = h(val) % elements.size();
 		while (elements[v] != unused) {
			if (e(elements[v],val)) {return std::make_pair(v, false);}
 			v = (v + 1) % elements.size();
 		}
 		++size;
		elements[v] = val;
		return std::make_pair(v, true);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief chaining_hash_table::erase
	/// \copydetails chaining_hash_table::erase
	///////////////////////////////////////////////////////////////////////////
 	inline void erase(const value_t & val) {
		size_t slot = find(val);
 		size_t cur = (slot+1) % elements.size();
		assert(size < elements.size());
 		while (elements[cur] != unused) {	   
 			size_t x = h(elements[cur]) % elements.size();
 			if (x <= slot && (cur > slot || x > cur)) {
 				elements[slot] = elements[cur];
 				slot = cur;
 			}
 			cur = (cur+1) % elements.size();
 		}
 		elements[slot] = unused;
 		--size;
 	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Hash map implementation backed by a template parameterized hash
/// table.
/// \tparam key_t Type of keys to store.
/// \tparam data_t Type of data associated with each key.
/// \tparam hash_t (Optional) Hash function to use.
/// \tparam equal_t (Optional) Equality predicate.
/// \tparam index_t (Optional) Index type into bucket array. Always size_t.
/// \tparam table_t (Optional) Hash table implementation.
///////////////////////////////////////////////////////////////////////////////
template <typename key_t, 
		  typename data_t, 
		  typename hash_t=hash<key_t>,
		  typename equal_t=std::equal_to<key_t>, 
		  typename index_t=size_t,
		  template <typename, typename, typename, typename> class table_t = chaining_hash_table
		  >
class hash_map: public linear_memory_base< hash_map<key_t, data_t, hash_t, equal_t, index_t, table_t> > {
public:
	typedef std::pair<key_t, data_t> value_t;
private:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Equality predicate used by hash table.
	///////////////////////////////////////////////////////////////////////////
	struct key_equal_t {
		equal_t e;
		key_equal_t(const equal_t & equal): e(equal) {}
		inline bool operator()(const value_t & a, const value_t & b) const {
			return e(a.first, b.first);
		}
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Hash function used by hash table.
	///////////////////////////////////////////////////////////////////////////
	struct key_hash_t {
		hash_t h;
		key_hash_t(const hash_t & hash): h(hash) {}
		inline size_t operator()(const value_t & a) const {
			return h(a.first);
		}
	};

	typedef table_t<value_t, key_hash_t, key_equal_t, index_t> tbl_t;

	tbl_t tbl;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Iterator base.
	///////////////////////////////////////////////////////////////////////////
 	template <typename IT>
	class iter_base {
	protected:
		IT & tbl;
		size_t cur;
 		iter_base(IT & t, size_t c): tbl(t), cur(c) {};
 		friend class hash_map::iterator;
 		friend class hash_map;
 	public:
 		inline const key_t & key() const {return tbl.get(cur).first;}
 		inline const data_t & value() const {return tbl.get(cur).second;}
 		inline const value_t & operator*() const {return tbl.get(cur);}
 		inline const value_t * operator->() const {return &tbl.get(cur);}

		template <typename IIT>
 		inline bool operator==(const iter_base<IIT> & o) const {return o.cur == cur;}
		template <typename IIT>
 		inline bool operator!=(const iter_base<IIT> & o) const {return o.cur != cur;}
 		inline void operator++() {
			++cur;
 			while (cur != tbl.end()) {
				if (tbl.get(cur) != tbl.unused) break;
				++cur;
 			}
 		}
 	};
public:
	/** \brief Const iterator type. */
	typedef iter_base<const tbl_t> const_iterator;
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Non-const iterator type.
	///////////////////////////////////////////////////////////////////////////
	class iterator: public iter_base<tbl_t> {
 	private:
		typedef iter_base<tbl_t> p_t;
 		friend class hash_map;
		inline iterator(tbl_t & tbl, size_t cur): p_t(tbl, cur) {}
		using p_t::tbl;
		using p_t::cur;
 	public:
 		inline key_t & key() {return tbl.get(cur)->first;}
 		inline data_t & value() {return tbl.get(cur)->second;}
 		inline value_t & operator*() {return tbl.get(cur);}
		inline value_t * operator->() {return &tbl.get(cur);}
 		inline operator const_iterator() const {return const_iterator(tbl, cur);}
 		inline bool operator==(const const_iterator & o) const {return o.cur == cur;}
 		inline bool operator!=(const const_iterator & o) const {return o.cur != cur;}
 	};


	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return tbl_t::memory_coefficient();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {
		return tbl_t::memory_overhead() - sizeof(tbl_t) + sizeof(hash_map);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct hash map.
	/// \param size Number of buckets in initial hash map.
	/// \param hash Hash function to use.
	/// \param equal Equality predicate to use.
	/// \param u Value to use for unused bucket entries.
	///////////////////////////////////////////////////////////////////////////
	inline hash_map(size_t size=0, const hash_t & hash=hash_t(),
					const equal_t & equal=equal_t(),
					value_t u=default_unused<value_t>::v() ):
		tbl(size, u, key_hash_t(hash), key_equal_t(equal)) {}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Resize hash map to given size and remove all entries.
	/// \param size New size of hash map.
	///////////////////////////////////////////////////////////////////////////
	inline void resize(size_t size) {tbl.resize(size);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Erase entry from hash map by key.
	/// \param key Key of entry to remove.
	///////////////////////////////////////////////////////////////////////////
	inline void erase(const key_t & key) {
		tbl.erase(value_t(key, tbl.unused.second));
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Erase entry from hash map by iterator.
	/// \param iter Entry to remove.
	///////////////////////////////////////////////////////////////////////////
	inline void erase(const iterator & iter) {erase(iter.key());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Insert data into the hash map.
	/// \param key Key of data to insert.
	/// \param data Data to associate with given key.
	///////////////////////////////////////////////////////////////////////////
	inline bool insert(const key_t & key, const data_t & data) {
		return tbl.insert(value_t(key, data)).second;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Look up data by key, creating an unused entry if it does not
	/// exist.
	/// \param key Key to look up.
	///////////////////////////////////////////////////////////////////////////
	inline data_t & operator[](const key_t & key) {
		return tbl.get(tbl.insert(value_t(key, tbl.unused.second)).first).second;
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Look up data by key.
	/// \param key Key to look up.
	///////////////////////////////////////////////////////////////////////////
	inline const data_t & operator[](const key_t & key) const {
		return tbl.get(tbl.find(key))->second;
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Get iterator to element.
	/// \param key Key of element to find.
	///////////////////////////////////////////////////////////////////////////
	inline iterator find(const key_t & key) {
		return iterator(tbl, tbl.find(value_t(key, tbl.unused.second)));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get iterator to element.
	/// \param key Key of element to find.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator find(const key_t & key) const {
		return const_iterator(tbl, tbl.find(value_t(key, tbl.unused.second)));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Search for element with key.
	/// Returns true if an element with the given key exists in the hash map.
	/// \param key Key of element to search for.
	///////////////////////////////////////////////////////////////////////////
	inline bool contains(const key_t & key) const {
		return tbl.find(value_t(key, tbl.unused.second)) != tbl.end();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return iterator to beginning of map.
	///////////////////////////////////////////////////////////////////////////
	inline iterator begin() {return iterator(tbl, tbl.begin());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return const iterator to beginning of map.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator begin() const {return const_iterator(tbl, tbl.begin());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return const iterator to beginning of map.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator cbegin() const {return const_iterator(tbl, tbl.begin());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return iterator to end of map.
	///////////////////////////////////////////////////////////////////////////
	inline iterator end() {return iterator(tbl, tbl.end());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return const iterator to end of map.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator end() const {return const_iterator(tbl, tbl.end());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return const iterator to end of map.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator cend() const {return const_iterator(tbl, tbl.end());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return number of elements in map.
	///////////////////////////////////////////////////////////////////////////
	inline size_t size() const {return tbl.size;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Clear hash map.
	///////////////////////////////////////////////////////////////////////////
	inline void clear() const {tbl.clear();}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Hash set implementation backed by a template parameterized hash
/// table.
/// \tparam key_t Type of keys to store.
/// \tparam hash_t (Optional) Hash function to use.
/// \tparam equal_t (Optional) Equality predicate.
/// \tparam index_t (Optional) Index type into bucket array. Always size_t.
/// \tparam table_t (Optional) Hash table implementation.
///////////////////////////////////////////////////////////////////////////////
template <typename key_t,
		  typename hash_t=hash<key_t>,
		  typename equal_t=std::equal_to<key_t>,
		  typename index_t=size_t,
		  template <typename, typename, typename, typename> class table_t=linear_probing_hash_table>
class hash_set {
private:
	typedef table_t<key_t, hash_t, equal_t, index_t> tbl_t;
	tbl_t tbl;
	typedef key_t value_t;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Iterator base.
	///////////////////////////////////////////////////////////////////////////
 	template <typename IT>
	class iter_base {
	protected:
		IT & tbl;
		size_t cur;
 		iter_base(IT & t, size_t c): tbl(t), cur(c) {};
 		//friend class hash_set::iterator;
 		friend class hash_set;
 	public:
 		inline const value_t & operator*() const {return tbl.get(cur);}
 		inline const value_t * operator->() const {return &tbl.get(cur);}
 		inline bool operator==(iter_base & o) const {return o.cur == cur;}
 		inline bool operator!=(iter_base & o) const {return o.cur != cur;}
 		inline void operator++() {
 			while (cur != tbl.end()) {
 				++cur;
				if (tbl.get(cur) != tbl.unused) break;
 			}
 		}
 	};
public:
	/** \brief Const iterator type. */
	typedef iter_base<const tbl_t> const_iterator;
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Non-const iterator type.
	///////////////////////////////////////////////////////////////////////////
	class iterator: public iter_base<tbl_t> {
 	private:
		typedef iter_base<tbl_t> p_t;
 		friend class hash_set;
		inline iterator(tbl_t & tbl, size_t cur): p_t(tbl, cur) {}
		using p_t::tbl;
		using p_t::cur;
 	public:
 		inline value_t & operator*() {return tbl.get(cur);}
		inline value_t * operator->() {return &tbl.get(cur);}
 		inline operator const_iterator() const {return const_iterator(tbl, cur);}
 		inline bool operator==(const const_iterator & o) const {return o.cur == cur;}
 		inline bool operator!=(const const_iterator & o) const {return o.cur != cur;}
 	};


	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return tbl_t::memory_coefficient();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {
		return tbl_t::memory_overhead() - sizeof(tbl_t) + sizeof(hash_set);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct hash set.
	/// \param size Number of buckets in initial hash map.
	/// \param hash Hash function to use.
	/// \param equal Equality predicate to use.
	/// \param u Value to use for unused bucket entries.
	///////////////////////////////////////////////////////////////////////////
	inline hash_set(size_t size=0, 
					const hash_t & hash=hash_t(), const equal_t & equal=equal_t(),
					value_t u=default_unused<value_t>::v()):
		tbl(size, u, hash, equal) {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Resize hash set to given size and remove all entries.
	/// \param size New size of hash set.
	///////////////////////////////////////////////////////////////////////////
	inline void resize(size_t size) {tbl.resize(size);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Erase entry from hash set by key.
	/// \param key Key of entry to remove.
	///////////////////////////////////////////////////////////////////////////
	inline void erase(const key_t & key) {tbl.erase(key);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Erase entry from hash set by iterator.
	/// \param key Key of entry to remove.
	///////////////////////////////////////////////////////////////////////////
	inline void erase(const iterator & iter) {erase(iter.key());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Insert key into set.
	/// \param key Key to insert.
	///////////////////////////////////////////////////////////////////////////
	inline bool insert(const key_t & key) {
		return tbl.insert(key).second;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get iterator to element.
	/// \param key Key to find.
	///////////////////////////////////////////////////////////////////////////
	inline iterator find(const key_t & key) {
		return iterator(tbl, tbl.find(key));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get const iterator to element.
	/// \param key Key to find.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator find(const key_t & key) const {
		return const_iterator(tbl, tbl.find(key));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Search for key.
	/// Returns true if the given key exists in the hash set.
	/// \param key Key to search for.
	///////////////////////////////////////////////////////////////////////////
	inline bool contains(const key_t & key) const {return tbl.find(key) != tbl.end();}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return iterator to beginning of set.
	///////////////////////////////////////////////////////////////////////////
	inline iterator begin() {return iterator(tbl, tbl.begin());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return const iterator to beginning of set.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator begin() const {return const_iterator(tbl, tbl.begin());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return const iterator to beginning of set.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator cbegin() const {return const_iterator(tbl, tbl.begin());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return iterator to end of set.
	///////////////////////////////////////////////////////////////////////////
	inline iterator end() {return iterator(tbl, tbl.end());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return const iterator to end of set.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator end() const {return const_iterator(tbl, tbl.end());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return const iterator to end of set.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator cend() const {return const_iterator(tbl, tbl.end());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return number of keys in set.
	///////////////////////////////////////////////////////////////////////////
	inline size_t size() const {return tbl.size;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Clear hash set.
	///////////////////////////////////////////////////////////////////////////
	inline void clear() const {tbl.clear();}
};


template <typename value_t, typename hash_t, typename equal_t, typename index_t>
const float linear_probing_hash_table<value_t, hash_t, equal_t, index_t>::sc = 2.0f;

template <typename value_t, typename hash_t, typename equal_t, typename index_t>
const float chaining_hash_table<value_t, hash_t, equal_t, index_t>::sc = 2.f;

}
#endif //__TPIE_HASHMAP_H__
