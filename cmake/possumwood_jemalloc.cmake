# using pkg-config
find_package(PkgConfig REQUIRED)

# try to find jemalloc using pkgconfig - works on newer versions of Ubuntu/Debian
pkg_check_modules(JEMALLOC jemalloc)

# not found using pkgconfig - lets try using find_path (older Ubuntu didn't have jemalloc pkg config setup)
if(NOT JEMALLOC_FOUND)
    find_library(JEMALLOC_LIBRARIES NAMES libjemalloc.so REQUIRED)
endif()

set(LIBS ${LIBS} ${JEMALLOC_LIBRARIES})