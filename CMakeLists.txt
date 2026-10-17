cmake_minimum_required(VERSION 3.22.1)

project("securelock_native")

# Hardened compilation flags:
# - C++17 standard
# - Stack canary protection (-fstack-protector-strong)
# - Hidden symbol visibility to hinder reverse engineering (-fvisibility=hidden)
# - Compiler optimization flag: -O3
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -fstack-protector-strong -fvisibility=hidden -D_FORTIFY_SOURCE=2")

add_library(
    securelock_native
    SHARED
    native-lib.cpp
    crypto_vault.cpp
)

find_library(
    log-lib
    log
)

find_library(
    android-lib
    android
)

target_link_libraries(
    securelock_native
    ${log-lib}
    ${android-lib}
)
