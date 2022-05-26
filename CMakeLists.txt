# CMakeList.txt: CMake-Projekt für "libtasmota". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project ("tasmota")

set(COMMON_SOURCES
    src/TasmotaAPI.cpp
    src/HttpClient.cpp
    src/Json.cpp
    src/Logger.cpp
    src/Url.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${COMMON_SOURCES} 
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    include
)


set(TEST_SOURCES
    src/Test.cpp
)
set(TEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${TEST_INCLUDE_DIR})
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES} )
add_dependencies(${PROJECT_NAME}_test ${PROJECT_NAME})

if (MSVC)
target_link_libraries(${PROJECT_NAME}_test tasmota.lib ws2_32.lib Iphlpapi.lib)
else()
target_link_libraries(${PROJECT_NAME}_test libtasmota.a)
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})