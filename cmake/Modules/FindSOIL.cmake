# - Try to find soil
# Once done this will define
#  SOIL_FOUND - System has soil
#  SOIL_INCLUDE_DIRS - The soil include directories
#  SOIL_LIBRARIES - The libraries needed for soil
#  SOIL_DEFINITIONS - Compiler switches required for using soil

find_package(PkgConfig)
pkg_check_modules(PC_SOIL QUIET soil)
set(SOIL_DEFINITIONS ${PC_SOIL_CFLAGS_OTHER})

find_path(SOIL_INCLUDE_DIR SOIL/SOIL.h
    HINTS ${PC_SOIL_INCLUDEDIR} ${PC_SOIL_INCLUDE_DIRS})

find_library(SOIL_LIBRARY SOIL
        HINTS ${PC_SOIL_LIBDIR} ${PC_SOIL_LIBRARY_DIRS})

set(SOIL_INCLUDE_DIRS ${SOIL_INCLUDE_DIR})
set(SOIL_LIBRARIES ${SOIL_LIBRARY})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SOIL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SOIL DEFAULT_MSG
    SOIL_INCLUDE_DIR SOIL_LIBRARY)

mark_as_advanced(SOIL_INCLUDE_DIR SOIL_LIBRARY)

