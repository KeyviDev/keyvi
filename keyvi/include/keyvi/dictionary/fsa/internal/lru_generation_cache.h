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
 * lru_generation_cache.h
 *
 *  Created on: May 2, 2014
 *      Author: hendrik
 */

#ifndef LRU_GENERATION_CACHE_H_
#define LRU_GENERATION_CACHE_H_

#include <vector>

#include "dictionary/fsa/internal/packed_state.h"
#include "dictionary/fsa/internal/minimization_hash.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/**
 * A simple implementation of a lightweight least recently used cache using generations.
 */
template <class EntryT = PackedState<>>
class LeastRecentlyUsedGenerationsCache
final {

   public:
  /**
   * Constructor of LeastRecentlyUsedGenerationsCache
   * @param memory_limit memory limit
   */
  LeastRecentlyUsedGenerationsCache (size_t memory_limit){

    current_generation_ = new MinimizationHash<EntryT>();

    auto memoryConfiguration =

    current_generation_->FindMemoryLimitConfiguration(
        memory_limit, 3, 6);

    size_per_generation_ = memoryConfiguration.best_fit_maximum_number_of_items_per_table;
    max_number_of_generations_ = memoryConfiguration.best_fit_generations;
  }

    /** Constructor of LeastRecentlyUsedGenerationsCache
     *
     *  @param sizePerGeneration The maximal size of one generation.
     *  @param maxNumberOfGenerations The maximum number of generations to keep.
     */
    LeastRecentlyUsedGenerationsCache(int size_per_generation,
                                      int max_number_of_generations)
        : size_per_generation_(size_per_generation),
          max_number_of_generations_(max_number_of_generations) {
      current_generation_ = new MinimizationHash<EntryT>();
    }

    ~LeastRecentlyUsedGenerationsCache() {
      delete current_generation_;
      for (MinimizationHash<EntryT>* generation : generations_) {
        delete generation;
      }
    }

    LeastRecentlyUsedGenerationsCache() = delete;
    LeastRecentlyUsedGenerationsCache& operator=(LeastRecentlyUsedGenerationsCache const&) = delete;
    LeastRecentlyUsedGenerationsCache(const LeastRecentlyUsedGenerationsCache& that) = delete;

    /** Add this object.
     * @param key The key to add
     */
    void Add(EntryT key) {
      if (current_generation_->Size() >= size_per_generation_) {
        MinimizationHash<EntryT>* newGeneration = NULL;
        if (generations_.size() + 1 == max_number_of_generations_) {
          // remove(free) the first generation
          newGeneration = generations_[0];
          newGeneration->Reset();
          generations_.erase(generations_.begin());
        }

        generations_.push_back(current_generation_);

        if (newGeneration == NULL) {
          newGeneration = new MinimizationHash<EntryT>();
        }

        current_generation_ = newGeneration;
      }

      current_generation_->Add(key);
    }

    template<typename EqualityType>
    const EntryT Get(EqualityType& key) {

      EntryT state = current_generation_->Get(key);

      if (!state.IsEmpty()) {
        return state;
      }

      // try to find it in one of the generations
      for (size_t i = generations_.size(); i > 0; --i) {
        state = generations_[i - 1]->GetAndMove(key, current_generation_);

        if (!state.IsEmpty()) {
          return state;
        }
      }

      // return am empty state
      return EntryT();
    }

    /***
     * Clear the cache
     */
    void Clear() {
      current_generation_->Clear();
      for (MinimizationHash<EntryT>* generation : generations_) {
        delete generation;
      }
      generations_.clear();
    }

    /***
     * Get the memory usage of the cache and all underlying data.
     *
     * @return the memory usage
     */
    size_t GetMemoryUsage() const {
      size_t memory = current_generation_->GetMemoryUsage();
      for (auto generation : generations_) {
        memory += generation->GetMemoryUsage();
      }
      return memory;
    }

   private:
    size_t size_per_generation_;
    size_t max_number_of_generations_;
    MinimizationHash<EntryT>* current_generation_;
    std::vector<MinimizationHash<EntryT>*> generations_;
  };

  } /* namespace internal */
  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* LRU_GENERATION_CACHE_H_ */
