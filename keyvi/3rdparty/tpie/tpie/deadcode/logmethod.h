// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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

#ifndef _TPIE_AMI_LOGMETHOD_H
#define _TPIE_AMI_LOGMETHOD_H

///////////////////////////////////////////////////////////////////////////
/// \file logmethod.h
/// Logmethod_base, Logmethod2 and LogmethodB declarations and definitions.
/// Used for now only for building trees in build_bkdtree.cpp and 
/// build_btree.cpp.
///////////////////////////////////////////////////////////////////////////


#include <tpie/config.h>
#include <tpie/portability.h>

// For vector
#include <vector>
// For pair
#include <utility>
// TPIE stuff.
#include <tpie/stream.h>
#include <tpie/coll.h>

#include <tpie/stats_tree.h>

namespace tpie {

    namespace ami {

	template<class Tp, class T0p=Tp>
	class Logmethod_params {
	public:
	    size_t cached_blocks;
	    Tp tree_params;
	    T0p tree0_params;
	    
	    Logmethod_params(): cached_blocks(16), tree_params(), tree0_params() {}
	};

    }  //  ami namespace

}  // tpie namespace


// Requirements common to T and T0:
//   key_t         [key type]
//   size_t size();
//   bool erase(const Value&);
//   bool find(const Value&);
//   size_t window_query(const Value&, const Value&, AMI_STREAM<Value>*);
//   void persist(persistence);
//   void unload(AMI_STREAM<Value>*);
//
// Requirements specific to T:
//   const Tp& params();
//   void load(AMI_STREAM<Value>*);
//   T(char*, AMI_collection_type, Tp);
//   ~T();
//
// Requirements specific to T0:
//   const T0p& params();
//   size_t os_block_count();
//   void insert(const Value&);
//   T0(char*, AMI_cllection_type, T0p);
//   ~T0();

namespace tpie {

    namespace ami {

	template<class Key, class Value, class T, class Tp, class T0 = T, class T0p = Tp>
	class Logmethod_base {
	public:
	    typedef stream<Value> stream_t;
	    typedef Logmethod_params<Tp, T0p> params_t;

	    // Delete a point.
	    bool erase(const Value& p);

	    // Point query.
	    bool find(const Value& p);

	    // Window query. Report results in stream os.
	    TPIE_OS_OFFSET window_query(const Key &lop, const Key &hip, stream_t* os);

	    void persist(persistence per);

	    // Inquire the mbr.
	    const std::pair<Value, Value> &mbr();

	    // Inquire the size.
	    TPIE_OS_OFFSET size() const { return header_.size; }

	    // Inquire the run-time parameters.
	    const Logmethod_params<Tp, T0p>& params() const { return params_; }
	    // Inquire the statistics. 
	    
	    const stats_tree &stats();

	    // Destructor. Delete all trees.
	    ~Logmethod_base();
	    
	protected:
	    // Constructor. Create a new struct. with the given base name for
	    // all its files.  Protected to avoid instantiation of this base
	    // class.
	Logmethod_base(const std::string& base_file_name, const Logmethod_params<Tp, T0p>& params);
	    
	    class header_type {
	    public:
		TPIE_OS_OFFSET size; // The total number of elements stored in the structure.
		TPIE_OS_SIZE_T last_tree; // the index of the last tree in the trees_ vector
		header_type(): size(0), last_tree(0) {}
	    };
	    
	    // Run-time parameters.
	    Logmethod_params<Tp, T0p> params_;

	    // Critical information (will be written in the header of the first tree)
	    header_type header_;

	    // The first tree.
	    T0 *tree0_;
	    
	    // The vector of trees, in increasing size.
	    std::vector<T*, tpie::allocator<T*> > trees_;
	    
	    // The base name of all trees.
	    std::string base_file_name_;
	    
	    // String used for constructing tree names.
	    std::string temp_name_;

