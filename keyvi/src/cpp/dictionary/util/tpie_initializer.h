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
 * tpie_initializer.h
 *
 *  Created on: Jul 18, 2014
 *      Author: hendrik
 */

#ifndef TPIE_INITIALIZER_H_
#define TPIE_INITIALIZER_H_

#include "tpie/tpie.h"

namespace keyvi {
namespace dictionary {
namespace util {

/**
 * Helper (singleton) class to handle Tpie intialization
 */
class TpieIntializer
final
{
   public:
    static TpieIntializer& getInstance() {
      static TpieIntializer instance;
      return instance;
    }

    ~TpieIntializer() {
      tpie::tpie_finish();
    }

    void SetTempDirectory(const std::string& temp_path) const {
      tpie::tempname::set_default_path(temp_path);
    }

   private:
    // todo: only init whats needed tpie::flags<subsystem> tpie_systems_ = tpie::subsystem::ALL; // crashes, probably miss a service?: tpie::subsystem::MEMORY_MANAGER | tpie::subsystem::DEFAULT_LOGGING;

    TpieIntializer() {
      tpie::tpie_init();
    }

    TpieIntializer(TpieIntializer const&) = delete;
    void operator=(TpieIntializer const&) = delete;
  };

  } /* namespace util */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* TPIE_INITIALIZER_H_ */
