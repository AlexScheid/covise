# - Find JSBSim
# Find the JSBSim includes and library
#
#  JSBSIM_INCLUDE_DIR - Where to find JSBSim includes
#  JSBSIM_LIBRARIES   - List of libraries when using JSBSim
#  JSBSIM_FOUND       - True if JSBSim was found

IF(JSBSIM_INCLUDE_DIR)
  SET(JSBSIM_FIND_QUIETLY TRUE)
ENDIF(JSBSIM_INCLUDE_DIR)

FIND_PATH(JSBSIM_INCLUDE_DIR "FGFDMExec.h"
  PATHS
  $ENV{JSBSIM_HOME}/include
  ~/Library/Frameworks/include
  /Library/Frameworks/include
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
  DOC "JSBSIM - Headers"
)

SET(JSBSIM_NAMES jsbsim)
SET(JSBSIM_DBG_NAMES jsbsimd)

FIND_LIBRARY(JSBSIM_LIBRARY NAMES ${JSBSIM_NAMES}
  PATHS
  $ENV{JSBSIM_HOME}
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  PATH_SUFFIXES lib lib64
  DOC "JSBSIM - Library"
)

INCLUDE(FindPackageHandleStandardArgs)

IF(MSVC)
  # VisualStudio needs a debug version
  FIND_LIBRARY(JSBSIM_LIBRARY_DEBUG NAMES ${JSBSIM_DBG_NAMES}
    PATHS
    $ENV{JSBSIM_HOME}/lib
    DOC "JSBSIM - Library (Debug)"
  )
  
  IF(JSBSIM_LIBRARY_DEBUG AND JSBSIM_LIBRARY)
    SET(JSBSIM_LIBRARIES optimized ${JSBSIM_LIBRARY} debug ${JSBSIM_LIBRARY_DEBUG})
  ENDIF(JSBSIM_LIBRARY_DEBUG AND JSBSIM_LIBRARY)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(JSBSIM DEFAULT_MSG JSBSIM_LIBRARY JSBSIM_LIBRARY_DEBUG JSBSIM_INCLUDE_DIR)

  MARK_AS_ADVANCED(JSBSIM_LIBRARY JSBSIM_LIBRARY_DEBUG JSBSIM_INCLUDE_DIR)
  
ELSE(MSVC)
  # rest of the world
  SET(JSBSIM_LIBRARIES ${JSBSIM_LIBRARY})

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(JSBSIM DEFAULT_MSG JSBSIM_LIBRARY JSBSIM_INCLUDE_DIR)
  
  MARK_AS_ADVANCED(JSBSIM_LIBRARY JSBSIM_INCLUDE_DIR)
  
ENDIF(MSVC)

IF(JSBSIM_FOUND)
  SET(JSBSIM_INCLUDE_DIRS ${JSBSIM_INCLUDE_DIR})
ENDIF(JSBSIM_FOUND)
