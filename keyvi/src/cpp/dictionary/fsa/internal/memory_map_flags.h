/* * keyvi - A key value store.
 *
 * Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * memory_map_flags.h
 *
 *  Created on: May 17, 2016
 *      Author: hendrik
 */

#ifndef KEYVI_MEMORY_MAP_FLAGS_H_
#define KEYVI_MEMORY_MAP_FLAGS_H_

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

// Mac has no MAP_POPULATE
#if defined(OS_MACOSX)
#ifndef MAP_POPULATE
#define MAP_POPULATE 0
#endif
#endif


namespace keyvi {
namespace dictionary {

enum class loading_strategy_types {
               default_os, // no special treatment, use whatever the OS/Boost has as default
               lazy, // load data as needed with some read-ahead
               populate, // immediately load everything in memory (blocks until everything is fully read)
               populate_key_part, // populate only the key part, load value part lazy
               populate_lazy, // load data lazy but ask the OS to read ahead if possible (does not block)
               lazy_no_readahead, // disable any read-ahead (for cases when index > x * main memory)
               lazy_no_readahead_value_part, // disable read-ahead only for the value part
               populate_key_part_no_readahead_value_part // populate the key part, but disable read ahead value part
             };

namespace fsa {
namespace internal {

class MemoryMapFlags final{
public:
   /**
    * Translates the loading strategy into the according options for mmap. To be used for loading the FSA part.
    *
    * @param strategy load strategy
    * @return flags to be used for mmap (via boost).
    */
   static int FSAGetMemoryMapOptions(const loading_strategy_types strategy) {
#ifdef _Win32

     // there is no comparable fine-grained control on windows, so simply use the defaults
     return boost::interprocess::default_map_options;
#else // not _Win32

     if (strategy ==  loading_strategy_types::default_os) {
       return boost::interprocess::default_map_options;
     }

     int flags = 0;

     flags |= MAP_SHARED;

#ifdef MAP_NOSYNC
     flags |= MAP_NOSYNC
#endif

     switch (strategy){
       case loading_strategy_types::populate:
       case loading_strategy_types::populate_key_part:
       case loading_strategy_types::populate_key_part_no_readahead_value_part:
         flags |= MAP_POPULATE;
         break;
       default:
         break;
     }

     return flags;
#endif // not _Win32
   }

   /**
    * Translates the loading strategy into the according options for mmap. To be used for loading the Values part.
    *
    * @param strategy load strategy
    * @return flags to be used for mmap (via boost).
    */
   static int ValuesGetMemoryMapOptions(const loading_strategy_types strategy) {
#ifdef _Win32

     // there is no comparable fine-grained control on windows, so simply use the defaults
     return boost::interprocess::default_map_options;
#else // not _Win32

     if (strategy == loading_strategy_types::default_os) {
       return boost::interprocess::default_map_options;
     }

     int flags = 0;
     flags |= MAP_SHARED;

#ifdef MAP_NOSYNC
     flags |= MAP_NOSYNC
#endif

     switch (strategy){
       case loading_strategy_types::populate:
         flags |= MAP_POPULATE;
         break;
       default:
         break;
     }

     return flags;
#endif // not _Win32
   }


   /**
    * Translates the loading strategy into the according options for madvise. To be used for loading the FSA part.
    *
    * @param strategy load strategy
    * @return advise to be used for madvise (via boost)
    */
   static boost::interprocess::mapped_region::advice_types FSAGetMemoryMapAdvices(const loading_strategy_types strategy) {
#ifdef _Win32

     // there is no madvise on windows, so simply use the default
     return boost::interprocess::mapped_region::advice_types::advice_normal;
#else // _Win32
     switch (strategy){
       case loading_strategy_types::lazy_no_readahead:
         return boost::interprocess::mapped_region::advice_types::advice_random;
       case loading_strategy_types::lazy_no_readahead_value_part:
       case loading_strategy_types::populate_key_part_no_readahead_value_part:
         break;
       case loading_strategy_types::populate_lazy:
         return boost::interprocess::mapped_region::advice_types::advice_willneed;
       default:
         break;
     }

     return boost::interprocess::mapped_region::advice_types::advice_normal;
#endif
   }

   /**
    * Translates the loading strategy into the according options for madvise. To be used for loading the Values part.
    *
    * @param strategy load strategy
    * @return advise to be used for madvise (via boost)
    */
   static boost::interprocess::mapped_region::advice_types ValuesGetMemoryMapAdvices(const loading_strategy_types strategy) {
#ifdef _Win32

     // there is no madvise on windows, so simply use the default
     return boost::interprocess::mapped_region::advice_types::advice_normal;
#else // _Win32
     switch (strategy){
       case loading_strategy_types::lazy_no_readahead:
       case loading_strategy_types::lazy_no_readahead_value_part:
       case loading_strategy_types::populate_key_part_no_readahead_value_part:
         return boost::interprocess::mapped_region::advice_types::advice_random;
       case loading_strategy_types::populate_lazy:
         return boost::interprocess::mapped_region::advice_types::advice_willneed;
       default:
         break;
     }

     return boost::interprocess::mapped_region::advice_types::advice_normal;
#endif
  }
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */


#endif /* KEYVI_MEMORY_MAP_FLAGS_H_ */
