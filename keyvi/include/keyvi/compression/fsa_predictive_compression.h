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
 * fsa_predictive_compression.h
 *
 *  Created on: Apr 10, 2015
 *      Author: hendrik
 */

#ifndef FSA_PREDICTIVE_COMPRESSION_H_
#define FSA_PREDICTIVE_COMPRESSION_H_

#include <bitset>
#include <sstream>
#include "dictionary/dictionary.h"
#include "dictionary/fsa/automata.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace compression {

/**
 * Short string compression inspired by RFC 1978 (Predictor Compression Protocol)
 */
class FsaPredictiveCompression final {

 public:
  FsaPredictiveCompression(dictionary::fsa::automata_t fsa): fsa_(fsa){
  }

  FsaPredictiveCompression(dictionary::dictionary_t d) {
    fsa_ = d->GetFsa();
  }

  // highly inefficient for now
  std::string LookupBigram(const unsigned char* bigram){

    // skip for null bytes
    if (bigram[0] == 0 || bigram[1] == 0){
      return "";
    }

    uint64_t state = fsa_->GetStartState();
    state = fsa_->TryWalkTransition(state, bigram[0]);
    if (!state) {
      TRACE ("LookupBigram: %c%c failed.", bigram[0], bigram[1]);

      return "";
    }
    state = fsa_->TryWalkTransition(state, bigram[1]);

    if (!state) {
      return "";
    }

    return fsa_->GetValueAsString(fsa_->GetStateValue(state));
  }

  std::string Compress(const std::string& input){
    std::ostringstream output_buffer;

    size_t input_length = input.size();
    size_t offset = 0;
    std::bitset<8> current_bitset;
    char uncompressed_buf[8];
    unsigned char bigram[2];
    size_t bitset_position = 0;
    size_t uncompressed_position  = 0;

    TRACE("Compressing %s", input.c_str());

    if (input_length < 2) {
      return input;
    }

    // setup first character
    bigram[0] = input[0];
    bigram[1] = input[1];
    bitset_position = 2;
    uncompressed_buf[0] = input[0];
    uncompressed_buf[1] = input[1];
    uncompressed_position = 2;

    offset = 2;
    while ( offset < input_length ) {

      std::string prediction = LookupBigram(bigram);
      if (prediction.size() > 0 && prediction == input.substr(offset, prediction.size())) {
        // prediction succeeded
        TRACE ("Prediction success: %s", prediction.c_str());
        current_bitset.set(bitset_position++);
        offset += prediction.size();

        if (prediction.size() > 1) {
          bigram[0] = prediction[prediction.size() - 2];
        } else {
          bigram[0] = bigram[1];
        }

        bigram[1] = prediction[prediction.size() - 1];

      } else {
        // prediction failed
        TRACE ("Prediction failed: %s", prediction.c_str());

        current_bitset.set(bitset_position++, 0);
        uncompressed_buf[uncompressed_position++] = input[offset];

        bigram[0] = bigram[1];
        bigram[1] = input[offset++];
      }

      // check for flush
      if (bitset_position == 8) {
        TRACE("Flush bitset:%s data: '%s' ", current_bitset.to_string().c_str(),
              std::string(uncompressed_buf, uncompressed_position).c_str());

        output_buffer.put(static_cast<unsigned char>( current_bitset.to_ulong() ));
        output_buffer.write(uncompressed_buf, uncompressed_position);
        bitset_position =0;
        current_bitset.reset();
        uncompressed_position = 0;
      }
    }

    // write remainder
    if (bitset_position != 0) {
      TRACE("Last chunk bitset:%s data: '%s' ", current_bitset.to_string().c_str(),
                    std::string(uncompressed_buf, uncompressed_position).c_str());

      output_buffer.put(static_cast<unsigned char>( current_bitset.to_ulong() ));
      output_buffer.write(uncompressed_buf, uncompressed_position);
    }

    TRACE("Compressed string: %s", output_buffer.str().c_str());

    return output_buffer.str();
  }

  std::string Uncompress(const std::string & input) {
    if (input.size() < 2) {
      return input;
    }

    std::ostringstream output_buffer;

    size_t input_length = input.size();
    size_t offset = 0;
    std::bitset<8> current_bitset;
    //char uncompressed_buf[8];
    unsigned char bigram[2];
    size_t bitset_position = 0;

    // setup first character
    bigram[0] = input[1];
    bigram[1] = input[2];
    current_bitset = static_cast<unsigned long> (input[0]);
    bitset_position = 2;
    output_buffer.put(input[1]);
    output_buffer.put(input[2]);

    offset = 3;

    while ( offset < input_length ) {
      if (current_bitset[bitset_position++]) {
        TRACE("Bit is set, do lookup ");
        std::string prediction = LookupBigram(bigram);
        TRACE("write buffer %s", prediction.c_str());
        output_buffer << prediction;
        if (prediction.size() > 1) {
          bigram[0] = prediction[prediction.size() - 2];
        } else {
          bigram[0] = bigram[1];
        }

        bigram[1] = prediction[prediction.size() - 1];
      } else {
        TRACE("Bit is not set, take raw buffer %c", input[offset]);
        output_buffer.put(input[offset]);
        bigram[0] = bigram[1];
        bigram[1] = input[offset++];
      }

      if (bitset_position == 8) {
        TRACE("Read next chunk");

        current_bitset = static_cast<unsigned long> (input[offset++]);
        bitset_position = 0;
      }
    }

    TRACE("Uncompressed string: %s", output_buffer.str().c_str());
    return output_buffer.str();
  }

 private:
  dictionary::fsa::automata_t fsa_;

};


} /* namespace compression */
} /* namespace keyvi */



#endif /* FSA_PREDICTIVE_COMPRESSION_H_ */

