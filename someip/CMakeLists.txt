

add_executable(someip_publisher socket_pub.cpp)
add_executable(someip_subscriber socket_sub.cpp)

target_link_libraries(someip_publisher  ${Boost_LIBRARIES} ${VSOMEIP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(someip_subscriber  ${Boost_LIBRARIES} ${VSOMEIP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})