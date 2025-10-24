/*
 * attributes_converter.h
 *
 *  Created on: May 28, 2015
 *      Author: hendrik
 */

#ifndef ATTRIBUTES_CONVERTER_H_
#define ATTRIBUTES_CONVERTER_H_

#include <Python.h>
#include <boost/variant.hpp>

namespace keyvi {
namespace python {

class attributes_visitor : public boost::static_visitor<PyObject*>
{
public:
  PyObject* operator()(int i) const
    {
        return PyLong_FromLong(i);
    }

  PyObject* operator()(const std::string & str) const
    {
        return PyString_FromString(str.c_str());
    }
};

inline PyObject* Convert (dictionary::Match& m,  const std::string& key) {
    auto result = m.GetAttributeVar(key);
    return boost::apply_visitor( attributes_visitor(), result );
  }

} /* namespace python */
} /* namespace keyvi */

#endif /* ATTRIBUTES_CONVERTER_H_ */
