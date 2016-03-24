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
 private:
  struct SegmentEntryForMerge
  {
    SegmentEntryForMerge(fsa::EntryIterator& e, int p): entry_iterator(e), priority(p) {}

    fsa::EntryIterator entry_iterator;
    int priority;
    bool operator<(const SegmentEntryForMerge& rhs) const
    {
      // very important difference in semantics: we have to ensure that in case of equal key,
      // the iterator with the higher priority is taken

      if (priority < rhs.priority) {
        return entry_iterator > rhs.entry_iterator;
       }

      return rhs.entry_iterator < entry_iterator;
    }
  };

 public:
  DictionaryMerger(): dicts_to_merge_(){
  }

  void Add (const std::string& filename){
    fsa::automata_t fsa (new fsa::Automata(filename.c_str()));

    if (fsa->GetValueStore()->GetValueStoreType() != ValueStoreT::GetValueStoreType()) {
      throw std::invalid_argument("Dictionaries must have the same type.");
    }

    dicts_to_merge_.push_back(fsa);
  }

  void Merge(const std::string& filename){
    std::priority_queue<SegmentEntryForMerge> pqueue;
    fsa::EntryIterator end_it;

    int i = 0;
    for (auto fsa: dicts_to_merge_) {
      fsa::EntryIterator e_it(fsa);
      pqueue.push(SegmentEntryForMerge(e_it, i++));
    }

    ValueStoreT* value_store = new ValueStoreT();

    fsa::Generator<PersistenceT, ValueStoreT> generator(1073741824, fsa::generator_param_t(), value_store);

    std::string top_key;

    while(!pqueue.empty()){
      auto entry_it = pqueue.top();
      pqueue.pop();

      top_key = entry_it.entry_iterator.GetKey();

      // check for same keys and merge only the most recent one
      while (!pqueue.empty() and pqueue.top().entry_iterator.operator==(top_key)) {

        auto to_inc = pqueue.top();
        TRACE("removing element with prio %d (in favor of %d)", to_inc.priority, entry_it.priority);

        pqueue.pop();
        if (++to_inc.entry_iterator != end_it) {
          TRACE("push iterator");
          pqueue.push(to_inc);
        }
      }

      fsa::ValueHandle handle;
      handle.no_minimization = false;

      // Todo: if inner weights are used update them
      //handle.weight = value_store_->GetWeightValue(value);
      handle.weight = 0;

      handle.value_idx = value_store->GetValue(entry_it.entry_iterator.GetFsa()->GetValueStore()->GetValueStorePayload(),
                                               entry_it.entry_iterator.GetValueId(),
                                               handle.no_minimization);

      TRACE("Add key: %s", top_key.c_str());
      generator.Add(std::move(top_key), handle);

      if (++entry_it.entry_iterator != end_it) {
        pqueue.push(entry_it);
      }
    }
    TRACE("finished iterating, do final compile.");

    generator.CloseFeeding();

    generator.WriteToFile(filename);
  }

 private:
  std::vector<fsa::automata_t> dicts_to_merge_;

};

} /* namespace dictionary */
} /* namespace keyvi */


#endif /* SRC_CPP_DICTIONARY_DICTIONARY_MERGER_H_ */
