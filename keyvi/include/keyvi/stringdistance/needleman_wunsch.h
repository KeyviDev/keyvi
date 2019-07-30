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
 * needleman_wunsch.h
 *
 *  Created on: Jun 24, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_STRINGDISTANCE_NEEDLEMAN_WUNSCH_H_
#define KEYVI_STRINGDISTANCE_NEEDLEMAN_WUNSCH_H_

#include <algorithm>
#include <climits>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "keyvi/stringdistance/distance_matrix.h"
#include "utf8.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace stringdistance {

template <class CostFunctionT>
class NeedlemanWunsch final {
 public:
  NeedlemanWunsch(const std::vector<uint32_t>& input_sequence, size_t rows, int32_t max_distance)
      : max_distance_(max_distance),
        input_sequence_(input_sequence),
        distance_matrix_(rows, input_sequence.size() + 1),
        cost_function_() {
    init(rows);
  }

  NeedlemanWunsch() = delete;
  NeedlemanWunsch& operator=(NeedlemanWunsch const&) = delete;
  NeedlemanWunsch(const NeedlemanWunsch& that) = delete;

  NeedlemanWunsch(NeedlemanWunsch&& other)
      : max_distance_(other.max_distance_),
        compare_sequence_(other.compare_sequence_),
        compare_sequence_size_(other.compare_sequence_size_),
        intermediate_scores_(other.intermediate_scores_),
        last_put_position_(other.last_put_position_),
        latest_calculated_row_(other.latest_calculated_row_),
        input_sequence_(std::move(other.input_sequence_)),
        distance_matrix_(std::move(other.distance_matrix_)),
        cost_function_(std::move(other.cost_function_)) {
    other.max_distance_ = 0;
    other.compare_sequence_ = 0;
    other.compare_sequence_size_ = 0;
    other.intermediate_scores_ = 0;
    other.last_put_position_ = 0;
    other.latest_calculated_row_ = 0;
  }

  ~NeedlemanWunsch() {
    if (compare_sequence_) {
      delete[] compare_sequence_;
    }

    if (intermediate_scores_) {
      delete[] intermediate_scores_;
    }
  }

  int32_t Put(uint32_t codepoint, size_t position) {
    size_t row = position + 1;

    // ensure that we have enough rows in the matrix
    EnsureCapacity(row + 1);
    compare_sequence_[position] = codepoint;

    last_put_position_ = position;
    size_t columns = distance_matrix_.Columns();

    // Implementation of so called Ukkonen's cutoff.
    // The basic idea is to only calculate the cells of the matrix which are
    // really necessary given the maximum distance. You only need to
    // calculate the corridor, see the following picture for illustration
    // (+ calculated, - not calculated ):
    //
    // +++++---
    // -+++++--
    // --+++++-
    // ---+++++

    // 3 because minimum from MaxDistance + 1, IntermediateScore + 1, row + 1
    size_t max_distance_as_size_t = static_cast<size_t>(max_distance_);

    size_t right_cutoff = std::min(columns, row + max_distance_as_size_t - intermediate_scores_[row - 1] + 3);
    size_t left_cutoff = row > max_distance_as_size_t ? row - max_distance_as_size_t : 1;
    int32_t intermediate_score = std::numeric_limits<int32_t>::max();
    if (left_cutoff > columns) {
      intermediate_scores_[row] = std::numeric_limits<int32_t>::max();
      return std::numeric_limits<int32_t>::max();
    }

    // initialize the first cell according to Ukkonen's cutoff
    distance_matrix_.Set(row, left_cutoff - 1, (row - left_cutoff + 1) * cost_function_.GetInsertionCost());

    for (size_t i = left_cutoff; i < right_cutoff; ++i) {
      int32_t field_result;

      // 1. check for exact match according to the substitution cost
      // function
      int32_t substitution_cost = cost_function_.GetSubstitutionCost(input_sequence_[i - 1], codepoint);

      int32_t substitution_result = substitution_cost + distance_matrix_.Get(row - 1, i - 1);

      if (substitution_cost == 0) {
        // codePoints match
        field_result = substitution_result;
      } else {
        // 2. calculate costs for deletion, insertion and transposition
        int32_t deletion_result = distance_matrix_.Get(row, i - 1) + cost_function_.GetDeletionCost();
        int32_t insertion_result = distance_matrix_.Get(row - 1, i) + cost_function_.GetInsertionCost();

        int32_t transposition_result = std::numeric_limits<int32_t>::max();

        if (row > 1 && i > 1 && input_sequence_[i - 1] == compare_sequence_[position - 1] &&
            input_sequence_[i - 2] == compare_sequence_[position]) {
          transposition_result = distance_matrix_.Get(row - 2, i - 2) + cost_function_.GetTranspositionCost();
        }

        // 4. take the minimum cost
        // field_result = std::min( { deletion_result, insertion_result,
        //    transposition_result, substitution_result });

        field_result = std::min(deletion_result, insertion_result);

        field_result = std::min(field_result, transposition_result);

        field_result = std::min(field_result, substitution_result);
      }

      // put cost into matrix
      distance_matrix_.Set(row, i, field_result);
      if (field_result < intermediate_score) {
        intermediate_score = field_result;
      }
    }

    // set cutoff cell to maxDistance + 1 so that the following row does not
    // get wrong values due to uninitialized values
    if (right_cutoff < columns) {
      distance_matrix_.Set(row, right_cutoff, max_distance_ + 1);
      distance_matrix_.Set(row, columns - 1, max_distance_ + 1);
    }

    latest_calculated_row_ = row;

    if (intermediate_score > max_distance_) {
      intermediate_score = max_distance_ + 1;
      distance_matrix_.Set(row, std::min(row, columns - 1), intermediate_score);
      distance_matrix_.Set(row, columns - 1, max_distance_ + 1);
    }

    intermediate_scores_[row] = intermediate_score;

    return intermediate_score;
  }

