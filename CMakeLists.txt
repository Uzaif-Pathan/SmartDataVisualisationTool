cmake_minimum_required(VERSION 3.16)

project(SmartDataVisualisationTool
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_path(ARCHIVE_INCLUDE_DIR
    NAMES archive.h
)

find_library(ARCHIVE_LIBRARY
    NAMES archive
)

if(NOT ARCHIVE_INCLUDE_DIR OR NOT ARCHIVE_LIBRARY)
    message(FATAL_ERROR
        "libarchive development files not found. "
        "Install them with: sudo apt install libarchive-dev"
    )
endif()

find_path(TINYXML2_INCLUDE_DIR
    NAMES tinyxml2.h
)

find_library(TINYXML2_LIBRARY
    NAMES tinyxml2
)

if(NOT TINYXML2_INCLUDE_DIR OR NOT TINYXML2_LIBRARY)
    message(FATAL_ERROR
        "TinyXML2 development files not found. "
        "Install them with: sudo apt install libtinyxml2-dev"
    )
endif()

add_executable(${PROJECT_NAME}
    src/main/main.cpp

    src/core/ApplicationState.cpp

    src/data/DataValue.cpp
    src/data/DataColumn.cpp
    src/data/DataRow.cpp
    src/data/DataTable.cpp
    src/data/DataSource.cpp
    src/data/FileFormatDetector.cpp
    src/data/CSVParser.cpp
    src/data/CSVReader.cpp
    src/data/CSVWriter.cpp
    src/data/ExcelReader.cpp
    src/data/DataLoader.cpp
    src/statistics/NumericalStatistics.cpp
    src/statistics/CategoricalStatistics.cpp
    src/statistics/CorrelationStatistics.cpp
    src/statistics/StatisticsEngine.cpp
    src/utils/FileUtils.cpp
    src/utils/PathUtils.cpp
    src/utils/StringUtils.cpp
    src/utils/DateUtils.cpp
    src/utils/NumberUtils.cpp
    src/utils/MathUtils.cpp
    src/utils/Logger.cpp
    src/utils/Error.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${ARCHIVE_INCLUDE_DIR}
        ${TINYXML2_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${ARCHIVE_LIBRARY}
        ${TINYXML2_LIBRARY}
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        SDV_VERSION="${PROJECT_VERSION}"
)
enable_testing()

add_executable(StatisticsTest
    tests/statistics_test.cpp

    src/core/ApplicationState.cpp

    src/data/DataValue.cpp
    src/data/DataColumn.cpp
    src/data/DataRow.cpp
    src/data/DataTable.cpp
    src/data/DataSource.cpp
    src/data/FileFormatDetector.cpp
    src/data/CSVParser.cpp
    src/data/CSVReader.cpp
    src/data/CSVWriter.cpp
    src/data/ExcelReader.cpp
    src/data/DataLoader.cpp

    src/statistics/NumericalStatistics.cpp
    src/statistics/CategoricalStatistics.cpp
    src/statistics/CorrelationStatistics.cpp
    src/statistics/StatisticsEngine.cpp

    src/utils/FileUtils.cpp
    src/utils/PathUtils.cpp
    src/utils/StringUtils.cpp
    src/utils/DateUtils.cpp
    src/utils/NumberUtils.cpp
    src/utils/MathUtils.cpp
    src/utils/Logger.cpp
    src/utils/Error.cpp
)

target_include_directories(StatisticsTest
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${ARCHIVE_INCLUDE_DIR}
        ${TINYXML2_INCLUDE_DIR}
)

target_link_libraries(StatisticsTest
    PRIVATE
        ${ARCHIVE_LIBRARY}
        ${TINYXML2_LIBRARY}
)

target_compile_options(StatisticsTest
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_compile_definitions(StatisticsTest
    PRIVATE
        SDV_VERSION="${PROJECT_VERSION}"
)

add_test(
    NAME StatisticsTest
    COMMAND StatisticsTest
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
