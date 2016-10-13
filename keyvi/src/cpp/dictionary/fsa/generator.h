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
 * generator.h
 *
 *  Created on: Apr 28, 2014
 *      Author: hendrik
 */

#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <stdexcept>
#include <arpa/inet.h>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "dictionary/fsa/internal/serialization_utils.h"
#include "dictionary/fsa/internal/sparse_array_builder.h"
#include "dictionary/fsa/internal/unpacked_state_stack.h"
#include "dictionary/fsa/internal/unpacked_state.h"
#include "dictionary/fsa/internal/null_value_store.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

/**
 * helper function: returns the length of the common prefix of the given strings
 *
 * @param first first string to compare
 * @param second second string to compare
 *
 * @returns length of the longest common prefix of given strings
 */

inline size_t get_common_prefix_length(const std::string& first, const std::string& second) {

  size_t common_prefix_length = 0;

  while (first[common_prefix_length] == second[common_prefix_length] && common_prefix_length < first.size()) {
    ++common_prefix_length;
  }
  return common_prefix_length;
}

/**
 * helper template functions to be flexible regarding strings
 */

template<typename StringType>
inline const char* c_stringify(const StringType&) {
  throw std::invalid_argument("unsupported stringtype");
  return "";
}

template<>
inline const char* c_stringify<const char *>(const char* const & str) {
  return str ? str : "";  // return empty string if str is NULL
}

template<>
inline const char* c_stringify<std::string>(std::string const & str) {
  return str.c_str();
}

/**
 *  states of the generator
 *
 * FEEDING - generator is ready for consumption, expects more data or close()
 * FINALIZING - generator got all data
 * COMPILED - automaton created, client can now call write(), get_automaton_t() and/or get_buffer()
 *
 */

enum class generator_state {
  FEEDING,
  FINALIZING,
  COMPILED
};

/**
 * Exception class for generator, thrown when generator is used in the wrong order.
 */

class generator_exception final: public std::runtime_error {
  using std::runtime_error::runtime_error;
};


/**
 * Allows for generating a fsa from a sorted list of key-value pairs.
 * The algorithm uses a stack for each column, example:
 *
 * stack  |12345
 * -------|------------------
 * k      |aa
 * e      |abc
 * y      |abcde
 * s      |abe
 *
 * We process key by key (must be in sorted order) and fill the stacks for the non-common suffix
 * when comparing two consecutive keys.
 * For example in step 1 we compare "aa" with "abc" and put "a" (the second "a" of "aa") into
 * stack 2, step 2 compares "abc" with "abcde", we do nothing except remembering that "c" is a
 * state, in step 3 ( "abe", "abcde" ): "e" into stack 5, "c" into stack 3, "d" into stack
 * 4 ...
 * In each iteration we check which stacks can be "consumed", that means we collected all
 * outgoing transitions of a state in the state automaton. We pop (take out and delete)
 * these stacks and put the transition vector into the automaton.
 * In our example our first stack we consume stack 5 first, containing "e", than stack 4 with
 * "d", stack 3 with "c", "e" and so on.
 * Note: The input must be sorted according to a user-defined sort order.
 */
typedef const internal::IValueStoreWriter::vs_param_t generator_param_t;

struct ValueHandle final {

  bool operator==(const ValueHandle other) const {
    return (value_idx == other.value_idx)
        && (count == other.count)
        && (weight == other.weight)
        && (no_minimization == other.no_minimization)
        && (deleted == other.deleted);
  }

  bool operator!=(const ValueHandle other) const {
    return !(*this == other);
  }

  uint64_t value_idx;
  size_t count;
  uint32_t weight;
  bool no_minimization;
  bool deleted;
};

