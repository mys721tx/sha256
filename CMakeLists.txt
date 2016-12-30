cmake_minimum_required(VERSION 2.6)

project (sha256)

set (sha256_VERSION 0.1)
set (sha256_BUGREPORT "mys721tx\@gmail.com")
set ( CMAKE_C_FLAGS "-Wall")

add_executable (sha256 sha256.c)

install(TARGETS sha256 DESTINATION bin)
