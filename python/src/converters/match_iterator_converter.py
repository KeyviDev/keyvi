from __future__ import print_function
from autowrap.Code import Code
from autowrap.ConversionProvider import TypeConverterBase


class MatchIteratorPairConverter(TypeConverterBase):
    def get_base_types(self):
        return ("_MatchIteratorPair",)

    def matches(self, cpp_type):
        return not cpp_type.is_ptr

    def matching_python_type(self, cpp_type):
        return "MatchIterator"

    def output_conversion(self, cpp_type, input_cpp_var, output_py_var):
        return Code().add(
            """
            |cdef MatchIterator $output_py_var = MatchIterator.__new__(MatchIterator)
            |$output_py_var.it = _r.begin()
            |$output_py_var.end = _r.end()
            """,
            locals(),
        )
