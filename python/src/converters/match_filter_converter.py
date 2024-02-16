from typing import Tuple
from autowrap.Types import CppType
from autowrap.ConversionProvider import TypeConverterBase


class MatchFilterConverter(TypeConverterBase):
    def get_base_types(self):
        return ("match_filter",)

    def matches(self, cpp_type):
        return not cpp_type.is_ptr

    def matching_python_type(self, cpp_type):
        return ""

    def type_check_expression(self, cpp_type: CppType, argument_var: str) -> str:
        return "isinstance(%s, object)" % (argument_var,)

    def input_conversion(
        self, cpp_type: CppType, argument_var: str, arg_num: int
    ) -> Tuple[str, str, str]:
        return "", "filter_callback, <void*> %s" % argument_var, ""