	    // Minimum bounding rectangle.
	    std::pair<Value, Value> mbr_;
	    
	    bool mbr_is_set_;

	    // Persistence flag.
	    persistence per_;
	    
	    // Statistics.
	    stats_tree stats_;
	    
	    // Create a tree name in temp_name_ from base_file_name_ and the
	    // given index.
	    void create_tree(TPIE_OS_SIZE_T idx);
	};

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
	
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	class Logmethod2: public Logmethod_base<Key, Value, T, Tp, T0, T0p> {

	protected:
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::tree0_;
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::trees_;
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::params_;
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::stats_;
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::header_;
	    
	public:
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::create_tree;
	    
	    Logmethod2(const std::string& base_file_name, const Logmethod_params<Tp, T0p> &params);
	    bool insert(const Value& p);
	};

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
	
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	class LogmethodB: public Logmethod_base<Key, Value, T, Tp, T0, T0p> {

	protected:
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::tree0_;
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::trees_;
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::params_;
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::stats_;
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::header_;
	    
	public:
	    using Logmethod_base<Key, Value, T, Tp, T0, T0p>::create_tree;
	    
	    LogmethodB(const std::string& base_file_name, const Logmethod_params<Tp, T0p> &params);
	    bool insert(const Value& p);
	    static TPIE_OS_SIZE_T B;
	};
	
    }  //  ami namespace

}  //  tpie namespace

////////////////////////////////////////////
/////////// ***Implementation*** ///////////
////////////////////////////////////////////

namespace tpie {

    namespace ami {
	
#define LOGMETHOD_BASE Logmethod_base<Key, Value, T, Tp, T0, T0p>
#define LOGMETHOD2 Logmethod2<Key, Value, T, Tp, T0, T0p>
#define LOGMETHODB LogmethodB<Key, Value, T, Tp, T0, T0p>

	
    }  //  ami namespace

}  //  tpie namespace

///////////////////////////////////////
///////// **Logmethod_base** //////////
///////////////////////////////////////

namespace tpie {

    namespace ami {
	

//// *Logmethod_base::Logmethod_base* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	LOGMETHOD_BASE::Logmethod_base(const std::string& base_file_name, 
				       const Logmethod_params<Tp, T0p> &params):
	    params_(params), header_(), tree0_(NULL), trees_(0), per_(PERSIST_PERSISTENT), stats_() {
	    
	    // Copy the name and make sure it has two free positions, to be
	    // filled later with a unique number for each tree.
		base_file_name_ = base_file_name;
	    mbr_is_set_ = false;
		temp_name_ = base_file_name_;
	    
	    TPIE_OS_FILE_DESCRIPTOR fd; // file descriptor for the header file.
	    
	    // Try to open header file read-only.
		fd = TPIE_OS_OPEN_ORDONLY(base_file_name_);
	    if (TPIE_OS_IS_VALID_FILE_DESCRIPTOR(fd)) {
		if (TPIE_OS_READ(fd, &header_, sizeof(header_)) != sizeof(header_)) {

		    TP_LOG_WARNING_ID("Corrupt header file.");

		    assert(0);

		}
		TPIE_OS_CLOSE(fd);
		
		assert(header_.last_tree < 100);

		TPIE_OS_SIZE_T i;
		
		// Initialize trees.
		for (i = 0; i <= header_.last_tree; i++) {
		    trees_.push_back(0);
		    create_tree(i);
		}

		// Get the real params.
		params_.tree0_params = tree0_->params();
		if (i >= 1)
		    params_.tree_params = trees_[1]->params();
	    } 
	    else {

		TP_LOG_APP_DEBUG_ID("Creating new logmethod structure.");

		// Bogus entry in the trees_ vector.
		trees_.push_back(0);

		// Create tree0_.
		create_tree(0);
	    }
	}

//// *Logmethod_base::erase* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	bool LOGMETHOD_BASE::erase(const Value& p) {

	    bool ans = false;
	    
