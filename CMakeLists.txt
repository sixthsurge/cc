cmake_minimum_required(VERSION 3.5)
project(cc)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# -----------------
#   file globbing
# -----------------

file(GLOB_RECURSE src src/*.c)

# ----------------------
#   target definitions
# ----------------------

add_executable(cc ${src})

target_compile_options(cc PUBLIC -Wall)

target_include_directories(cc PUBLIC include)

set_property(TARGET cc PROPERTY C_STANDARD 99)

