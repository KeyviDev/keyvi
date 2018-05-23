from .pykeyvi_autowrap_conversion_providers import *
from autowrap.ConversionProvider import special_converters


def register_converters():
    special_converters.append(MatchIteratorPairConverter())
