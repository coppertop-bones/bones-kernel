## tm - type manager
product_size_type - u16 - handles products up to 64k in length


bt_summaries - array of bt_summary - indexed by btype \
bt_sizes - array of product_size_type - indexed by btype \
bt_desc_rps - array of bt_desc_rp - indexed by btype \
bt_name_rps - array of istring_rp - indexed by btype (sparce so could be optimised with a 1024 length probe and hash) \
bt_name_rpToBt_idMap - hash to get the btype for a name \
btypelists - buffer of btypelist - used for intersectDescs, unionDescs and tupleDescs \
btypelistoffsets - buffer of btypeoffsetlist - used for intersectDescs, unionDescs and tupleDescs - not sure about it yet \
symlists - buffer of symlist - e.g. for records, structs, fns, etc \
btypelist_rpToTuple_idMap - hash to get a tuples btype from an ordered list of types - also can find a 
btypelist_rp from a list of btype \
structDescs - buffer of structDesc \
seqDescs - array of seqDesc \
mapDescs - array of mapDesc \
fnDescs - array of fnDesc \
also need hash maps to convert descriptions into RPs and IDs


### bkheader
u32 (since u16 can only handle 64k types and no extra flags), e.g.: \
familial - 1 bit \
explicit - 1 bit \
orthogonal - 1 bit \
isPtr - 1 bit - can deref immediately \
hasPtr - 1 bit - needs scanning if hasPtr, if not hasPtr we know it is contiguous memory \
hasT - 1 bit - code gen might be needed \
unused - 2 bit \
24 lsbs reserved as btype


### bt_summary - 1 byte
isPtr - 1 bit - should this be encoded in the typeId for faster mm?
hasPtr - 1 bit
hasT - 1 bit
unused - 1 bit
bmt enum - 4 bits
- btill = 0,  // illegal metatype
- btnom = 1,  // nominal - atomic type with a given name
- btint = 2,  // intersection - sorted list of other types
- btuni = 3,  // union - sorted list of other types
- bttup = 4,  // tuple - ordered list of other types
- btstr = 5,  // struct - ordered and named list of other types
- btrec = 6,  // record - sorted named list of other types - not yet implemented
- btseq = 7,  // sequence - tElement
- btmap = 8,  // map / dictionary - tKey, tValue
- btfnc = 9,  // function - argnames, tArgs, tRet, tFunc, num args
- btsvr = 10, // schema variable
inference metatypes? 11 - 15


### bt_desc_rp - 4 bytes
familial - 1 bit
explicit - 1 bit
orthogonal - 1 bit
unused - 5 bits
lower 24 bits - descRP into the relevant description array


### btypelist - variable
size prefixed list of btype


### btypeoffsetlist - variable
size prefixed list of product_size_type


### symlist - variable
size prefixed list of symId


### structDesc - 8 bytes
symlistRP
btypelist or btypelistRP - former might be faster 1 less indirect but maybe slower as not cache hot?


### seqDesc - 4 bytes
value_btype


### mapDesc - 8 bytes
key_bt_id
value_bt_id


### fnDesc - 8 bytes
args_bt_rp - structDescRP
value_bt_id



OPEN: does recusion need explicitly modelling?

