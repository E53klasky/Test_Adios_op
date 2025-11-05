cmake_minimum_required(VERSION 3.10)
project(Test_Adios_op CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
        caesar::caesar_lib
        ${ZSTD_LIBRARIES}
)