	    if (tree0_->size() > 0 && tree0_->erase(p)) {
		ans = true;
	    } 
	    else {
		for (TPIE_OS_SIZE_T i = 1; i < trees_.size(); i++) {
		    if (trees_[i]->size() > 0 && trees_[i]->erase(p)) {
			ans = true;
			break;
		    }
		}
	    }
	    
	    if (ans)
		header_.size--;
	    
	    return ans;
	}

//// *Logmethod_base::find* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	bool LOGMETHOD_BASE::find(const Value& p) {
	    
	    bool ans = false;
	    
	    // Search in all nonempty trees_.
	    if (tree0_->size() > 0 && tree0_->find(p)) {
		ans = true;
	    } 
	    else {
		for (TPIE_OS_SIZE_T i = 1; i < trees_.size(); i++) {
		    // Order is important! Short circuit evaluation.
		    if (trees_[i]->size() > 0 && trees_[i]->find(p)) {
			ans = true;
			break;
		    }
		}
	    }
	    return ans;      
	}


//// *Logmethod_base::window_query* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	TPIE_OS_OFFSET LOGMETHOD_BASE::window_query(const Key &lop, const Key &hip, 
						    AMI_STREAM<Value>* stream) {
	    TPIE_OS_OFFSET result = 0;
	    TPIE_OS_SIZE_T i;
	    
	    if (tree0_->size() > 0) {
		result += tree0_->window_query(lop, hip, stream);
	    }
	    
	    for (i = 1; i < trees_.size(); i++) {
		if (trees_[i]->size() > 0) {
		    result += trees_[i]->window_query(lop, hip, stream);
		}
	    }
	    
	    return result;
	}
	
//// *Logmethod_base::persist* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	void LOGMETHOD_BASE::persist(persistence per) {
	    per_ = per;
	}
	
//// *Logmethod_base::~Logmethod_base* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	LOGMETHOD_BASE::~Logmethod_base() {
	    TPIE_OS_FILE_DESCRIPTOR fd;
	    
	    if (per_ == PERSIST_PERSISTENT) {
		header_.last_tree = trees_.size() - 1;

		// Open the header file (create if not present).
		if (!TPIE_OS_IS_VALID_FILE_DESCRIPTOR(fd = TPIE_OS_OPEN_OEXCL(base_file_name_, TPIE_OS_FLAG_USE_MAPPING_FALSE))) {
		    //    if ((fd = open(base_file_name_, O_RDWR | O_CREAT | O_EXCL,
		    //   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1) {
		    
		    // Try again, hoping it exists.
		    if (!TPIE_OS_IS_VALID_FILE_DESCRIPTOR(fd = TPIE_OS_OPEN_ORDWR(base_file_name_, TPIE_OS_FLAG_USE_MAPPING_FALSE))) {
			//      if ((fd = open(base_file_name_, O_RDWR)) == -1) {

			TP_LOG_WARNING_ID("Error creating header file.");
			TP_LOG_WARNING_ID(strerror(errno));
			
			assert(0);
		    }
		}
		
		TPIE_OS_WRITE(fd, &header_, sizeof(header_));
	    }
	    
	    if (TPIE_OS_CLOSE(fd)) {
		TP_LOG_FATAL_ID("Failed to close() ");
		TP_LOG_FATAL_ID(base_file_name_);
	    }

	    if (per_ == PERSIST_DELETE) {
		if (TPIE_OS_UNLINK(base_file_name_)) {
		    TP_LOG_FATAL_ID("Failed to unlink() ");
		    TP_LOG_FATAL_ID(base_file_name_);
		}
	    }

	    tree0_->persist(per_);
	    tpie_delete(tree0_);
	    tree0_ = NULL;
	    
	    for (TPIE_OS_SIZE_T i = 1; i < trees_.size(); i++) {
			trees_[i]->persist(per_);
			tpie_delete(trees_[i]);
			trees_[i] = NULL;
	    }
	}

