

add_executable(socket_publisher socket_pub.cpp)
add_executable(socket_subscriber socket_sub.cpp)

target_link_libraries(socket_publisher  ${Boost_LIBRARIES} ${VSOMEIP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(socket_subscriber  ${Boost_LIBRARIES} ${VSOMEIP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})