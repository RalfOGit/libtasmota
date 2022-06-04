# CMakeList.txt: CMake-Projekt für "libtasmota". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project ("tasmota")

#
# Target:  ${PROJECT_NAME}  =>  create tasmota.lib or libtasmota.a
#
set(COMMON_SOURCES
    src/TasmotaAPI.cpp
    src/HttpClient.cpp
    src/Json.cpp
    src/Logger.cpp
    src/Url.cpp
)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} STATIC
    ${COMMON_SOURCES} 
)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})

#
# Target:  ${PROJECT_NAME}_test  =>  create tasmota_test.exe
#
set(TEST_SOURCES
    src/Test.cpp
)
set(TEST_INCLUDE_DIR ${INCLUDE_DIR})

add_executable(${PROJECT_NAME}_test ${TEST_SOURCES} )
add_dependencies(${PROJECT_NAME}_test ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}_test PUBLIC ${TEST_INCLUDE_DIR})

if (MSVC)
target_link_libraries(${PROJECT_NAME}_test tasmota.lib ws2_32.lib Iphlpapi.lib)
else()
target_link_libraries(${PROJECT_NAME}_test ${CMAKE_SOURCE_DIR}/build/libtasmota.a)
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
    OUTPUT_NAME ${PROJECT_NAME}_test
    ARCHIVE_OUTPUT_NAME ${PROJECT_NAME}
)
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_test RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
