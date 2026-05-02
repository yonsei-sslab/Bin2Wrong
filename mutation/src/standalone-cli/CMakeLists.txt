cmake_minimum_required(VERSION 3.13)
project(bin2wrong_standalone CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Standalone CLI build for Bin2Wrong source mutators (Yonsei extension).
# Links against the upstream code-mutators + utils-fuzzer code; no AFL++ harness.
# Locate LLVM/Clang via CMAKE_PREFIX_PATH (e.g. set CONDA_PREFIX with conda env).
find_package(ZLIB REQUIRED)
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Clang include: ${CLANG_INCLUDE_DIRS}")

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# upstream Bin2Wrong sources (one level up from this CMakeLists.txt)
set(BIN2WRONG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(graycutils STATIC
    ${BIN2WRONG_SRC}/utils-fuzzer/GrayCUtils.cpp
    ${BIN2WRONG_SRC}/utils-fuzzer/GrayCCustomRandom.cpp
)

add_library(codemutators STATIC
    ${BIN2WRONG_SRC}/code-mutators/CodeMutators.cpp
)
target_link_libraries(codemutators PUBLIC graycutils)

add_executable(mutate_source main.cpp)
target_link_libraries(mutate_source PRIVATE
    codemutators
    clangTooling
    clangFrontend
    clangAST
    clangBasic
    clangRewrite
    clangLex
    clangSerialization
    clangDriver
    clangParse
    clangSema
    clangAnalysis
    clangEdit
    clangSupport
    LLVMSupport
)

install(TARGETS mutate_source RUNTIME DESTINATION bin)