template<class PersistenceT, class ValueStoreT = internal::NullValueStore, class OffsetTypeT = uint32_t, class HashCodeTypeT = int32_t>
class Generator
final {
   public:
    Generator(size_t memory_limit = 1073741824,
              const generator_param_t& params = generator_param_t(), ValueStoreT* value_store = NULL)
        : memory_limit_(memory_limit), params_(params) {

      // use 50% or limit minus 200MB for the memory limit of the hashtable
      size_t memory_limit_minimization = std::max(
          memory_limit / 2, memory_limit - (200 * 1024 * 1024));

      if (params_.count(TEMPORARY_PATH_KEY) == 0) {
        params_[TEMPORARY_PATH_KEY] =
            boost::filesystem::temp_directory_path().string();
      }

      if (params_.count(MINIMIZATION_KEY) > 0 && params_[MINIMIZATION_KEY] == "off") {
        minimize_ = false;
      }

      persistence_ = new PersistenceT(memory_limit - memory_limit_minimization,
                                      params_[TEMPORARY_PATH_KEY]);

      stack_ = new internal::UnpackedStateStack<PersistenceT>(persistence_, 30);
      builder_ = new internal::SparseArrayBuilder<PersistenceT, OffsetTypeT, HashCodeTypeT>(
          memory_limit_minimization, persistence_, ValueStoreT::inner_weight, minimize_);

      if (value_store != NULL) {
        value_store_ = value_store;
      } else {
        value_store_ = new ValueStoreT(params_);
      }

    }

    ~Generator() {
      delete persistence_;
      delete value_store_;
      if (stack_) {
        delete stack_;
      }

      if (builder_) {
        delete builder_;
      }
    }

    Generator& operator=(Generator const&) = delete;
    Generator(const Generator& that) = delete;

    /**
     * Add a key-value pair to the generator.
     * @param input_key The input key.
     * @param value A value (depending on the Valuestore implementation).
     */
    void Add(const std::string& input_key, typename ValueStoreT::value_t value =
                 ValueStoreT::no_value) {

      if(state_ != generator_state::FEEDING) {
        throw generator_exception("not in feeding state");
      }

      const size_t commonPrefixLength = get_common_prefix_length(last_key_, input_key);

      // keys are equal, just return
      if (commonPrefixLength == input_key.size() && last_key_.size() == input_key.size()) {
        return;
      }

      // check which stack can be consumed (packed into the sparse array)
      ConsumeStack(commonPrefixLength);

      // put everything that is not common between the two strings (the suffix) into the stack
      FeedStack(commonPrefixLength, input_key);

      // get value and mark final state
      bool no_minimization = false;
      uint64_t value_idx = value_store_->GetValue(value, no_minimization);

      stack_->InsertFinalState(input_key.size(), value_idx, no_minimization);

      // count number of entries
      ++number_of_keys_added_;

      // if inner weights are used update them
      uint32_t weight = value_store_->GetWeightValue(value);
      if (weight > 0){
        stack_->UpdateWeights(0, input_key.size() + 1, weight);
      }

      last_key_ = input_key;
      state_ = generator_state::FEEDING;
    }

    /**
     * Add a key and previously inserted value to the generator.
     * @param input_key The input key.
     * @param ValueHandle A handle returned by a previous call to RegisterValue
     */
    void Add(const std::string& input_key, const ValueHandle& handle) {
      if(state_ != generator_state::FEEDING) {
        throw generator_exception("not in feeding state");
      }

      const size_t commonPrefixLength = get_common_prefix_length(last_key_, input_key);

      // keys are equal, just return
      if (commonPrefixLength == input_key.size() && last_key_.size() == input_key.size()) {
        return;
      }

      // check which stack can be consumed (packed into the sparse array)
      ConsumeStack(commonPrefixLength);

      // put everything that is not common between the two strings (the suffix) into the stack
      FeedStack(commonPrefixLength, input_key);

      stack_->InsertFinalState(input_key.size(), handle.value_idx, handle.no_minimization);

      // count number of entries
      ++number_of_keys_added_;

      // if inner weights are used update them
      if (handle.weight > 0){
        stack_->UpdateWeights(0, input_key.size() + 1, handle.weight);
      }

      last_key_ = input_key;
      state_ = generator_state::FEEDING;
    }

    void CloseFeeding() {
      if(state_ != generator_state::FEEDING) {
        throw generator_exception("not in feeding state");
      }

      state_ = generator_state::FINALIZING;

      // Consume all but stack[0].
      ConsumeStack(0);

      // handling of last State.
      internal::UnpackedState<PersistenceT>* unpackedState = stack_->Get(0);

      start_state_ = builder_->PersistState(*unpackedState);

      TRACE("wrote start state at %d", start_state_); TRACE("Check first transition: %d/%d %s",
          (*unpackedState)[0].label, persistence_->ReadTransitionLabel(start_state_ + (*unpackedState)[0].label),
          (*unpackedState)[0].label == persistence_->ReadTransitionLabel(start_state_ + (*unpackedState)[0].label) ? "OK" : "BROKEN");

      // free structures that are not needed anymore
      delete stack_;
      stack_ = 0;
      number_of_states_ = builder_->GetNumberOfStates();
      delete builder_;
      builder_ = 0;

      persistence_->Flush();

      state_ = generator_state::COMPILED;
    }

    /**
     * Write persisted data into the given stream.
     * @param stream The stream to write into.
     */
    void Write(std::ostream& stream) {
      if(state_ != generator_state::COMPILED) {
        throw generator_exception("not compiled yet");
      }

      stream << "KEYVIFSA";
      WriteHeader(stream);
      // write data from persistence
      persistence_->Write(stream);

      // write date from value store
      value_store_->Write(stream);
    }

    template<typename StringType>
    void WriteToFile(StringType filename) {
      std::ofstream out_stream(filename, std::ios::binary);
      Write(out_stream);
      out_stream.close();
    }

    size_t GetFsaSize() const {
      return builder_->GetSize();
    }

    /**
     * Set a custom manifest to be embedded into the index file.
     *
     * @param manifest as JSON string
     */
    inline void SetManifestFromString(const std::string& manifest){
      SetManifest(internal::SerializationUtils::ReadJsonRecord(manifest));
    }

    /**
     * Set a custom manifest to be embedded into the index file.
     *
     * @param manifest
     */
    inline void SetManifest(const boost::property_tree::ptree& manifest){
      manifest_ = manifest;
    }

   private:
    size_t memory_limit_;
    internal::IValueStoreWriter::vs_param_t params_;
    PersistenceT* persistence_;
    ValueStoreT* value_store_;
    internal::SparseArrayBuilder<PersistenceT, OffsetTypeT, HashCodeTypeT>* builder_;
    internal::UnpackedStateStack<PersistenceT>* stack_;
    std::string last_key_ = std::string();
    size_t highest_stack_ = 0;
    uint64_t number_of_keys_added_ = 0;
    generator_state state_ = generator_state::FEEDING;
    OffsetTypeT start_state_ = 0;
    uint64_t number_of_states_ = 0;
    boost::property_tree::ptree manifest_ = boost::property_tree::ptree();
    bool minimize_ = true;

    void WriteHeader(std::ostream& stream) {
      boost::property_tree::ptree pt;
      pt.put("version", "1");
      pt.put("start_state", std::to_string(start_state_));
      pt.put("number_of_keys", std::to_string(number_of_keys_added_));
      pt.put("value_store_type", std::to_string(value_store_->GetValueStoreType()));
      pt.put("number_of_states", std::to_string(number_of_states_));
      pt.add_child("manifest", manifest_);

      internal::SerializationUtils::WriteJsonRecord(stream, pt);
    }

    inline void FeedStack(const size_t start, const std::string& key) {
      for (size_t i = start; i < key.size(); ++i) {
        uint32_t ukey =
            static_cast<uint32_t>(static_cast<unsigned char>(key[i]));
        stack_->Insert(i, ukey, 0);
      }

      // remember highest stack
      if (key.size() > highest_stack_) {
        highest_stack_ = key.size();
      }
    }

    inline void ConsumeStack(const size_t end) {
      while (highest_stack_ > end) {

        // Get outgoing transitions from the stack.
        internal::UnpackedState<PersistenceT>* unpackedState = stack_->Get(
            highest_stack_);

        OffsetTypeT transitionPointer = builder_->PersistState(*unpackedState);

        // Save transition_pointer in previous stack, indicate whether it makes sense continuing minimization
        stack_->PushTransitionPointer(highest_stack_ - 1, transitionPointer,
                                      unpackedState->GetNoMinimizationCounter());

        // Delete state
        stack_->Erase(highest_stack_);

        --highest_stack_;
      }
    }
  };

  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* GENERATOR_H_ */
