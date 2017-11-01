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
 * damerau_levenshtein.h
 *
 *  Created on: Jul 16, 2014
 *      Author: hendrik
 */

#ifndef DAMERAU_LEVENSHTEIN_H_
#define DAMERAU_LEVENSHTEIN_H_


namespace keyvi {
namespace stringdistance {
namespace costfunctions {

/**
 * A Damerau Levenshtein cost function which returns 1 for mismatch, insertion, deletion and transposition.
 */
class Damerau_Levenshtein final {
 public:
  int GetSubstitutionCost(int codepoint_from, int codepoint_to){
    if (codepoint_from != codepoint_to) {
      return 1;
    }

    return 0;
  }

  int GetInsertionCost(){
      return 1;
  }

  int GetDeletionCost(){
    return 1;
  }

  int GetTranspositionCost(){
    return 1;
  }
};

} /* namespace costfunctions */
} /* namespace stringdistance */
} /* namespace keyvi */


#endif /* DAMERAU_LEVENSHTEIN_H_ */
