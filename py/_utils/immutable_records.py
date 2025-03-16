# **********************************************************************************************************************
# DESCRIPTION:
#   A record implementation for use in pyfns:
#   - currently based on dataclasses (which may change)
#   - records are immutable in the sense that fields are initialised to Missing and can be set only once and not deleted
#   - record types can be joined together at runtime, e.g. {a:str} * {b:int} => {a:str, b:int}
#   - record types be compared for equality and inequality with each other and other types
#   - unordered in the sense that Field Order is indiscernible except when unpacking to tuples
#   - can be converted to and from dictionaries (with runtime type checking)
#   - {x:int, y:float} and {x:float, y:float} are considered as distinct types
#
#   In time support the following:
#   - ability to name types created at runtime post creation
#   - keep a global set of types so joins that already exist will return current type rather than creating a duplicate
#   - automatically detect when initialised (i.e. when every field is <: its field type)
#   - ability to set canonical order of field names (once and only once)
#   - ability to compare two record instances subtype awareness, i.e. if A <: B then A == B and B != A is valid
#
#
# REFERENCES:
#  - https://en.wikipedia.org/wiki/Record_(computer_science)
#  - https://en.wikipedia.org/wiki/Product_type
#  - https://en.wikipedia.org/wiki/Function_type        aka Exponential Type, aka Arrow Type
#
# NOTES:
#  - For our purposes we define a type as a label used in a "type" system to do something useful. For us a type may
#    label memory layout such as a struct, e.g. a dict, or it may label a attribute, e.g. Mapping[Text, Type], for
#    which there may be more than one implementation.
#  - not aware of any Python type checker that can handle dynamically created types so mypy etc cannot penetrate
#    dynamically created records
#
# OPEN:
#  - figure out how mypy, pyright etc interact with dataclasses, if general make this implementation conform
# **********************************************************************************************************************


# Python imports
import dataclasses, itertools, types, typing
from dataclasses import dataclass
from typing import Mapping, Tuple, Type

# symmetry.common imports
from bones.core.sentinels import Missing
from bones.core.errors import NotYetImplemented



class ClassName(str): pass


allIRTypes: Mapping[ClassName, dataclass] = {}
_irTypeNameSeed = itertools.count(1)


class ImmutableRecordMeta:

    def __new__(cls, *args, **kwargs):
        return super().__new__(cls)

    def __ror__(self, other):  # other | self
        # return a Union
        return typing.Union[other, type(self)]

    def __mul__(self, other):  # self * other
        if isinstance(other, (ImmutableRecord, dict)):
            return joinImmutableRecordTypes(self, other)
        return NotImplemented

    def __rmul__(self, other):  # other * self
        if isinstance(other, (ImmutableRecord, dict)):
            return joinImmutableRecordTypes(other, self)
        raise NotImplementedError()

    def __le__(self, other):  # self <= other
        if isinstance(other, (ImmutableRecord, dict)):
            return fitsWithin(self, other)
        else:
            return False

    def __ge__(self, other):  # other <= self  aka self >= other
        # this is called if other returns NotImplemented for __le__
        if isinstance(other, (dict, ImmutableRecord)):
            return fitsWithin(other, self)
        else:
            return False


# OPEN: figure if possible to introduce a metaclass (the above) that works with dataclass
@dataclass(slots=True, init=False)
class ImmutableRecord:
    _initialised: bool

    def __init__(self, **kwargs):
        _underrides = kwargs.pop('_underrides', {})
        # ensure all fields are set to Missing
        for f in dataclasses.fields(self):
            object.__setattr__(self, f.name, Missing)
        # populate with kwargs
        self._setFromDict(kwargs)
        # finally if any fields are still Missing get them from the _underride dict if present
        for k, v in _underrides.items():
            if getattr(self, k, '__unlikely_name_called_joe__') is Missing:
                setattr(self, k, v)

    def __len__(self):
        num = 0
        for f in dataclasses.fields(self):
            if not f.name.startswith('_'): num += 1
        return num

    def _setFromDict(self, d: dict):
        for k, v in d.items():
            setattr(self, k, v)

    def __iter__(self):
        # for tuple unpacking, OPEN: use canonical order if set
        return iter(dataclasses.astuple(self))

    def __setattr__(self, name, value):
        currentValue = getattr(self, name, Missing)
        if currentValue is not Missing:
            raise AttributeError(f'Attribute {name} is already set')
        object.__setattr__(self, name, value)  # will raise AttributeError if name is not a field

    def __delattr__(self, name):
        raise AttributeError(f'Attribute {name} is already set')

    def __getitem__(self, name):
        # allow to be read like a dict (but no get method)
        return getattr(self, name)


ImmutableRecordType = type(ImmutableRecord)


def setCanonicalOrder(r: ImmutableRecordType) -> ImmutableRecordType:
    raise NotYetImplemented()


def nameAs(r: ImmutableRecordType, name: str) -> ImmutableRecordType:
    raise NotYetImplemented()


def immutable_record(*args, **kwargs):
    # a decorator to indicate that the class is defining a record

    if kwargs:
        raise TypeError('@immutable_record does not take kwargs')

    if len(args) == 1 and isinstance(args[0], type):
        newClass = dataclass(slots=True, init=False)(args[0])
        allIRTypes[newClass.__name__] = newClass
        return newClass
    else:
        raise TypeError(f'Invalid args passed - {[type(arg) for arg in args]}')


def joinImmutableRecordTypes(*args) -> ImmutableRecord:
    fields = {}
    for rt in args:
        for f in dataclasses.fields(rt):
            if f.name in fields:
                t = fields[f.name]
                if f.type != t:
                    raise TypeError(f'field {f.name} is already present')
                else:
                    continue
            fields[f.name] = f.type
    return newImmutableRecordType(**fields)


