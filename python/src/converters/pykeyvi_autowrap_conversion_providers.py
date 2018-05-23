from __future__ import print_function
from autowrap.Code import Code
from autowrap.ConversionProvider import TypeConverterBase


class MatchIteratorPairConverter(TypeConverterBase):

    def get_base_types(self):
        return "_MatchIteratorPair",

    def matches(self, cpp_type):
        return not cpp_type.is_ptr

    def matching_python_type(self, cpp_type):
        return "MatchIterator"

    #def type_check_expression(self, cpp_type, argument_var):
    #    if cpp_type.is_ref:
    #        return "isinstance(%s, String)" % (argument_var,)
    #    return "isinstance(%s, bytes)" % (argument_var,)

    #def input_conversion(self, cpp_type, argument_var, arg_num):
        #if cpp_type.is_ref:
        #    call_as = "deref(%s.inst.get())" % argument_var
        #else:
        #    call_as = "(_String(<char *>%s))" % argument_var
        #code = cleanup = ""
        #return code, call_as, cleanup

    def output_conversion(self, cpp_type, input_cpp_var, output_py_var):
        return Code().add("""
            |cdef MatchIterator $output_py_var = MatchIterator.__new__(MatchIterator)
            |$output_py_var.it = _r.begin()
            |$output_py_var.end = _r.end()
            """, locals())
