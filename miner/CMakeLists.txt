cmake_minimum_required(VERSION 3.15)

project(ezminer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB MINER_HEADERS
        *.h
)

file(GLOB MINER_SOURSES
        *.cpp
)
#set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME} ${MINER_HEADERS} ${MINER_SOURSES})
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto)