//// *Logmethod_base::mbr* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	const std::pair<Value, Value>& LOGMETHOD_BASE::mbr() {

	    TPIE_OS_SIZE_T i;

	    if (!mbr_is_set_) {
		if (tree0_->size() > 0) {
		    if (mbr_is_set_) {
			mbr_.first.set_min(tree0_->mbr().first);
			mbr_.second.set_max(tree0_->mbr().second);
		    } 
		    else {
			mbr_.first = tree0_->mbr().first;
			mbr_.second = tree0_->mbr().second;
		    }
		}   
		
		for (i = 0; i < trees_.size(); i++) {
		    if (trees_[i]->size() > 0) {
			if (mbr_is_set_) {
			    mbr_.first.set_min(trees_[i]->mbr().first);
			    mbr_.second.set_max(trees_[i]->mbr().second);
			} 
			else {
			    mbr_.first = trees_[i]->mbr().first;
			    mbr_.second = trees_[i]->mbr().second;
			}
		    }
		}
		mbr_is_set_ = true;
	    }
	    return mbr_;
	}

//// *Logmethod_base::create_tree* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	void LOGMETHOD_BASE::create_tree(TPIE_OS_SIZE_T idx) 
	{
		// untested: temp_name_ should become base_file_name_
		// plus two digits.
		std::stringstream ss;
		ss	<< temp_name_ 
			<< '0' << ((idx/10) % 10) 
			<< '0' << (char)(idx % 10);
		temp_name_ = ss.str();
	    
	    if (idx == 0) {
			if (sizeof(T0p) == 0) {
				tree0_ = tpie_new<T0>(temp_name_, WRITE_COLLECTION);
			}
			else {
				tree0_ = tpie_new<T0>(temp_name_, WRITE_COLLECTION, 
									  params_.tree0_params);
			}
	    } 
	    else {
			if (sizeof(Tp) == 0) {
				trees_[idx] = tpie_new<T>(temp_name_, WRITE_COLLECTION);
			}
			else {
				trees_[idx] = tpie_new<T>(temp_name_, WRITE_COLLECTION, 
										  params_.tree_params);
			}
	    }
	}
	
//// *Logmethod_base::stats* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	const stats_tree &LOGMETHOD_BASE::stats() {
	    for (TPIE_OS_SIZE_T i = 1; i < trees_.size(); i++) {
		if (trees_[i]->size() > 0)
		    stats_.record(trees_[i]->stats());
	    }
	    return stats_;
	}
	
    }  //  ami namespace

}  //  tpie namespace

///////////////////////////////////////
/////////// **Logmethod2** ////////////
///////////////////////////////////////

namespace tpie {

    namespace ami {
	
//// *Logmethod2::Logmethod2* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	LOGMETHOD2::Logmethod2(const std::string& base_file_name, 
			       const Logmethod_params<Tp, T0p> &params):
	    Logmethod_base<Key, Value, T, Tp, T0, T0p>(base_file_name, params) {
	}
	
//// *Logmethod2::insert* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	bool LOGMETHOD2::insert(const Value& p) {
	    
	    assert(tree0_ != NULL);
	    
