//
// keyvi - A key value store.
//
// Copyright 2018 Hendrik Muhs<hendrik.muhs@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/*
 * types.h
 *
 *  Created on: Sep 6, 2019
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_TYPES_H_
#define KEYVI_INDEX_TYPES_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace keyvi {
namespace index {

using key_value_vector_t = std::vector<std::pair<std::string, std::string>>;
using key_values_ptr_t = std::shared_ptr<key_value_vector_t>;

} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_TYPES_H_
