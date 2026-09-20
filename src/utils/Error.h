#ifndef SMARTDATAVISUALISATION_ERROR_H
#define SMARTDATAVISUALISATION_ERROR_H

#include <string>

namespace sdv::utils {

enum class ErrorCode {
    None,
    FileNotFound,
    FileReadError,
    FileWriteError,
    InvalidData,
    InvalidConfiguration,
    UnsupportedFormat,
    EmptyDataset,
    UnknownError
};

struct Error {
    ErrorCode code = ErrorCode::None;
    std::string message;

    bool hasError() const {
        return code != ErrorCode::None;
    }
};

} // namespace sdv::utils

#endif // SMARTDATAVISUALISATION_ERROR_H
