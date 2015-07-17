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
 * flat_array_distance_matrix.h
 *
 *  Created on: Jun 24, 2014
 *      Author: hendrik
 */

#ifndef DISTANCE_MATRIX_H_
#define DISTANCE_MATRIX_H_

#include<cstring>

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace stringdistance {

struct DistanceMatrix
final {
   public:
    DistanceMatrix(size_t rows, size_t columns) {
      if (rows < 1 || columns < 1) {
        throw new std::invalid_argument(
            "Distance Matrix must have at least 1 row and 1 column.");
      }

      distance_matrix_ = new int[rows * columns];
      number_of_rows_ = rows;
      number_of_columns_ = columns;
    }

    DistanceMatrix() = delete;
    DistanceMatrix& operator=(DistanceMatrix const&) = delete;
    DistanceMatrix(const DistanceMatrix& that) = delete;

    DistanceMatrix(DistanceMatrix&& other)
        : distance_matrix_(other.distance_matrix_),
          number_of_columns_(other.number_of_columns_),
          number_of_rows_(other.number_of_rows_) {
      other.distance_matrix_ = 0;
      other.number_of_columns_ = 0;
      other.number_of_rows_ = 0;
    }

    ~DistanceMatrix() {
      if (distance_matrix_) {
        delete[] distance_matrix_;
      }
    }

    int Get(int row, int column) const {
      return distance_matrix_[((row * number_of_columns_) + column)];
    }

    void Set(int row, int column, int value) {
      distance_matrix_[((row * number_of_columns_) + column)] = value;
    }

    void EnsureRowCapacity(size_t minimum_rows) {
      TRACE("boundary check %d", minimum_rows);

      TRACE("boundary check %d %d", minimum_rows, number_of_rows_);
      if (minimum_rows > number_of_rows_) {
        // increase to given capacity or at least 120% of old number of rows
        int new_rows = std::max(minimum_rows, (number_of_rows_ * 6) / 5);
        TRACE("increase capacity to %d", new_rows);

        int* newDistanceMatrix = new int[new_rows * number_of_columns_];

        // copy the old array into the new one
        std::memcpy(newDistanceMatrix, distance_matrix_,
                    sizeof(int) * number_of_rows_ * number_of_columns_);

        delete[] distance_matrix_;

        // switch matrix
        distance_matrix_ = newDistanceMatrix;
        number_of_rows_ = new_rows;
      }
    }

    size_t Rows() const {
      return number_of_rows_;
    }

    size_t Columns() const {
      return number_of_columns_;
    }

   private:
    int* distance_matrix_;  //< internal matrix
    size_t number_of_columns_;  //< number of columns in the table. This is fixed at initialization time.
    size_t number_of_rows_;

  };
} /* namespace stringdistance */
} /* namespace keyvi */

#endif /* DISTANCE_MATRIX_H_ */
