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
 * fsa_transform.h
 *
 *  Created on: Apr 4, 2015
 *      Author: hendrik
 */

#ifndef FSA_TRANSFORM_H_
#define FSA_TRANSFORM_H_

#include <sstream>
#include "dictionary/fsa/automata.h"
#include "dictionary/dictionary.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace transform {

class FsaTransform final {

 public:
  FsaTransform(dictionary::fsa::automata_t fsa): fsa_(fsa){
  }

  FsaTransform(dictionary::dictionary_t d) {
      fsa_ = d->GetFsa();
  }

  std::string Normalize(const std::string& input) const {
    std::ostringstream output_buffer;

    uint64_t state = fsa_->GetStartState();
    size_t input_length = input.size();
    uint64_t last_final_state = 0;
    size_t last_final_state_position = 0;
    size_t current_matching_depth = 0;
    size_t offset = 0;

    TRACE("Normalizing %s", input.c_str());
    while ( offset < input_length ) {

      state = fsa_->TryWalkTransition(state, input[offset]);

      if (state){
        // got a match
        TRACE("Matched at %d", offset);

        ++current_matching_depth;

        if (fsa_->IsFinalState(state)) {
          last_final_state = state;
          last_final_state_position = current_matching_depth;
        }

      } else {
        // no match

        if (last_final_state){

          TRACE("Write normalization");

          output_buffer << fsa_->GetValueAsString(fsa_->GetStateValue(last_final_state));
          last_final_state = 0;

          // calculate the offset in the input buffer, that's the offset
          // before traversal plus the offset till the last final match
          offset -= ( current_matching_depth - last_final_state_position + 1 );
        } else {

          // just write the plain input
          output_buffer.put(input[offset]);
          offset -= current_matching_depth;
        }

        current_matching_depth = 0;
        state = fsa_->GetStartState();
      }

      ++offset;
    }

    if (last_final_state){
      TRACE("write pending match");
      output_buffer << fsa_->GetValueAsString(fsa_->GetStateValue(last_final_state));

      // if we had a partial match and a partial traversal, add the original strings at the end
      while (current_matching_depth > last_final_state_position) {
        output_buffer.put(input[offset + current_matching_depth - last_final_state_position]);
        ++ last_final_state_position;
      }
    }
    TRACE("Normalization result: %s", output_buffer.str().c_str());
    return output_buffer.str();
  }

 private:
  dictionary::fsa::automata_t fsa_;

};


} /* namespace transform */
} /* namespace keyvi */




#endif /* FSA_TRANSFORM_H_ */
