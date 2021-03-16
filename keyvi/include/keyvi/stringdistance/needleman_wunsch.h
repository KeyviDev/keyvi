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
        compare_sequence_(std::move(other.compare_sequence_)),
        intermediate_scores_(std::move(other.intermediate_scores_)),
        completion_row_(other.completion_row_),
        last_put_position_(other.last_put_position_),
        latest_calculated_row_(other.latest_calculated_row_),
        input_sequence_(std::move(other.input_sequence_)),
        distance_matrix_(std::move(other.distance_matrix_)),
        cost_function_(std::move(other.cost_function_)) {
    other.max_distance_ = 0;
    other.last_put_position_ = 0;
    other.latest_calculated_row_ = 0;
    other.completion_row_ = std::numeric_limits<int32_t>::max();
  }

  ~NeedlemanWunsch() {}

  int32_t Put(uint32_t codepoint, size_t position) {
    size_t row = position + 1;
    TRACE("Calculating row: %ld", row);

    // ensure that we have enough rows in the matrix
    EnsureCapacity(row + 1);
    compare_sequence_[position] = codepoint;

    // reset completion row if we walked backwards
    if (row <= completion_row_) {
      TRACE("reset completion row");
      completion_row_ = std::numeric_limits<int32_t>::max();
    }

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

    size_t max_distance_as_size_t = static_cast<size_t>(max_distance_);

    size_t right_cutoff = std::min(columns, row + max_distance_as_size_t + 1);
    size_t left_cutoff = row > max_distance_as_size_t ? row - max_distance_as_size_t : 1;

    int32_t intermediate_score = intermediate_scores_[row - 1] + cost_function_.GetInsertionCost(codepoint);

    // if left_cutoff >= columns, the candidate string is longer than the input + max edit distance, we can make a
    // shortcut
    if (left_cutoff >= columns) {
      // last character == exact match?
      if (row > completion_row_ || compare_sequence_[columns - 2] == input_sequence_.back()) {
        intermediate_scores_[row] = intermediate_scores_[row - 1] + cost_function_.GetCompletionCost();
      } else {
        intermediate_scores_[row] = intermediate_scores_[row - 1] + cost_function_.GetInsertionCost(codepoint);
      }
      return intermediate_scores_[row];
    }

    TRACE("Ukkonen's cutoff: left: %ld, right: %ld", left_cutoff, right_cutoff);
    // initialize the first cell according to Ukkonen's cutoff
    distance_matrix_.Set(row, left_cutoff - 1, (row - left_cutoff + 1) * cost_function_.GetInsertionCost(codepoint));
    int32_t field_result;

    for (size_t column = left_cutoff; column < right_cutoff; ++column) {
      // 1. check for exact match according to the substitution cost
      // function
      int32_t substitution_cost = cost_function_.GetSubstitutionCost(input_sequence_[column - 1], codepoint);
      int32_t substitution_result = substitution_cost + distance_matrix_.Get(row - 1, column - 1);

      if (substitution_cost == 0) {
        // codePoints match
        field_result = substitution_result;
      } else {
        // 2. calculate costs for deletion, insertion and transposition
        int32_t deletion_result =
            distance_matrix_.Get(row, column - 1) + cost_function_.GetDeletionCost(input_sequence_[column - 1]);

        int32_t completion_result = std::numeric_limits<int32_t>::max();

        if (row > completion_row_) {
          completion_result = distance_matrix_.Get(row - 1, column) + cost_function_.GetCompletionCost();
        } else if (column + 1 == columns && columns > 1 &&
                   compare_sequence_[last_put_position_ - 1] == input_sequence_.back()) {
          completion_row_ = row;
          completion_result = distance_matrix_.Get(row - 1, column) + cost_function_.GetCompletionCost();
        }

        int32_t insertion_result = distance_matrix_.Get(row - 1, column) + cost_function_.GetInsertionCost(codepoint);

        int32_t transposition_result = std::numeric_limits<int32_t>::max();

        if (row > 1 && column > 1 && input_sequence_[column - 1] == compare_sequence_[position - 1] &&
            input_sequence_[column - 2] == compare_sequence_[position]) {
          transposition_result =
              distance_matrix_.Get(row - 2, column - 2) +
              cost_function_.GetTranspositionCost(input_sequence_[column - 1], input_sequence_[column - 2]);
        }

        // 4. take the minimum cost
        // field_result = std::min( { deletion_result, insertion_result,
        //    transposition_result, substitution_result });

        field_result = std::min(deletion_result, transposition_result);

        field_result = std::min(field_result, substitution_result);
        field_result = std::min(field_result, insertion_result);
        field_result = std::min(field_result, completion_result);
      }

      // put cost into matrix
      distance_matrix_.Set(row, column, field_result);

      // take the best intermediate result from the possible cells in the matrix
      if ((column + 1 == columns || column + max_distance_ >= row) && field_result <= intermediate_score) {
        intermediate_score = field_result;
      }
    }

    // set cutoff cell to maxDistance + 1 so that the following row does not
    // get wrong values due to uninitialized values
    if (right_cutoff < columns) {
      TRACE("right_cutoff < columns");
      distance_matrix_.Set(row, right_cutoff, max_distance_ + 1);
      distance_matrix_.Set(row, columns - 1, max_distance_ + 1);
    }

    latest_calculated_row_ = row;

    TRACE("intermediate score: %d", intermediate_score);

    intermediate_scores_[row] = intermediate_score;
    TRACE("%s", GetMatrixAsString().c_str());

    return intermediate_score;
  }

  int32_t GetScore() const { return distance_matrix_.Get(latest_calculated_row_, distance_matrix_.Columns() - 1); }

  std::string GetCandidate() {
    std::vector<unsigned char> utf8result;
    utf8::utf32to8(compare_sequence_.begin(), compare_sequence_.begin() + last_put_position_ + 1,
                   back_inserter(utf8result));

    return std::string(utf8result.begin(), utf8result.end());
  }

  const std::vector<uint32_t> GetInputSequence() const { return input_sequence_; }

 private:
  int32_t max_distance_ = 0;
  std::vector<uint32_t> compare_sequence_;
  std::vector<int32_t> intermediate_scores_;

  size_t completion_row_ = 0;
  size_t last_put_position_ = 0;
  size_t latest_calculated_row_ = 0;

  std::vector<uint32_t> input_sequence_;
  DistanceMatrix distance_matrix_;
  CostFunctionT cost_function_;

  void init(size_t rows) {
    // initialize first row (sand box row)
    for (size_t i = 0; i < distance_matrix_.Columns(); ++i) {
      distance_matrix_.Set(0, i, i * cost_function_.GetDeletionCost(0));
    }

    latest_calculated_row_ = 1;
    completion_row_ = std::numeric_limits<int32_t>::max();

    // initialize compare Sequence and immediateScore
    compare_sequence_.reserve(rows);
    intermediate_scores_.reserve(rows);
    intermediate_scores_.push_back(0);
  }

  void EnsureCapacity(size_t capacity) {
    // ensure that we have enough rows in the matrix
    distance_matrix_.EnsureRowCapacity(capacity + 1);

    if (compare_sequence_.size() < capacity) {
      compare_sequence_.resize(capacity);
      compare_sequence_.resize(compare_sequence_.capacity());
      intermediate_scores_.resize(capacity);
      intermediate_scores_.resize(intermediate_scores_.capacity());
    }
  }

  std::string GetMatrixAsString() {
    std::stringstream buffer;
    buffer << "\n   |   | ";
    std::vector<unsigned char> utf8_buffer;

    for (size_t i = 0; i < input_sequence_.size(); ++i) {
      utf8::utf32to8(input_sequence_.begin() + i, input_sequence_.begin() + i + 1, back_inserter(utf8_buffer));
      buffer << std::string(utf8_buffer.begin(), utf8_buffer.end());
      utf8_buffer.clear();
      buffer << " | ";
    }
    buffer << "\n";

    for (size_t j = 0; j <= latest_calculated_row_; ++j) {
      buffer << " ";
      if (j > 0) {
        try {
          utf8::utf32to8(compare_sequence_.begin() + j - 1, compare_sequence_.begin() + j, back_inserter(utf8_buffer));
          buffer << std::string(utf8_buffer.begin(), utf8_buffer.end());
        } catch (utf8::invalid_code_point& e) {
          buffer << "�";  // NOLINT
        }
        utf8_buffer.clear();
      } else {
        buffer << " ";
      }
      buffer << " | ";
      for (size_t i = 0; i < distance_matrix_.Columns(); ++i) {
        buffer << distance_matrix_.Get(j, i);
        buffer << " | ";
      }
      buffer << "\n";
    }
    return buffer.str();
  }
};

} /* namespace stringdistance */
} /* namespace keyvi */

#endif  // KEYVI_STRINGDISTANCE_NEEDLEMAN_WUNSCH_H_