  int32_t GetScore() const { return distance_matrix_.Get(latest_calculated_row_, distance_matrix_.Columns() - 1); }

  std::string GetCandidate() {
    std::vector<unsigned char> utf8result;
    try {
      utf8::utf32to8(compare_sequence_, compare_sequence_ + last_put_position_ + 1, back_inserter(utf8result));
    } catch (utf8::invalid_code_point& e) {
      // std::cout << "invalid codepoint " << e.code_point() << '\n';
      return "utf8 support not implemented yet";
    }
    return std::string(utf8result.begin(), utf8result.end());
  }

 private:
  int32_t max_distance_ = 0;
  uint32_t* compare_sequence_ = 0;
  size_t compare_sequence_size_;
  int32_t* intermediate_scores_ = 0;
  size_t last_put_position_ = 0;
  size_t latest_calculated_row_ = 0;
  std::vector<uint32_t> input_sequence_;
  DistanceMatrix distance_matrix_;
  CostFunctionT cost_function_;

  void init(size_t rows) {
    // initialize first row (sand box row)
    for (size_t i = 0; i < distance_matrix_.Columns(); ++i) {
      distance_matrix_.Set(0, i, i * cost_function_.GetDeletionCost());
    }

    latest_calculated_row_ = 1;

    // initialize compare Sequence and immediateScore
    compare_sequence_ = new uint32_t[rows];
    intermediate_scores_ = new int32_t[rows];
    intermediate_scores_[0] = 0;
    compare_sequence_size_ = rows;
  }

  void EnsureCapacity(size_t capacity) {
    // ensure that we have enough rows in the matrix
    distance_matrix_.EnsureRowCapacity(capacity + 1);
    if (capacity /*distance_matrix_.Rows()*/ > compare_sequence_size_) {
      // todo: increase by more than 1

      TRACE("expanding buffers from %d to %d", compare_sequence_size_, capacity);
      uint32_t* compare_sequence_new = new uint32_t[capacity];
      std::memcpy(compare_sequence_new, compare_sequence_, sizeof(uint32_t) * compare_sequence_size_);

      int32_t* intermediate_scores_new = new int32_t[capacity];
      std::memcpy(intermediate_scores_new, intermediate_scores_, sizeof(int32_t) * compare_sequence_size_);

      delete[] compare_sequence_;
      delete[] intermediate_scores_;

      intermediate_scores_ = intermediate_scores_new;
      compare_sequence_ = compare_sequence_new;
      compare_sequence_size_ = capacity;
    }
  }
};

} /* namespace stringdistance */
} /* namespace keyvi */

#endif  // KEYVI_STRINGDISTANCE_NEEDLEMAN_WUNSCH_H_
