cmake_minimum_required(VERSION 3.10)
project(Test_Adios_op CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Debug mode by default (adds -g, disables optimization)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Optionally add more detailed debugging flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -O0 -g3")

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZSTD REQUIRED libzstd)

# --- define missing alias expected by Caesar ---
add_library(PkgConfig::ZSTD INTERFACE IMPORTED)
set_target_properties(PkgConfig::ZSTD PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${ZSTD_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${ZSTD_LIBRARIES}"
)

find_package(MPI REQUIRED)
find_package(ADIOS2 REQUIRED)
find_package(CAESAR REQUIRED)
find_package(Torch QUIET)

# --- gen_data ---
add_executable(gen_data gen_data.cpp)
target_link_libraries(gen_data
    PRIVATE
        MPI::MPI_CXX
        adios2::cxx11_mpi
        ${ZSTD_LIBRARIES}
)

# --- compress_data ---
add_executable(compress_data compress_data.cpp)
target_link_libraries(compress_data
    PRIVATE
        MPI::MPI_CXX
        adios2::cxx11_mpi
        caesar::caesar
        ${ZSTD_LIBRARIES}
)

# --- test_decompress ---
add_executable(test_decompress test_decompress.cpp)
target_link_libraries(test_decompress
    PRIVATE
        MPI::MPI_CXX
        adios2::cxx11_mpi
        caesar::caesar
        ${ZSTD_LIBRARIES}
)

add_executable(convert_to_bin convert_to_bin.cpp)
target_link_libraries(convert_to_bin
    PRIVATE
        MPI::MPI_CXX
        adios2::cxx11_mpi
        ${ZSTD_LIBRARIES}
)


add_executable(gen_small_data gen_small_data.cpp)
target_link_libraries(gen_small_data
    PRIVATE
        MPI::MPI_CXX
        adios2::cxx11_mpi
        ${ZSTD_LIBRARIES}
)
