cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(kndp)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DRELEASE)

add_executable(kndp main.c)

target_link_libraries(kndp
  taihenForKernel_stub
  SceLibKernel_stub
  SceSysmemForDriver_stub
  SceThreadmgrForDriver_stub
  SceIofilemgrForDriver_stub
  k
  gcc
)

set_target_properties(kndp
  PROPERTIES LINK_FLAGS "-nostdlib"
  COMPILE_FLAGS "-D__VITA_KERNEL__"
)

add_custom_target(kndp.skprx ALL
  COMMAND vita-elf-create -e ${CMAKE_SOURCE_DIR}/kndp.yml kndp kndp.velf ${CMAKE_SOURCE_DIR}/taihen.json
  COMMAND vita-make-fself -c kndp.velf kndp.skprx
)

add_dependencies(kndp.skprx kndp)

vita_create_stubs(stubs kndp ${CMAKE_SOURCE_DIR}/kndp.yml KERNEL)

install(FILES ${CMAKE_BINARY_DIR}/stubs/libkndp_stub.a DESTINATION lib)
install(FILES ${CMAKE_BINARY_DIR}/stubs/libkndp_stub_weak.a DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/kndp.h DESTINATION include)
