cmake_minimum_required(VERSION 3.8)
project(fake-jni VERSION 0.1.0)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)

if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    message(FATAL_ERROR "You are using an unsupported compiler, please build with clang.")
endif()

add_subdirectory(CX)

set(BUILD_FAKE_JNI_DEBUG OFF CACHE BOOL "Enables fake-jni debug building")
if(${BUILD_FAKE_JNI_DEBUG})
 set(CMAKE_BUILD_TYPE Debug)
 set(compiler_flags "\
  -g\
  -fstandalone-debug\
  -O0\
  -fno-inline\
  -fno-omit-frame-pointer\
  -DFAKE_JNI_DEBUG\
 ")
else()
 set(CMAKE_BUILD_TYPE Release)
 set(compiler_flags "-O0")
endif()

if(${BUILD_FAKE_JNI_ASAN})
 string(APPEND compiler_flags "-fsanitize=address")
endif()

if(${BUILD_FAKE_JNI_UBSAN})
 string(APPEND compiler_flags " -fsanitize=undefined")
endif()

string(APPEND CMAKE_CXX_FLAGS " ${compiler_flags}")
string(APPEND CMAKE_C_FLAGS " ${compiler_flags}")

set(AUTOMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(AUTOMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
if(CMAKE_OSX_SYSROOT)
 set(AUTOMAKE_CXX_FLAGS "${AUTOMAKE_CXX_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT}")
 set(AUTOMAKE_C_FLAGS "${AUTOMAKE_C_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT}")
endif()
if(CMAKE_CXX_COMPILER_TARGET)
 set(AUTOMAKE_CXX_FLAGS "${AUTOMAKE_CXX_FLAGS} --target=${CMAKE_CXX_COMPILER_TARGET}")
 set(AUTOMAKE_C_FLAGS "${AUTOMAKE_C_FLAGS} --target=${CMAKE_CXX_COMPILER_TARGET}")
endif()

#libffi
if(NOT DEFINED FFI_CC)
 set(FFI_CC ${CMAKE_C_COMPILER})
endif()

if(NOT DEFINED FFI_CXX)
 set(FFI_CXX ${CMAKE_CXX_COMPILER})
endif()

if(NOT DEFINED FFI_CONFIGURE_FLAGS)
 set(FFI_CONFIGURE_FLAGS "")
endif()

message("configure ${FFI_CONFIGURE_FLAGS} --disable-shared --disable-builddir --prefix=<INSTALL_DIR> --host=${CMAKE_CXX_COMPILER_TARGET} CC=${FFI_CC} CFLAGS=${AUTOMAKE_C_FLAGS} CXX=${FFI_CXX} CXXFLAGS=${AUTOMAKE_CXX_FLAGS}")
ExternalProject_Add(ffi
 SOURCE_DIR ${PROJECT_SOURCE_DIR}/libffi
 UPDATE_COMMAND ""
 CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh && <SOURCE_DIR>/configure ${FFI_CONFIGURE_FLAGS} --disable-shared --disable-builddir --prefix=<INSTALL_DIR> --host=${CMAKE_CXX_COMPILER_TARGET} CC=${FFI_CC} CFLAGS=${AUTOMAKE_C_FLAGS} CXX=${FFI_CXX} CXXFLAGS=${AUTOMAKE_CXX_FLAGS}
 BUILD_IN_SOURCE 1
 BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libffi.a
)

ExternalProject_Get_Property(ffi INSTALL_DIR)
set(LIBFFI_INSTALL_DIR ${INSTALL_DIR}/lib/libffi-3.2.1/)
ExternalProject_Get_Property(ffi INSTALL_DIR)
set(ffi_INSTALL_DIR ${INSTALL_DIR})

add_library(libffi STATIC IMPORTED GLOBAL)
file(MAKE_DIRECTORY ${LIBFFI_INSTALL_DIR}/include)
set_target_properties(libffi PROPERTIES
 IMPORTED_LOCATION ${INSTALL_DIR}/lib/libffi.a
 INTERFACE_INCLUDE_DIRECTORIES ${LIBFFI_INSTALL_DIR}/include
)

#clean up libffi's non-compliant build artifacts
ExternalProject_Get_Property(ffi SOURCE_DIR)
add_custom_command(
 TARGET ffi PRE_BUILD
 COMMAND git clean -x -d -f
 COMMAND git reset --hard
 WORKING_DIRECTORY ${SOURCE_DIR}
 COMMENT "Cleaning libffi directory..."
)

#libunwind
set(BUILD_FAKE_JNI_WITH_LIBUNWIND OFF CACHE BOOL "Enables integration with libunwind")

if (BUILD_FAKE_JNI_WITH_LIBUNWIND)
 if(NOT DEFINED UNWIND_CC)
  set(UNWIND_CC ${CMAKE_C_COMPILER})
 endif()

 if(NOT DEFINED UNWIND_CXX)
  set(UNWIND_CXX ${CMAKE_CXX_COMPILER})
 endif()

 if(NOT DEFINED UNWIND_CONFIGURE_FLAGS)
  set(UNWIND_CONFIGURE_FLAGS "--disable-coredump")
 endif()

 ExternalProject_Add(unwind
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/libunwind
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh && <SOURCE_DIR>/configure ${UNWIND_CONFIGURE_FLAGS} --disable-minidebuginfo --disable-shared --prefix=<INSTALL_DIR> --host=${CMAKE_CXX_COMPILER_TARGET} CC=${UNWIND_CC} CFLAGS=${AUTOMAKE_C_FLAGS} CXX=${UNWIND_CXX} CXXFLAGS=${AUTOMAKE_CXX_FLAGS}
  BUILD_IN_SOURCE 1
  BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libunwind.a
 )
 ExternalProject_Get_Property(unwind INSTALL_DIR)
 set(LIBUNWIND_INSTALL_DIR ${INSTALL_DIR}/lib/)
 ExternalProject_Get_Property(unwind INSTALL_DIR)
 set(unwind_INSTALL_DIR ${INSTALL_DIR})
 add_library(libunwind STATIC IMPORTED GLOBAL)
 file(MAKE_DIRECTORY ${LIBUNWIND_INSTALL_DIR}/include)
 set_target_properties(libunwind PROPERTIES
  IMPORTED_LOCATION ${INSTALL_DIR}/lib/libunwind.a
  INTERFACE_INCLUDE_DIRECTORIES ${LIBUNWIND_INSTALL_DIR}/include
 )

 #clean up libunwind's non-compliant build artifacts
 ExternalProject_Get_Property(unwind SOURCE_DIR)
 add_custom_command(
  TARGET unwind PRE_BUILD
  COMMAND git clean -x -d -f
  COMMAND git reset --hard
  WORKING_DIRECTORY ${SOURCE_DIR}
  COMMENT "Cleaning unwind directory..."
 )
endif()

#fake-jni
file(GLOB_RECURSE FAKE_JNI_SRC "src/**.cpp")
add_library(fake-jni STATIC ${FAKE_JNI_SRC})

target_link_libraries(fake-jni dl libffi)
add_dependencies(fake-jni ffi)
target_include_directories(fake-jni PUBLIC
 include
 ${cx_SOURCE_DIR}
 ${ffi_INSTALL_DIR}/include
)
target_compile_options(fake-jni PUBLIC
 -Wall
 -Wextra
# -Werror
 -pedantic
 -Wno-unused-parameter
 -Wno-unused-function
 -Wno-gnu-anonymous-struct
 -Wno-nested-anon-types
 -Wno-vla-extension
 -Wno-zero-length-array
)

if (BUILD_FAKE_JNI_WITH_LIBUNWIND)
 target_link_libraries(fake-jni libunwind)
 add_dependencies(fake-jni unwind)
 target_include_directories(fake-jni PUBLIC ${unwind_INSTALL_DIR}/include)
 target_compile_definitions(fake-jni PRIVATE BUILD_FAKE_JNI_WITH_LIBUNWIND)
endif()

set(BUILD_FAKE_JNI_TESTS ON CACHE BOOL "Enables fake-jni tests")
set(BUILD_FAKE_JNI_EXAMPLES ON CACHE BOOL "Enables fake-jni examples")

if (${BUILD_FAKE_JNI_TESTS})
 add_subdirectory(tests)
endif()

if (${BUILD_FAKE_JNI_EXAMPLES})
 add_subdirectory(examples)
endif()