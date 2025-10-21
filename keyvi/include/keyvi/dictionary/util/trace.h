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
 * trace.h
 *
 *  Created on: May 12, 2014
 *      Author: hendrik
 */

//The following is left intentionally without include guard
//so that tracing can be switched on and off on a per file basis.
#ifdef ENABLE_TRACING
# undef TRACE
# define TRACE ::keyvi::dictionary::util::trace::trace_it
# undef ENABLE_TRACING
#else
# undef TRACE
# define TRACE(x,...)
#endif

#ifndef TRACE_H_
#define TRACE_H_

#include <cstdarg>
#include <cstdio>

namespace keyvi {
namespace dictionary {
namespace util {

class trace final {
 public:
    static void trace_it(const char* message, ...) {
      va_list arguments;
      va_start(arguments, message);

      fprintf(stderr, "* ");
      vfprintf(stderr, message, arguments);
      fprintf(stderr, "\n");
    }
  };

  } /* namespace util */
  } /* namespace dictionary */
  } /* namespace keyvi */
#endif /* TRACE_H_ */
