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
 * generator_adapter.h
 *
 * Interface to abstract away compile time(template) parameters of the generator.
 *  Created on: Oct 28, 2015
 *      Author: hendrik
 */

#ifndef GENERATOR_PIMPL_H_
#define GENERATOR_PIMPL_H_

#include "dictionary/fsa/generator.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

template<class PersistenceT, class ValueStoreT>
class GeneratorAdapterInterface {
 public:
  GeneratorAdapterInterface(){}

  virtual void Add(const std::string& input_key, typename ValueStoreT::value_t value =
               ValueStoreT::no_value) {}
  virtual void Add(const std::string& input_key, const fsa::ValueHandle& value) {}

  virtual size_t GetFsaSize() const {return 0;}
  virtual void CloseFeeding() {}
  virtual void Write(std::ostream& stream) {}
  virtual void WriteToFile(const std::string& filename) {}
  virtual void SetManifestFromString(const std::string& manifest) {}
  virtual void SetManifest(const boost::property_tree::ptree& manifest) {}

  virtual ~GeneratorAdapterInterface(){}
};

template<class PersistenceT, class ValueStoreT, class OffsetTypeT, class HashCodeTypeT>
class GeneratorAdapter final: public GeneratorAdapterInterface<PersistenceT, ValueStoreT> {
 public:
  GeneratorAdapter(size_t memory_limit = 1073741824,
                   const generator_param_t& value_store_params = generator_param_t(), ValueStoreT* value_store = NULL):
                     generator_(memory_limit, value_store_params, value_store)
 {}

  void Add(const std::string& input_key, typename ValueStoreT::value_t value =
                 ValueStoreT::no_value) {
    generator_.Add(std::move(input_key), value);
  }

  void Add(const std::string& input_key, const fsa::ValueHandle& value) {
    generator_.Add(std::move(input_key), value);
  }

  size_t GetFsaSize() const {
    return generator_.GetFsaSize();
  }

  void CloseFeeding(){
    generator_.CloseFeeding();
  }

  void Write(std::ostream& stream) {
    generator_.Write(stream);
  }

  void WriteToFile(const std::string& filename) {
    generator_.WriteToFile(filename);
  }

  void SetManifestFromString(const std::string& manifest) {
    generator_.SetManifestFromString(manifest);
  }

  void SetManifest(const boost::property_tree::ptree& manifest) {
    generator_.SetManifest(manifest);
  }

 private:
  Generator<PersistenceT, ValueStoreT, OffsetTypeT, HashCodeTypeT> generator_;

};

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* GENERATOR_PIMPL_H_ */
