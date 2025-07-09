cmake_minimum_required(VERSION 3.10)

#
# Config
#
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER       clang  )
set(CMAKE_CXX_COMPILER     clang++)
set(CMAKE_OBJCOPY     llvm-objcopy)
set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS       OFF)
set(TARGET "kernel")
project(${TARGET} LANGUAGES ASM C CXX)

#
# Source
#

set(SOURCES
    "src/arch/arm64/boot_el2.cc"
    "src/arch/arm64/mmu.cc"
    "src/common/cstring/memcpy.cc"
    "src/kernel/kernel_main.cc"
    "src/kernel/util.cc"
)

set(ASM_SOURCES
    "src/arch/arm64/boot.S"
)

add_executable(${TARGET}
    ${SOURCES}
    ${ASM_SOURCES}
)
target_include_directories(${TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/arch/
)


#
# Compiler flag
#

set(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS} -DNDEBUG")
set(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -DDEBUG")
set(COMPILE_FLAGS "-Wall -nostdlib -nodefaultlibs -fno-builtin -ffreestanding -mstrict-align")
# Fix an issue using std::function: undefined reference to `__dso_handle'
set(COMPILE_FLAGS "${COMPILE_FLAGS} -fno-use-cxa-atexit")
# Fix undefined reference to `__cxa_guard_acquire'
set(COMPILE_FLAGS "${COMPILE_FLAGS} -fno-threadsafe-statics")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
endif()
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} ${COMPILE_FLAGS} -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${COMPILE_FLAGS} -DDEBUG")
set(CXX_COMMON_FLAGS "-fno-exceptions -fno-unwind-tables -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CXX_COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CXX_COMMON_FLAGS}")

#
# Linker flags
#

# Disable creation of ".note.gnu.build-id" section"
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--build-id=none -Wl,--gc-sections")

# Linker script
set(LINKER_SCRIPT "src/arch/arm64/scripts/qemu/linker.ld")
set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
target_link_options(${TARGET} PRIVATE "-T${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT}")
target_link_options(${TARGET} PRIVATE "-static")

#
# Custom build
#

add_custom_command(
   TARGET ${TARGET}
   POST_BUILD
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
   COMMAND mv ${TARGET} ${TARGET}.elf
)

add_custom_command(
   TARGET ${TARGET}
   POST_BUILD
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
   COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET}.elf ${TARGET}.bin
)