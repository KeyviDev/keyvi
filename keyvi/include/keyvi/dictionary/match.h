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
 * match.h
 *
 *  Created on: Jun 4, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_MATCH_H_
#define KEYVI_DICTIONARY_MATCH_H_

#include <memory>
#include <string>
#include <utility>
#include <variant>

#include <boost/container/flat_map.hpp>

#include "keyvi/compression/compression_strategy.h"
#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/util/json_value.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
struct Match;

using match_t = std::shared_ptr<Match>;
}  // namespace dictionary

namespace index {
namespace internal {
template <class MatcherT, class DeletedT>
keyvi::dictionary::match_t NextFilteredMatch(const MatcherT&, const DeletedT&);
template <class MatcherT, class DeletedT>
keyvi::dictionary::match_t FirstFilteredMatch(const MatcherT&, const DeletedT&);
}  // namespace internal
}  // namespace index
namespace dictionary {

struct Match {
  using attribute_t = std::variant<std::string, int, double, bool>;
  using attributes_t = std::shared_ptr<boost::container::flat_map<std::string, attribute_t>>;

  Match(size_t a, size_t b, const std::string& matched_item, uint32_t score = 0, uint32_t weight = 0)
      : start_(a), end_(b), matched_item_(matched_item), raw_value_(), score_(score) {
    TRACE("initialized Match %d->%d %s", a, b, matched_item.c_str());
  }

  Match(size_t a, size_t b, const std::string& matched_item, uint32_t score, const fsa::automata_t& fsa, uint64_t state,
        uint32_t weight = 0)
      : start_(a), end_(b), matched_item_(matched_item), raw_value_(), score_(score), fsa_(fsa), state_(state) {
    TRACE("initialized Match %d->%d %s", a, b, matched_item.c_str());
  }

  Match() : matched_item_(), raw_value_() {}

  Match(Match&& other)
      : start_(other.start_),
        end_(other.end_),
        matched_item_(std::move(other.matched_item_)),
        raw_value_(std::move(other.raw_value_)),
        score_(other.score_),
        fsa_(other.fsa_),
        state_(other.state_),
        attributes_(std::move(other.attributes_)) {
    other.start_ = 0;
    other.end_ = 0;
    other.score_ = 0;
    other.state_ = 0;
  }

  Match& operator=(Match&& other) {
    start_ = other.start_;
    end_ = other.end_;
    matched_item_ = std::move(other.matched_item_);
    raw_value_ = std::move(other.raw_value_);
    score_ = other.score_;
    fsa_ = std::move(other.fsa_);
    state_ = other.state_;
    attributes_ = std::move(other.attributes_);

    other.start_ = 0;
    other.end_ = 0;
    other.score_ = 0;
    other.state_ = 0;
    return *this;
  }

  size_t GetEnd() const { return end_; }

  void SetEnd(size_t end = 0) { end_ = end; }

  const std::string& GetMatchedString() const { return matched_item_; }

  void SetMatchedString(const std::string& matched_item) { matched_item_ = matched_item; }

  double GetScore() const { return score_; }

  void SetScore(double score = 0) { score_ = score; }

  size_t GetStart() const { return start_; }

  void SetStart(size_t start = 0) { start_ = start; }

  bool IsEmpty() const { return start_ == 0 && end_ == 0; }

#ifdef Py_PYTHON_H
  PyObject* GetAttributePy(const std::string& key) {
    auto result = GetAttribute(key);

    return std::visit(
        [](auto&& arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, bool>) {
            return arg ? Py_True : Py_False;
          } else if constexpr (std::is_same_v<T, int>) {
            return PyLong_FromLong(arg);
          } else if constexpr (std::is_same_v<T, double>) {
            return PyFloat_FromDouble(arg);
          } else if constexpr (std::is_same_v<T, std::string>) {
            return PyUnicode_FromString(arg.c_str());
          }
          // not reachable
          throw std::runtime_error("unexpected attribute type");
        },
        result);
  }
#endif

  const attribute_t& GetAttribute(const std::string& key) {
    // lazy creation
    if (!attributes_) {
      if (fsa_) {
        attributes_ = fsa_->GetValueAsAttributeVector(state_);
      } else {
        attributes_ = attributes_t(new fsa::internal::IValueStoreReader::attributes_raw_t);
      }
    }

    return attributes_->at(key);
  }

  template <typename U>
  void SetAttribute(const std::string& key, U value) {
    if (!attributes_) {
      if (fsa_) {
        attributes_ = fsa_->GetValueAsAttributeVector(state_);
      } else {
        attributes_ = attributes_t(new fsa::internal::IValueStoreReader::attributes_raw_t);
      }
    }

    (*attributes_)[key] = value;
  }

  uint32_t GetWeight() const {
    if (!fsa_) {
      return 0;
    }

    return fsa_->GetWeight(state_);
  }

  std::string GetValueAsString() const {
    if (!fsa_) {
      if (raw_value_.size() != 0) {
        return keyvi::util::DecodeJsonValue(raw_value_);
      } else {
        return "";
      }
    }

    return fsa_->GetValueAsString(state_);
  }

  std::string GetRawValueAsString() const {
    if (!fsa_) {
      return raw_value_;
    }

    return fsa_->GetRawValueAsString(state_);
  }

  std::string GetMsgPackedValueAsString(const compression::CompressionAlgorithm compression_algorithm =
                                            compression::CompressionAlgorithm::NO_COMPRESSION) const {
    if (!fsa_) {
      if (raw_value_.empty()) {
        return raw_value_;
      }

      if (raw_value_[0] == compression_algorithm) {
        return raw_value_.substr(1);
      }

      // decompress
      const compression::decompress_func_t decompressor =
          compression::decompressor_by_code(static_cast<compression::CompressionAlgorithm>(raw_value_[0]));
      std::string msgpacked_value = decompressor(raw_value_);

      if (compression_algorithm == compression::CompressionAlgorithm::NO_COMPRESSION) {
        return msgpacked_value;
      }
      // compress
      const compression::compression_strategy_t compressor =
          compression::compression_strategy_by_code(compression_algorithm);

      return compressor->CompressWithoutHeader(msgpacked_value);
    }

    return fsa_->GetMsgPackedValueAsString(state_, compression_algorithm);
  }

  /**
   * being able to set the value, e.g. when keyvi is used over network boundaries
   *
   * @param value
   */
  void SetRawValue(const std::string& value) {
    raw_value_ = value;
  }

 private:
  size_t start_ = 0;
  size_t end_ = 0;
  std::string matched_item_;
  std::string raw_value_;
  double score_ = 0;
  fsa::automata_t fsa_ = 0;
  uint64_t state_ = 0;
  attributes_t attributes_ = 0;

  // friend for accessing the fsa
  template <class MatcherT, class DeletedT>
  friend match_t index::internal::NextFilteredMatch(const MatcherT&, const DeletedT&);
  template <class MatcherT, class DeletedT>
  friend match_t index::internal::FirstFilteredMatch(const MatcherT&, const DeletedT&);

  fsa::automata_t& GetFsa() {
    return fsa_;
  }
};

} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_MATCH_H_
