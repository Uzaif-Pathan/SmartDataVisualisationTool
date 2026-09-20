#ifndef SMARTDATAVISUALISATION_TYPES_H
#define SMARTDATAVISUALISATION_TYPES_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace sdv {

using RowIndex = std::size_t;
using ColumnIndex = std::size_t;
using NumericValue = double;

using StringList = std::vector<std::string>;

} // namespace sdv

#endif // SMARTDATAVISUALISATION_TYPES_H
