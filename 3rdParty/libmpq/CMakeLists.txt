find_package(ZLIB REQUIRED)

if(NOT TARGET BZip2::BZip2)
  find_package(BZip2 REQUIRED)
endif()

include(FetchContent_MakeAvailableExcludeFromAll)

include(FetchContent)
FetchContent_Declare(libmpq
    URL https://github.com/diasurgical/libmpq/archive/b7907a9e56cbb647e10b9a9df39c88a321efc0c0.tar.gz
    URL_HASH MD5=25d9762b9ad81caa33f74c317fdbe404
)
FetchContent_MakeAvailableExcludeFromAll(libmpq)

add_library(libmpq STATIC
  ${libmpq_SOURCE_DIR}/libmpq/common.c
  ${libmpq_SOURCE_DIR}/libmpq/explode.c
  ${libmpq_SOURCE_DIR}/libmpq/extract.c
  ${libmpq_SOURCE_DIR}/libmpq/huffman.c
  ${libmpq_SOURCE_DIR}/libmpq/mpq.c
  ${libmpq_SOURCE_DIR}/libmpq/wave.c
)

target_include_directories(libmpq PUBLIC ${libmpq_SOURCE_DIR})
target_include_directories(libmpq PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(libmpq PRIVATE ZLIB::ZLIB BZip2::BZip2)
