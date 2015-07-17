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
 * levenshtein.h
 *
 *  Created on: Jun 24, 2014
 *      Author: hendrik
 */

#ifndef LEVENSHTEIN_H_
#define LEVENSHTEIN_H_

#include "stringdistance/needleman_wunsch.h"
#include "stringdistance/costfunctions/damerau_levenshtein.h"

namespace keyvi {
namespace stringdistance {

// Levenshtein is NeedlemanWunsch with constant cost of 1 for all operations
typedef NeedlemanWunsch<costfunctions::Damerau_Levenshtein> Levenshtein;
typedef std::shared_ptr<Levenshtein> levenshtein_t;

} /* namespace stringdistance */
} /* namespace keyvi */

#endif /* LEVENSHTEIN_H_ */