	    if ((size_t)tree0_->os_block_count() < params_.cached_blocks) {
		
		// tree0_ must have insert capabilities, ie, the T0 class
		// must have insert capabilities. 
		tree0_->insert(p);

	    } 
	    else {
//     std::cout << "trees_[0]->os_block_count(): " << trees_[0]->os_block_count() << endl;
//     std::cout << "  leaf_count: " << trees_[0]->leaf_count() 
// 	 << "  node_count: " << trees_[0]->node_count() << endl;
//     std::cout << "trees_[0]->size():        " << trees_[0]->size() << endl;
//     std::cout << "  node_cache_size: " << trees_[0]->params().node_cache_size
// 	 << "  leaf_cache_size: " << trees_[0]->params().leaf_cache_size << endl;
		
		// First unload all relevant trees to a stream.
		
		typename LOGMETHOD_BASE::stream_t *stream = tpie_new<typename LOGMETHOD_BASE::stream_t>();
		stream->persist(PERSIST_DELETE);
		
		tree0_->unload(stream);
		tree0_->persist(PERSIST_DELETE);
		delete tree0_;
		///    create_tree(0);
		    
		    // Free index. The index of the first empty tree.
		    TPIE_OS_SIZE_T fi = 1;
		    while (fi < trees_.size() && trees_[fi]->size() > 0) {
			trees_[fi]->unload(stream);
			trees_[fi]->persist(PERSIST_DELETE);
			stats_.record(trees_[fi]->stats());
			tpie_delete(trees_[fi]);
			///      create_tree(fi);
			    fi++;
		    }
		    
		    // Add a new tree position if necessary (ie, no empty tree found).
		    if (fi == trees_.size()) {
				trees_.push_back(0);
			create_tree(fi);
		    }
		    
		    assert(trees_[fi]->size() == 0);
		    
		    // Write the new guy.
		    stream->write_item(p);
		    
		    // Create a new tree from stream.
		    trees_[fi]->load(stream);
		    tpie_delete(stream);
		    
		    // Create empty trees in positions 0 to fi-1.
		    for (TPIE_OS_SIZE_T ii = 0; ii < fi; ii++)
			create_tree(ii);
	    }
	    
	    header_.size++;
	    return true;
	}
	
    }  //  ami namespace

}  //  tpie namespace


////////////////////////////////////////
/////////// **LogmethodB** /////////////
////////////////////////////////////////

namespace tpie {

    namespace ami {

//// *LogmethodB::LogmethodB* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	LOGMETHODB::LogmethodB(const std::string& base_file_name,
			       const Logmethod_params<Tp, T0p> &params): 
	    Logmethod_base<Key, Value, T, Tp, T0, T0p>(base_file_name, params) {
	}
	
//// *LogmethodB::insert* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	bool LOGMETHODB::insert(const Value& p) {
	    
	    assert(tree0_ != NULL);
	    
	    // Check whether the cached block is full.
	    if (tree0_->os_block_count() < params_.cached_blocks) {
		
		tree0_->insert(p);
		
	    } 
	    else {
		
		TPIE_OS_SIZE_T fi;
		typename LOGMETHOD_BASE::stream_t *stream = new typename LOGMETHOD_BASE::stream_t;
		stream->persist(PERSIST_DELETE);
		
		tree0_->unload(stream);
		tree0_->persist(PERSIST_DELETE);
		delete tree0_;
		
		// Write the new guy.
		stream->write_item(p);
		
		// Now unload all relevant trees to stream.
		fi = 0;
		TPIE_OS_OFFSET b_to_fi = stream->stream_len();
		
		while (stream->stream_len() >= b_to_fi) {
		    fi++;
		    b_to_fi *= B;
		    
		    if (fi == trees_.size())
			break;
		    
		    if (trees_[fi]->size() > 0) {
			trees_[fi]->unload(stream);
			trees_[fi]->persist(PERSIST_DELETE);
			stats_.record(trees_[fi]->stats());
			delete trees_[fi];
		    }
		}
		
		// Add a new tree position if necessary.
		if (fi == trees_.size()) {
			trees_.push_back(0);
		}
		
		create_tree(fi);
		
		assert(trees_[fi] != NULL);
		
		trees_[fi]->load(stream);
		
		for (TPIE_OS_SIZE_T ii = 0; ii < fi; ii++)
		    create_tree(ii);
		
		delete stream;
	    }
	    
	    header_.size++;
	    return true;
	}
	
//// *LogmethodB::B* ////
	template<class Key, class Value, class T, class Tp, class T0, class T0p>
	TPIE_OS_SIZE_T LOGMETHODB::B = 100;

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_LOGMETHOD_H
