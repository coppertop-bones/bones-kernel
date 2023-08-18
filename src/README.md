bones kernel is a dynamic link library, i.e. .so

bk/ bones kernel
jones/ python interface to bk


// ---------------------------------------------------------------------------------------------------------------------
// Writing the Setup Configuration File - https://docs.python.org/3/distutils/configfile.html
// build with > python setup.py build
// ---------------------------------------------------------------------------------------------------------------------


case styles for variable names within jones:
- t_integral_types
- equationvaluesarehandcasetokeepthemshort
- objects_are_snakecase
- *pointers_to_obejcts_are_not_typedeffed
- PyTypesArePascal
- pythonObjectsAreCamel
- CONSTANT_ARE_BLOCK_CAPITALS
- JonesTypesArePascalWRT

null terminated strings are unsigned char
t_utf8 indicates it maybe uft8
