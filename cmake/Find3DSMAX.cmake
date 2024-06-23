# - Find 3ds Max 2024 SDK installation
# Find the 3ds Max 2024 SDK includes and library
# This module defines
#  3DSMAX_INCLUDE_DIRS - Where to find 3DSMAX includes
#  3DSMAX_LIBRARIES   - List of libraries when using 3DSMAX
#  3DSMAX_FOUND       - True if 3DSMAX was found
#  3DSMAX_ROOT_DIR    - Directory where 3DSMAX was installed.

FIND_PATH(3DSMAX_INCLUDE_DIRS "max.h"
  PATHS
  "C:/Program Files/Autodesk/3ds Max 2024 SDK/maxsdk/include"
  $ENV{ADSK_3DSMAX_SDK_2024}/include
  $ENV{3DSMAXSDK}/include
)

get_filename_component(3DSMAX_ROOT_DIR "${3DSMAX_INCLUDE_DIRS}/.." ABSOLUTE)

set(3DSMAX_LIBRARY_PATHS "${3DSMAX_ROOT_DIR}/lib/x64/Release")

set(3DSMAX_LIBRARIES
    "${3DSMAX_LIBRARY_PATHS}/core.lib"
    "${3DSMAX_LIBRARY_PATHS}/geom.lib"
    "${3DSMAX_LIBRARY_PATHS}/mesh.lib"
    "${3DSMAX_LIBRARY_PATHS}/maxutil.lib"
    "${3DSMAX_LIBRARY_PATHS}/paramblk2.lib"
)

# handle the QUIETLY and REQUIRED arguments and set 3DSMAX_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(3DSMAX DEFAULT_MSG 3DSMAX_ROOT_DIR 3DSMAX_LIBRARIES 3DSMAX_INCLUDE_DIRS)