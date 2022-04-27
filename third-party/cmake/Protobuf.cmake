include(ExternalProject)

set(PROTOBUF_TAR_GZ https://github.com/protocolbuffers/protobuf/releases/download/v3.20.1/protobuf-cpp-3.20.1.tar.gz)
set(protobuf_DESTDIR ${THIRD_PARTY_DIR}/protobuf)

ExternalProject_Add(
  protobuf-external
  PREFIX protobuf
  URL ${PROTOBUF_TAR_GZ}
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/protobuf
  CMAKE_CACHE_ARGS
    "-Dprotobuf_BUILD_TESTS:BOOL=OFF"
    "-Dprotobuf_BUILD_EXAMPLES:BOOL=OFF"
    "-Dprotobuf_WITH_ZLIB:BOOL=ON"
    "-DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}"
  SOURCE_SUBDIR cmake
  BUILD_ALWAYS 1
  STEP_TARGETS build
  INSTALL_DIR ${protobuf_DESTDIR}
  INSTALL_COMMAND cmake --install . --prefix ${protobuf_DESTDIR}
)
