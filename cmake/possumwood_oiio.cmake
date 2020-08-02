# using pkg-config
find_package(PkgConfig REQUIRED)

# try to find OIIO using pkgconfig - works on newer versions of Ubuntu/Debian
pkg_check_modules(OPENIMAGEIO OpenImageIO)

# not found using pkgconfig - lets try using find_path (older Ubuntu didn't have oiio pkg config setup)
if(NOT OPENIMAGEIO_FOUND)
    find_path(OPENIMAGEIO_INCLUDE_DIRS NAMES OpenImageIO/imageio.h REQUIRED)
    find_library(OPENIMAGEIO_LIBRARIES NAMES libOpenImageIO.so REQUIRED)
endif()

include_directories(${OPENIMAGEIO_INCLUDE_DIRS}) # make sure OpenImageIO/ include prefix is maintained
set(LIBS ${LIBS} ${OPENIMAGEIO_LIBRARIES})
