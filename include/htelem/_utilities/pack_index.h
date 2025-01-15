#ifndef HTELEM__UTILITIES_PACK_INDEX_H
#define HTELEM__UTILITIES_PACK_INDEX_H

#include <tuple> // IWYU pragma: keep

#if __cpp_pack_indexing >= 202311L
#define _ht_pack_index(pack, idx) pack...[idx]
#else
#define _ht_pack_index(pack, idx) std::tuple_element_t<idx, std::tuple<pack...>>
#endif

#endif

