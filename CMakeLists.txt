cmake_minimum_required(VERSION 3.18)

project(MaxExp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project can only be built as a 64-bit application.")
endif()

set(CMAKE_CONFIGURATION_TYPES Release Debug)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(EG_Exp MODULE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(3DSMAX REQUIRED)
if(NOT 3DSMAX_FOUND)
    message(FATAL_ERROR "Could not find 3ds Max 2024 SDK")
endif()

set(MAXEXP_SOURCES
    src/export.cpp
    src/main.cpp
    src/ParCenter.cpp
    src/ParGroup.cpp
    src/ParMatrix.cpp
    src/ParObj.cpp
)
set(MAXEXP_HEADERS
    src/main.hpp
    src/maintok.hpp
    src/resource.hpp
    src/veroverrides.hpp
)

set(MAXEXP_RESOURCES
    src/main.def
    src/main.rc
)

set(MAXEXP_BASE_SOURCES
    src/Base/stdafx.cpp
    src/Base/CBlockPar.cpp
    src/Base/CBuf.cpp
    src/Base/CException.cpp
    src/Base/CFile.cpp
    src/Base/CHeap.cpp
    src/Base/CMain.cpp
    src/Base/Registry.cpp
    src/Base/Tracer.cpp
    src/Base/utils.cpp
)
set(MAXEXP_BASE_HEADERS
    src/Base/stdafx.h
    src/Base/Base.hpp
    src/Base/BaseDef.hpp
    src/Base/CBlockPar.hpp
    src/Base/CBuf.hpp
    src/Base/CException.hpp
    src/Base/CFile.hpp
    src/Base/CHeap.hpp
    src/Base/CMain.hpp
    src/Base/Registry.hpp
    src/Base/Tracer.hpp
    src/Base/utils.hpp
)

target_sources(EG_Exp PRIVATE
    ${MAXEXP_SOURCES}
    ${MAXEXP_HEADERS}
    ${MAXEXP_RESOURCES}
    ${MAXEXP_BASE_SOURCES}
    ${MAXEXP_BASE_HEADERS}
)

target_include_directories(EG_Exp PRIVATE src/Base)

source_group("Resources" FILES ${MAXEXP_RESOURCES})

source_group ("Base\\Source Files" FILES ${MAXEXP_BASE_SOURCES})
source_group ("Base\\Header Files" FILES ${MAXEXP_BASE_HEADERS})

if(WIN32)
    include_directories(AFTER ${3DSMAX_INCLUDE_DIRS})

    target_link_libraries(EG_Exp comctl32.lib)
    target_link_libraries(EG_Exp ${3DSMAX_LIBRARIES})

if(MSVC)
    set(MAXEXP_COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oy /GF /EHsc /MD /GS /Gy /Gd /D_UNICODE /DUNICODE>
        $<$<CONFIG:Debug>:/ZI /W3 /Od /Gm /RTC1 /MDd /Gd /D_UNICODE /DUNICODE>
    )
    set(MAXEXP_COMPILE_DEFINITIONS
        $<$<CONFIG:Release>:MAXEXP_EXPORTS _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN>
        $<$<CONFIG:Debug>:MAXEXP_EXPORTS WIN32_LEAN_AND_MEAN>
    )
    set(MAXEXP_LINK_OPTIONS
        $<$<CONFIG:Release>:/INCREMENTAL:NO /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /DYNAMICBASE:NO /MACHINE:X86>
        $<$<CONFIG:Debug>:/INCREMENTAL /DEBUG /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MACHINE:X86>
    )
endif(MSVC)

endif(WIN32)

target_compile_options(EG_Exp PRIVATE ${MAXEXP_COMPILE_OPTIONS})
target_compile_definitions(EG_Exp PRIVATE ${MAXEXP_COMPILE_DEFINITIONS})
target_link_options(EG_Exp PRIVATE ${MAXEXP_LINK_OPTIONS})

set(INSTALL_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/bin/x64")
set_target_properties(EG_Exp PROPERTIES SUFFIX .dle)

install(
    TARGETS EG_Exp
    RUNTIME DESTINATION ${INSTALL_TARGET}
    LIBRARY DESTINATION ${INSTALL_TARGET}
)
install(
    TARGETS EG_Exp
    CONFIGURATIONS Debug
    RUNTIME DESTINATION ${INSTALL_TARGET}/Debug
    LIBRARY DESTINATION ${INSTALL_TARGET}/Debug
)

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:EG_Exp>
        CONFIGURATIONS Debug
        DESTINATION ${INSTALL_TARGET}/Debug
    )
endif(MSVC)