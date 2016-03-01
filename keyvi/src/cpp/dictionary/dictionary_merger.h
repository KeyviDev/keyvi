/* * keyvi - A key value store.
 *
 * Copyright 2015, 2016 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * dictionary_merger.h
 *
 *  Created on: Feb 27, 2016
 *      Author: hendrik
 */

#ifndef DICTIONARY_MERGER_H_
#define DICTIONARY_MERGER_H_

#include <queue>

#include "dictionary/fsa/generator.h"
#include "dictionary/fsa/automata.h"
#include "dictionary/fsa/entry_iterator.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {

template<class PersistenceT, class ValueStoreT = fsa::internal::NullValueStore>
class DictionaryMerger
final {
 public:
  DictionaryMerger(): dicts_to_merge_(){
  }

  void Add (const std::string& filename){
    fsa::automata_t fsa (new fsa::Automata(filename.c_str()));

    // todo: check that new added dict are compatible
    dicts_to_merge_.push_back(fsa);
  }

  void Merge(){
    std::priority_queue<fsa::EntryIterator> pqueue;
    fsa::EntryIterator end_it;

    for (auto fsa: dicts_to_merge_) {
      fsa::EntryIterator e_it(fsa);
      pqueue.push(e_it);
    }

    fsa::Generator<PersistenceT, ValueStoreT> generator;

    while(!pqueue.empty()){
      auto e = pqueue.top();
      pqueue.pop();
      std::cout<<e.GetKey()<< std::endl;
      generator.Add(e.GetKey());

      if (++e != end_it) {
        pqueue.push(e);
      }
    }
    generator.Compile();

  }

  void Write(std::ostream& stream) {
  }

  void WriteToFile(std::string& filename) {
  }

 private:
  std::vector<fsa::automata_t> dicts_to_merge_;

};

} /* namespace dictionary */
} /* namespace keyvi */


#endif /* SRC_CPP_DICTIONARY_DICTIONARY_MERGER_H_ */
