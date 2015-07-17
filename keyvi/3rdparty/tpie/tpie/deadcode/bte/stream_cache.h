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

// BTE streams for main memory caches.
#ifndef _TPIE_BTE_STREAM_CACHE_H
#define _TPIE_BTE_STREAM_CACHE_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/bte/stream_base.h>

// This code makes assertions and logs errors.
#include <tpie/tpie_assert.h>
#include <tpie/tpie_log.h>

namespace tpie {

    namespace bte {
    
#define STREAM_CACHE_DEFAULT_MAX_LEN (1024 * 256)
    
#ifndef STREAM_CACHE_LINE_SIZE
#define STREAM_CACHE_LINE_SIZE 64
#endif // STREAM_CACHE_LINE_SIZE

//
// The cache stream class.
//
    
		template <class T>
		class stream_cache : public stream_base<T, stream_cache<T> > {
		public:
			typedef stream_base<T, stream_cache<T> > base_t;
		private:
			T *data;
			T *current;
			T *data_max;
			T *data_hard_end;
			unsigned int substream_level;
			unsigned int valid;
			bool r_only;
			bool w_only;
    
			stream_cache(void);

		public:
	
			// Constructors
			stream_cache(const std::string& /*path*/, stream_type st, TPIE_OS_OFFSET max_len); 
	
			// A psuedo-constructor for substreams.
			err new_substream(stream_type st, 
							  TPIE_OS_OFFSET sub_begin,
							  TPIE_OS_OFFSET sub_end, 
							  base_t **sub_stream);
    

			// Query memory usage
			err main_memory_usage(TPIE_OS_SIZE_T *usage,
								  stream_usage usage_type);

			// Return the number of items in the stream.
			TPIE_OS_OFFSET stream_len(void);
	
			// Move to a specific position in the stream.
			err seek(TPIE_OS_OFFSET offset);
	
			// Destructor
			~stream_cache(void);
	
			err read_item(T **elt);
			err write_item(const T &elt);

			bool read_only(void) { return r_only; };
			bool write_only(void) { return w_only; };
	
			int available_streams(void) { return -1; };    

			TPIE_OS_OFFSET tell(void);
			TPIE_OS_OFFSET chunk_size(void);
		};
    
    
		template<class T>
		stream_cache<T>::stream_cache(void) {
			// No code in this constructor.
		};
    
		template<class T>
		stream_cache<T>::stream_cache(const std::string& /*path*/, 
									  stream_type st,
									  TPIE_OS_OFFSET max_len) {

			// A stream being created out of the blue must be writable, so we
			// return an error if it is not.
	
			switch (st) {

			case READ_STREAM:
				r_only=true; //intentional fallthrough
			case APPEND_STREAM:
				r_only=false;
				w_only=true;
				valid = 0;

				break;

			case WRITEONLY_STREAM:
				w_only = true; //intentional fallthrough
			case WRITE_STREAM:
				w_only = r_only = false;

				if (!max_len) {
					max_len = STREAM_CACHE_DEFAULT_MAX_LEN;
				}

				// Use malloc() directly rather than new becasue this is
				// in "secondary memory" and will not necessarily go into
				// the cache.
				data = (T*)malloc(static_cast<size_t>(max_len)*sizeof(T));
				if (data == NULL) {
					valid = 0;

					TP_LOG_FATAL_ID("Out of \"secondary memory.\"");
                
					return;
				}

				current = data_max = data;
				data_hard_end = data + max_len;
				valid = 1;
			}
		};
    
    
		template<class T>
		err stream_cache<T>::new_substream(stream_type st, 
										   TPIE_OS_OFFSET sub_begin,
										   TPIE_OS_OFFSET sub_end,
										   base_t **sub_stream) {
			stream_cache *ss;
    
			if (st == APPEND_STREAM) {
				return PERMISSION_DENIED;
			} 
			else {

				if ((sub_begin >= data_hard_end - data) ||
					(sub_end >= data_hard_end - data) ||
					(sub_begin >= data_max - data) ||
					(sub_end >= data_max - data) ||
					(sub_end < sub_begin)) {
					return OFFSET_OUT_OF_RANGE;
				}

				ss = tpie_new<stream_cache>();
				ss->r_only = (st == READ_STREAM);
				ss->substream_level = substream_level + 1;
				ss->current = ss->data = data + sub_begin;
				ss->data_max = ss->data_hard_end = data + sub_end + 1;
				ss->w_only= (st == WRITEONLY_STREAM);

				*sub_stream = (base_t *)ss;                       

				return NO_ERROR;
			}
		};           
    

		template<class T>
		err stream_cache<T>::main_memory_usage(TPIE_OS_SIZE_T *usage,
											   stream_usage usage_type) {
			switch (usage_type) {

			case STREAM_USAGE_CURRENT:
			case STREAM_USAGE_MAXIMUM:
			case STREAM_USAGE_SUBSTREAM:
				*usage = sizeof(*this) + STREAM_CACHE_LINE_SIZE;

				break;

			case STREAM_USAGE_BUFFER:
				*usage = STREAM_CACHE_LINE_SIZE;

				break;

			case STREAM_USAGE_OVERHEAD:
				*usage = sizeof(this);

				break;

			}
			return NO_ERROR;
		};
    

		template<class T>
		TPIE_OS_OFFSET stream_cache<T>::stream_len(void) {
			return data_max - data;
		};
    
    
    
		template<class T>
		err stream_cache<T>::seek(TPIE_OS_OFFSET offset) {
	
			if (offset > data_hard_end - data) {
				return OFFSET_OUT_OF_RANGE;
			} 
			else {
				current = data + offset;
			}

			return NO_ERROR;
		};


		template<class T>
		stream_cache<T>::~stream_cache(void) {
			if (!substream_level) {
				std::free(data);
			}
		};

		template<class T>
		err stream_cache<T>::read_item(T **elt) {

			if (w_only) {	    
				return PERMISSION_DENIED;
			}

			if (current >= data_max) {
				return END_OF_STREAM;
			}
			else {
				*elt = current++;
			}
	
			return NO_ERROR;
	
		};

		template<class T>
		err stream_cache<T>::write_item(const T &elt) {

			if (r_only) {	    
				return PERMISSION_DENIED;
			}

			if (current >= data_hard_end) {
				return OUT_OF_SPACE;
			} 
			else {
				*current++ = elt;
				if (current > data_max) {
					data_max = current;
				}
			}

			return NO_ERROR;
    
		};


		template<class T>
		TPIE_OS_OFFSET stream_cache<T>::chunk_size(void) {
			return STREAM_CACHE_LINE_SIZE / sizeof(T);       
		}

		template<class T>
		TPIE_OS_OFFSET stream_cache<T>::tell(void) {
			return current - data;
		}   
    }  //  bte namespace
}  //  tpie namespace

#endif // _TPIE_BTE_STREAM_CACHE_H 