def newImmutableRecordType(*args, **kwargs) -> ImmutableRecord:
    if len(args) == 0:
        cls_name = f'RecordType{next(_irTypeNameSeed)}'
    else:
        cls_name = args[0]
    bases = (ImmutableRecord,)
    allIRTypes[cls_name] = rt = dataclasses.make_dataclass(cls_name, ((k, v) for k, v in kwargs.items()), bases=bases,
                                                           init=False, slots=True)
    return rt


_fitsCache: Mapping[Tuple[Type, Type], bool] = {}


def fitsWithin(A, B) -> bool:
    # Returns True if type A fits within type B, i.e. an instance of A can be used where an instance of B is expected
    # OPEN: noting a reason on failure would be helpful
    if not isinstance(A, type) and not isUnion(A): raise TypeError('A must be a type or a Union of types')
    if not isinstance(B, type) and not isUnion(B): raise TypeError('B must be a type or a Union of types')

    if (cached := _fitsCache.get((A, B), Missing)) is not Missing: return cached
    if A == B:
        _fitsCache[(A, B)] = True
        return True

    if isUnion(A):
        if isUnion(B):
            for e in typing.get_args(A):
                if not fitsWithin(e, B):
                    _fitsCache[(A, B)] = False
                    return False
            _fitsCache[(A, B)] = True
            return True
        else:
            _fitsCache[(A, B)] = False
            return False
    elif issubclass(A, ImmutableRecord):
        if isUnion(B):
            for e in typing.get_args(B):
                if fitsWithin(A, e):
                    _fitsCache[(A, B)] = True
                    return True
            _fitsCache[(A, B)] = False
            return False
        elif issubclass(B, ImmutableRecord):
            # for A to fit in B, A must have all the fields of B and each field in A must fit in the corresponding field in B
            aFields = {f.name: f for f in dataclasses.fields(A)}
            for bField in dataclasses.fields(B):
                bName = bField.name
                if bName in ('_canonical_order', '_name', '_initialised'): continue
                aField = aFields.get(bName, Missing)
                if aField is Missing or not fitsWithin(aField.type, bField.type):
                    _fitsCache[(A, B)] = False
                    return False
            _fitsCache[(A, B)] = True
            return True
        else:
            _fitsCache[(A, B)] = False
            return False
    else:
        if issubclass(A, B):  # use nominal subtyping for non-ImmutableRecords
            _fitsCache[(A, B)] = True
            return True
        else:
            _fitsCache[(A, B)] = False
            return False


def isUnion(t) -> bool:
    return (o := typing.get_origin(t)) is types.UnionType or o is typing.Union

# Ideas to keep for a while

# PoD
# purpose - scale the ids p/l breakdown report (potentially with mypy?)
# problem with dicts is they are exponentials (all fields are same type)
#
# IDEALLY
# PoD is product not an exponential
# can map to and from exponentials - tuple unpacking and serialisation to / from dicts
# a type must ba able to represent memory layout but is more than memory layout and sometimes is not memory layout
# PyCharm / mypy hints - only possible for PoD defined as class
# strongly typed - type(aPoD), isinstance(aPoD, FCB) - with subtypes, unions?, certainly subset / super set of fields
# dynamically created
# ad-hoc - type can be created by extending an instance
# but is {x:Number} and {x:Text} a TypeError or two distinct but unnamed types?
# type names would obviously be unique
# optionally immutable - must be fully constructed in one call
# just attributes, index access too?, field names are Capitalised
# can convert a seq of PoD to a Frame
# can tuple unpack a PoD
# can convert a Frame to a seq of PoD
#
# cannot extend via
# fcb.fred = 1
# fcb.with(fred=1) - answer a new PoD with the correct type
#
# assignment is dynamically type checked
# type construction
# instance construction from type
# structurally subtyped
# enforce simple types?
#
# unions probably make it difficult
# intersections - e.g. float and interestRate
#
# type can be named else PoD{'name':str, 'age':float} Person{'name':str, 'age':float}
#


# OPEN: replace this with relevant datastructure type once have understood the requirements and options available
# OPEN: enforce fieldnames must begin with a capital letter

# class PoD:
#
#     __slots__ = ['_dict']
#
#     def __new__(cls, *args, **kwargs):
#         # args is List(str) or Tuple(str) or Dict(str, Type)
#         if len(args) == 1:
#             if isinstance(args[0], (list, tuple)):
#                 new_instance = super().__new__(cls)
#                 new_instance._dict = {k: Missing for k in args[0]}
#                 return new_instance
#             elif isinstance(args[0], dict):
#                 new_instance = super().__new__(cls)
#                 new_instance._dict = dict(args[0])
#                 return new_instance
#         raise NotYetImplemented()
#
#     def __getattribute__(self, name):
#         if name == '_dict':
#             return super().__getattribute__('_dict')
#         answer = super().__getattribute__('_dict').get(name, '__UNLIKELY_VALUE_CALLED_FRED__')
#         if answer == '__UNLIKELY_VALUE_CALLED_FRED__':
#             raise AttributeError(f'"{name}" is not a valid field name')
#         else:
#             return answer
#
#     def __setattr__(self, name, value):
#         if name == '_dict':
#             super().__setattr__('_dict', value)
#             return self
#         _dict = super().__getattribute__('_dict')
#         if name not in _dict: raise AttributeError(f'"{name}" is not a valid field name')
#         _dict[name] = value
#         return self
#
#     def __iter__(self):
#         return iter(super().__getattribute__('_dict').values())
#
#     def __dir__(self):
#         return iter(super().__getattribute__('_dict').keys())
#
#     def __str__(self):
#         return str(self._dict)
#
