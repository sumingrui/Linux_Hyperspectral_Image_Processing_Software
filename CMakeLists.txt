cmake_minimum_required(VERSION 3.0.0)

project(tfclient)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ${PROJECT_SOURCE_DIR}/db
)

set(SOURCE share.cc bus.cc main.cc db/DBtools.cc db/hyperspectralDB.cc)

set(LIBS -pthread -lconfig++ -lmysqlclient -ljsoncpp -lcurl -ljsonrpccpp-common -ljsonrpccpp-client)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}) 

add_executable(tfclient ${SOURCE})

target_link_libraries(tfclient ${LIBS})