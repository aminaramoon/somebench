# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/amin/Applications/cmake/3.17.3/bin/cmake

# The command to remove a file.
RM = /home/amin/Applications/cmake/3.17.3/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/amin/Workspace/local/someip_benchmark

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/amin/Workspace/local/someip_benchmark/build

# Include any dependencies generated for this target.
include socket/CMakeFiles/socket_publisher.dir/depend.make

# Include the progress variables for this target.
include socket/CMakeFiles/socket_publisher.dir/progress.make

# Include the compile flags for this target's objects.
include socket/CMakeFiles/socket_publisher.dir/flags.make

socket/CMakeFiles/socket_publisher.dir/socket_pub.cpp.o: socket/CMakeFiles/socket_publisher.dir/flags.make
socket/CMakeFiles/socket_publisher.dir/socket_pub.cpp.o: ../socket/socket_pub.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/amin/Workspace/local/someip_benchmark/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object socket/CMakeFiles/socket_publisher.dir/socket_pub.cpp.o"
	cd /home/amin/Workspace/local/someip_benchmark/build/socket && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/socket_publisher.dir/socket_pub.cpp.o -c /home/amin/Workspace/local/someip_benchmark/socket/socket_pub.cpp

socket/CMakeFiles/socket_publisher.dir/socket_pub.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/socket_publisher.dir/socket_pub.cpp.i"
	cd /home/amin/Workspace/local/someip_benchmark/build/socket && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/amin/Workspace/local/someip_benchmark/socket/socket_pub.cpp > CMakeFiles/socket_publisher.dir/socket_pub.cpp.i

socket/CMakeFiles/socket_publisher.dir/socket_pub.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/socket_publisher.dir/socket_pub.cpp.s"
	cd /home/amin/Workspace/local/someip_benchmark/build/socket && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/amin/Workspace/local/someip_benchmark/socket/socket_pub.cpp -o CMakeFiles/socket_publisher.dir/socket_pub.cpp.s

# Object files for target socket_publisher
socket_publisher_OBJECTS = \
"CMakeFiles/socket_publisher.dir/socket_pub.cpp.o"

# External object files for target socket_publisher
socket_publisher_EXTERNAL_OBJECTS =

socket/socket_publisher: socket/CMakeFiles/socket_publisher.dir/socket_pub.cpp.o
socket/socket_publisher: socket/CMakeFiles/socket_publisher.dir/build.make
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_log.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_system.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_date_time.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_log_setup.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_filesystem.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_thread.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_regex.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_chrono.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_atomic.so
socket/socket_publisher: /home/amin/Applications/vsomeip/3.1.7/lib/libvsomeip3.so.3.1.7
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_regex.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_chrono.so
socket/socket_publisher: /home/amin/Applications/boost/1.65.0/lib/libboost_atomic.so
socket/socket_publisher: socket/CMakeFiles/socket_publisher.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/amin/Workspace/local/someip_benchmark/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable socket_publisher"
	cd /home/amin/Workspace/local/someip_benchmark/build/socket && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/socket_publisher.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
socket/CMakeFiles/socket_publisher.dir/build: socket/socket_publisher

.PHONY : socket/CMakeFiles/socket_publisher.dir/build

socket/CMakeFiles/socket_publisher.dir/clean:
	cd /home/amin/Workspace/local/someip_benchmark/build/socket && $(CMAKE_COMMAND) -P CMakeFiles/socket_publisher.dir/cmake_clean.cmake
.PHONY : socket/CMakeFiles/socket_publisher.dir/clean

socket/CMakeFiles/socket_publisher.dir/depend:
	cd /home/amin/Workspace/local/someip_benchmark/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/amin/Workspace/local/someip_benchmark /home/amin/Workspace/local/someip_benchmark/socket /home/amin/Workspace/local/someip_benchmark/build /home/amin/Workspace/local/someip_benchmark/build/socket /home/amin/Workspace/local/someip_benchmark/build/socket/CMakeFiles/socket_publisher.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : socket/CMakeFiles/socket_publisher.dir/depend

