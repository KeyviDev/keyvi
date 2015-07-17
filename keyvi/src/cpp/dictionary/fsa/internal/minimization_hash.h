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
 * minimization_hash.h
 *
 *  Created on: Apr 28, 2014
 *      Author: hendrik
 */

#ifndef MINIMIZATION_HASH_H_
#define MINIMIZATION_HASH_H_

#include <algorithm>

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/**
 * A memory-efficient (multi-)set that can return an "equal" object (as opposed to "same").
 * This consumes less memory than standard collections.
 * When looking up a key, the key may be of a different type, but it must be possible to map it
 * to an entry of type T.
 * It is a multi-set in that it can contain more than one "equal" object, but it will only
 * return the first one.
 *
 * @tparam T the type of entry.
 */

template<class T>
class MinimizationHash
final {
   public:
    struct MemoryLimitConfiguration
    final {
       public:
        MemoryLimitConfiguration(size_t memory_limit, int generations,
                                 int maximum_number_of_items_per_table)
            : best_fit_memory_limit(memory_limit),
              best_fit_generations(generations),
              best_fit_maximum_number_of_items_per_table(
                  maximum_number_of_items_per_table) {
        }

        size_t best_fit_memory_limit;
        int best_fit_generations;
        int best_fit_maximum_number_of_items_per_table;
      };

      MinimizationHash()
          : MinimizationHash(3) {
      }

      explicit MinimizationHash(size_t step_size, size_t overflow_limit = 8) {
        hash_size_step_ = std::min(step_size, kHashMaxSizeStep);
        kOriginalHashSizeStep = hash_size_step_;
        overflow_limit_ = overflow_limit;
        max_cookie_size_ = T::GetMaxCookieSize();
        Clear();
      }

      ~MinimizationHash() {
        if (entries_) {
          delete[] entries_;
        }

        if (overflow_entries_) {
          delete[] overflow_entries_;
        }
      }


      MemoryLimitConfiguration FindMemoryLimitConfiguration (
          size_t memory_limit, int min, int max) const {
        int best_fit_memory_limit = 0;
        int best_fit_generations = 0;
        int best_fit_maximum_number_of_items_per_table = 0;

        // try to find a good number of generations (between given minimum and maximum) with equal size given the memory Limit
        for (int i = min; i <= max; ++i) {
          int maximum_number_of_items = 0;

          // find the value that fits to it in the HashmapSteptable
          for (unsigned int step = 3; step < kHashMaxSizeStep; ++step) {
            // the memory usage is the size of the hashtable itself and the size of the array for the overflow buckets which is the number of buckets divided by 4.
            size_t item_forecast_for_hashtable = kHashSizeStepTable[step]
                + std::min(kHashSizeStepTable[step] >> 2, max_cookie_size_);

            size_t memoryUsageForeCast = item_forecast_for_hashtable * sizeof(T)
                * i;

            if (memory_limit < memoryUsageForeCast) {
              maximum_number_of_items = kHashSizeStepTable[step - 1];
              break;
            }
          }

          // calculate memory usage
          int usage = (maximum_number_of_items + (maximum_number_of_items >> 2))
              * sizeof(T) * i;
          if (usage > best_fit_memory_limit) {
            best_fit_memory_limit = usage;
            best_fit_generations = i;
            best_fit_maximum_number_of_items_per_table =
                (int) (maximum_number_of_items * kLoadFactor);
          }
        }

        if (best_fit_maximum_number_of_items_per_table == 0) {
          // todo: exception?
        }

        return MemoryLimitConfiguration(
            best_fit_memory_limit, best_fit_generations,
            best_fit_maximum_number_of_items_per_table);
      }

      /**
       * Remove all entries from the hash table
       */
      void Clear() {
        hash_size_step_ = kOriginalHashSizeStep;
        hash_size_ = kHashSizeStepTable[hash_size_step_];
        rehash_limit_ = static_cast<int>(hash_size_ * kLoadFactor);

        if (entries_) {
          delete[] entries_;
        }

        entries_ = new T[hash_size_];

        if (overflow_entries_) {
          delete[] overflow_entries_;
        }

        overflow_entries_size_ = std::min(hash_size_ >> 2, max_cookie_size_);
        overflow_entries_ = new T[overflow_entries_size_];
        Reset();
      }

      /**
       * Removes all entries from the hashtable without recreating the table.
       */
      void Reset() {
        std::fill_n(entries_, hash_size_, T());
        count_ = 0;
        overflow_count_ = 1;
      }

      /**
       * Perform a hash lookup. If the hash values are equal,
       * the key's equality operator is called upon each entry with the same hash value.
       * @tparam EqualityType a type that can be used for comparison (must implement a get_hashcode and operator=)
       * @param key key for lookup
       * @return the equal state or an empty value
       */
      template<typename EqualityType>
      inline const T Get(EqualityType& key) const {
        int hash = key.GetHashcode() & 0x7fffffff;
        int bucket = hash % hash_size_;

        T entry = entries_[bucket];
        while (!entry.IsEmpty()) {
          if (key == entry) {
            return (entry);
          }

          int overflowBucket = entry.GetCookie();
          if (overflowBucket != 0) {
            entry = overflow_entries_[overflowBucket];
          } else {
            return T();
          }
        }

        return T();
      }

      /**
       * Perform a hash lookup. If the hash values are equal,
       * the key's equality operator is called upon each entry with the same hash value.
       * @param key key for lookup
       * @param other another instance of MinimizationHash to move the entry to
       * @return the equal state or an empty value
       */
      template<typename EqualityType>
      inline const T GetAndMove(EqualityType& key, MinimizationHash<T>* other) {
        int hash = key.GetHashcode() & 0x7fffffff;
        int bucket = hash % hash_size_;
        T entry = entries_[bucket];

        if (!entry.IsEmpty()) {
          int overflow_bucket;
          if (key == entry) {
            // delete the old entry
            overflow_bucket = entry.GetCookie();
            if (overflow_bucket != 0) {
              // overwrite with the entry from overflow
              entries_[bucket] = overflow_entries_[overflow_bucket];
            }

            entry.SetCookie(0);
            other->Add(entry);
            return entry;
          }

          // check for more items in overflow
          overflow_bucket = entry.GetCookie();
          if (overflow_bucket != 0) {
            entry = overflow_entries_[overflow_bucket];
            if (key == entry) {
              // disconnect this entry
              entries_[bucket].SetCookie(entry.GetCookie());

              entry.SetCookie(0);
              other->Add(entry);
              return entry;
            }

            // search further in overflowEntries
            overflow_bucket = entry.GetCookie();
            entry = overflow_entries_[overflow_bucket];

            while (!entry.IsEmpty()) {
              if (key == entry) {
                // disconnect entry
                overflow_entries_[overflow_bucket].SetCookie(entry.GetCookie());

                entry.SetCookie(0);
                other->Add(entry);

                return entry;
              }

              overflow_bucket = entry.GetCookie();
              entry = overflow_entries_[overflow_bucket];
            }
          }
        }

        return T();
      }

      /**
       * Return the number of items in the hash.
       * @return number of items.
       */
      size_t Size() const {
        return count_;
      }

      /**
       * Add this entry. This does not test whether the object is already contained in the hash.
       * @param key The key to add.
       */
      inline void Add(const T key) {
        Insert(key);

        // do not increment count in insert as it is used for rehashing
        ++count_;

        // check condition for re-hashing: count reaches limit
        if (count_ > rehash_limit_ && hash_size_step_ < kHashMaxSizeStep) {
          GrowAndRehash();
        }

        // check condition for re-hashing: overflow reaches limit
        if (overflow_count_ == overflow_entries_size_
            && overflow_entries_size_ < max_cookie_size_
            && hash_size_step_ < kHashMaxSizeStep){
          GrowAndRehash();
        }
      }

      int64_t GetMemoryUsage() const {
        return (memory_usage_);
      }

     private:

      /// magic constants definition of good hash table sizes
      const size_t kHashSizeStepTable[22] = { 997, 2029, 4079, 8171, 16363, 32749,
          65519, 131041, 262127, 524269, 1048559, 2097133, 4194287, 8388587,
          16777199, 33554393, 67108837, 134217689, 268435399, 536870879,
          1073741789, 2147483629 };

      /// Load factor of the hash table, used to calculate the rehashLimit.
      const float kLoadFactor = 0.6;

      /// maximum size of the hash table
      const size_t kHashMaxSizeStep = 21;

      /// Size of hash table at construction
      size_t kOriginalHashSizeStep = 0;

      /// the current step of the hash table
      size_t hash_size_step_ = 0;

      /// Size of the current hash table
      size_t hash_size_ = 0;

      /// limit of current hashtable
      size_t rehash_limit_ = 0;

      /// the actual data storage
      T* entries_ = 0;

      /// overflow data storage for colliding entries
      T* overflow_entries_ = 0;

      /// number of items in the data
      size_t count_ = 0;

      /// number of items in the overflow buffer
      size_t overflow_count_ = 0;

      /// number of slots uses in overflow buffer
      size_t overflow_entries_size_ = 0;

      /// limit the number of overflows to avoid long overflow chains
      size_t overflow_limit_ = 0;

      /// max size of a cookie used for overflow
      size_t max_cookie_size_ = 0;

      /// memory used by hash table
      /// TODO: to be implemented
      int64_t memory_usage_ = 0;

      inline void Insert(const T key) {
        int hash = key.GetHashcode() & 0x7fffffff;
        int bucket = hash % hash_size_;

        if (entries_[bucket].IsEmpty()) {
          entries_[bucket] = key;
        } else {
          // overflow handling

          // overflowing is limited to the address space in PrivateUse, if we run out we drop the entries
          if (overflow_count_ == max_cookie_size_) {
            return;
          }

          int overflowBucket = entries_[bucket].GetCookie();

          if (overflowBucket == 0) {
            entries_[bucket].SetCookie(overflow_count_);
            overflow_entries_[overflow_count_++] = key;
          } else {
            auto number_of_overflows = 0;
            while (overflow_entries_[overflowBucket].GetCookie() != 0 && number_of_overflows < overflow_limit_) {
              overflowBucket = overflow_entries_[overflowBucket].GetCookie();
              ++number_of_overflows;
            }

            if (number_of_overflows == overflow_limit_)  {
              TRACE("Drop entry to avoid overflow");
              return;
            }

            overflow_entries_[overflowBucket].SetCookie(overflow_count_);
            overflow_entries_[overflow_count_++] = key;
          }
        }
      }

      /**
       * Enlarge the hash table: make larger and re-hash
       */
      inline void GrowAndRehash() {
        hash_size_step_++;

        TRACE("Re-size hash table(type: %s): size: %zd, overflow size: %zd entries:%zd, collisions: %zd",
              typeid(T).name(), hash_size_, overflow_entries_size_, count_, overflow_count_);

        int oldHashSize = hash_size_;

        hash_size_ = kHashSizeStepTable[hash_size_step_];
        rehash_limit_ = static_cast<int>(hash_size_ * kLoadFactor);

        T* old_entries = entries_;
        entries_ = new T[hash_size_];

        T* old_overflow_entries = overflow_entries_;
        overflow_entries_size_ = std::min(hash_size_ >> 2, max_cookie_size_);
        overflow_entries_ = new T[overflow_entries_size_];

        int old_overflow_count = overflow_count_;
        overflow_count_ = 1;

        for (int i = 0; i < oldHashSize; ++i) {
          T e = old_entries[i];
          if (!e.IsEmpty()) {
            // clear PrivateUse
            e.SetCookie(0);
            Insert(e);
          }
        }

        // overflowEntries[0] does not exist, therefore starting with 1
        for (int i = 1; i < old_overflow_count; ++i) {
          T e = old_overflow_entries[i];

          // clear PrivateUse
          e.SetCookie(0);
          Insert(e);
        }

        delete[] old_entries;
        delete[] old_overflow_entries;
      }
    };
    } /* namespace internal */
    } /* namespace fsa */
    } /* namespace dictionary */
    } /* namespace keyvi */

#endif /* MINIMIZATION_HASH_H_ */
