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
 * ivalue_store.h
 *
 *  Created on: Jun 12, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_INTERNAL_IVALUE_STORE_H_
#define KEYVI_DICTIONARY_FSA_INTERNAL_IVALUE_STORE_H_

#include <memory>
#include <string>

#include <boost/container/flat_map.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/variant.hpp>

#include "keyvi/dictionary/dictionary_merger_fwd.h"
#include "keyvi/dictionary/fsa/internal/value_store_properties.h"
#include "keyvi/dictionary/fsa/internal/value_store_types.h"
#include "keyvi/util/configuration.h"
#include "keyvi/util/json_value.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/*
 * Template that hold information about dictionary types, to be specialized.
 *
 * @tparam ValueStoreType The type of the value store to use
 * @tparam N Array size for fixed size value vectors, ignored otherwise
 */

template <value_store_t>
struct ValueStoreComponents {};

/* Writing value stores is based on template (duck-typing).
 * Base class / Interface definition for writing to the value store.
 *
 * The following types/constants/methods (incomplete list) are required:
 *
 * typedef {type} value_t;
 * static const {type} no_value = 0;
 *
 * uint64_t GetValue(value_t value, bool* no_minimization)
 *
 * value_store_t GetValueStoreType
 *
 * uint32_t GetWeightValue(value_t value)
 *
 * void CloseFeeding()
 *
 * void Write(std::ostream& stream)
 */

/**
 * Base class / Interface definition for reading from the value store.
 *
 * Each ValueStore Reader implementation should inherit this interface.
 *
 * In addition
 */
class IValueStoreReader {
 public:
  typedef boost::container::flat_map<std::string, boost::variant<std::string, int, double, bool>> attributes_raw_t;
  typedef std::shared_ptr<attributes_raw_t> attributes_t;

  /**
   * Default constructor. Override if the value store implementation requires extra data.
   *
   * @param file_mapping The file_mapping instance of the loader to use memory mapping
   * @param properties The dictionary properties
   */
  IValueStoreReader(boost::interprocess::file_mapping* file_mapping, const ValueStoreProperties& properties) {}

  virtual ~IValueStoreReader() {}

  virtual value_store_t GetValueStoreType() const = 0;

  /**
   * Generic value format.
   *
   * @param fsa_value numeric value
   * @return The value in form of attributes
   */
  virtual attributes_t GetValueAsAttributeVector(uint64_t fsa_value) const = 0;

  /**
   * Get Value as string in raw format
   *
   * Note: The raw format is an implementation detail of keyvi, not an official binary interface.
   * Value store implementers can override this method for performance reasons.
   *
   * @param fsa_value
   * @return the value as string without any decompression
   */
  virtual std::string GetRawValueAsString(uint64_t fsa_value) const {
    return keyvi::util::EncodeJsonValue(GetValueAsString(fsa_value));
  }

  /**
   * Get Value as string (for dumping or communication)
   *
   * @param fsa_value
   * @return the value as string
   */
  virtual std::string GetValueAsString(uint64_t fsa_value) const = 0;

  /**
   * Test whether this value store is compatible to the given value store.
   * Throws if they are not compatible.
   *
   * This can be overwritten by value stores, for specialization.
   *
   * @param other other value store
   */
  virtual void CheckCompatibility(const IValueStoreReader& other) {
    if (other.GetValueStoreType() != GetValueStoreType()) {
      throw std::invalid_argument("Dictionaries must have the same value store type");
    }
  }

 private:
  template <keyvi::dictionary::fsa::internal::value_store_t>
  friend class keyvi::dictionary::DictionaryMerger;

  virtual const char* GetValueStorePayload() const { return 0; }
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_INTERNAL_IVALUE_STORE_H_
