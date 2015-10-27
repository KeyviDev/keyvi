/*
 * entry_iterator_test.cpp
 *
 *  Created on: Oct 27, 2015
 *      Author: hendrik
 */




#include <boost/test/unit_test.hpp>
#include "dictionary/fsa/entry_iterator.h"
#include "dictionary/testing/temp_dictionary.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

BOOST_AUTO_TEST_SUITE( EntryIteratorTests )

BOOST_AUTO_TEST_CASE( EmptyDictionary ) {

  // empty dictionary
  std::vector<std::string> test_data =
          { };
  testing::TempDictionary dictionary(test_data);
  automata_t f = dictionary.GetFsa();

  EntryIterator it(f);
  EntryIterator end_it = EntryIterator();

  BOOST_CHECK(end_it == it);
}

BOOST_AUTO_TEST_SUITE_END()

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */
