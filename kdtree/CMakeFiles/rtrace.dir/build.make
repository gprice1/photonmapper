# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

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
CMAKE_COMMAND = /opt/local/bin/cmake

# The command to remove a file.
RM = /opt/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /opt/local/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/templeprice/Desktop/programming/cs/photonmapper

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/templeprice/Desktop/programming/cs/photonmapper/kdtree

# Include any dependencies generated for this target.
include CMakeFiles/rtrace.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rtrace.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rtrace.dir/flags.make

CMakeFiles/rtrace.dir/raytracer.cpp.o: CMakeFiles/rtrace.dir/flags.make
CMakeFiles/rtrace.dir/raytracer.cpp.o: ../raytracer.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/templeprice/Desktop/programming/cs/photonmapper/kdtree/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/rtrace.dir/raytracer.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/rtrace.dir/raytracer.cpp.o -c /Users/templeprice/Desktop/programming/cs/photonmapper/raytracer.cpp

CMakeFiles/rtrace.dir/raytracer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rtrace.dir/raytracer.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/templeprice/Desktop/programming/cs/photonmapper/raytracer.cpp > CMakeFiles/rtrace.dir/raytracer.cpp.i

CMakeFiles/rtrace.dir/raytracer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rtrace.dir/raytracer.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/templeprice/Desktop/programming/cs/photonmapper/raytracer.cpp -o CMakeFiles/rtrace.dir/raytracer.cpp.s

CMakeFiles/rtrace.dir/raytracer.cpp.o.requires:
.PHONY : CMakeFiles/rtrace.dir/raytracer.cpp.o.requires

CMakeFiles/rtrace.dir/raytracer.cpp.o.provides: CMakeFiles/rtrace.dir/raytracer.cpp.o.requires
	$(MAKE) -f CMakeFiles/rtrace.dir/build.make CMakeFiles/rtrace.dir/raytracer.cpp.o.provides.build
.PHONY : CMakeFiles/rtrace.dir/raytracer.cpp.o.provides

CMakeFiles/rtrace.dir/raytracer.cpp.o.provides.build: CMakeFiles/rtrace.dir/raytracer.cpp.o

CMakeFiles/rtrace.dir/photonmapper.cpp.o: CMakeFiles/rtrace.dir/flags.make
CMakeFiles/rtrace.dir/photonmapper.cpp.o: ../photonmapper.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/templeprice/Desktop/programming/cs/photonmapper/kdtree/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/rtrace.dir/photonmapper.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/rtrace.dir/photonmapper.cpp.o -c /Users/templeprice/Desktop/programming/cs/photonmapper/photonmapper.cpp

CMakeFiles/rtrace.dir/photonmapper.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rtrace.dir/photonmapper.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/templeprice/Desktop/programming/cs/photonmapper/photonmapper.cpp > CMakeFiles/rtrace.dir/photonmapper.cpp.i

CMakeFiles/rtrace.dir/photonmapper.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rtrace.dir/photonmapper.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/templeprice/Desktop/programming/cs/photonmapper/photonmapper.cpp -o CMakeFiles/rtrace.dir/photonmapper.cpp.s

CMakeFiles/rtrace.dir/photonmapper.cpp.o.requires:
.PHONY : CMakeFiles/rtrace.dir/photonmapper.cpp.o.requires

CMakeFiles/rtrace.dir/photonmapper.cpp.o.provides: CMakeFiles/rtrace.dir/photonmapper.cpp.o.requires
	$(MAKE) -f CMakeFiles/rtrace.dir/build.make CMakeFiles/rtrace.dir/photonmapper.cpp.o.provides.build
.PHONY : CMakeFiles/rtrace.dir/photonmapper.cpp.o.provides

CMakeFiles/rtrace.dir/photonmapper.cpp.o.provides.build: CMakeFiles/rtrace.dir/photonmapper.cpp.o

# Object files for target rtrace
rtrace_OBJECTS = \
"CMakeFiles/rtrace.dir/raytracer.cpp.o" \
"CMakeFiles/rtrace.dir/photonmapper.cpp.o"

# External object files for target rtrace
rtrace_EXTERNAL_OBJECTS =

librtrace.a: CMakeFiles/rtrace.dir/raytracer.cpp.o
librtrace.a: CMakeFiles/rtrace.dir/photonmapper.cpp.o
librtrace.a: CMakeFiles/rtrace.dir/build.make
librtrace.a: CMakeFiles/rtrace.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library librtrace.a"
	$(CMAKE_COMMAND) -P CMakeFiles/rtrace.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rtrace.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rtrace.dir/build: librtrace.a
.PHONY : CMakeFiles/rtrace.dir/build

CMakeFiles/rtrace.dir/requires: CMakeFiles/rtrace.dir/raytracer.cpp.o.requires
CMakeFiles/rtrace.dir/requires: CMakeFiles/rtrace.dir/photonmapper.cpp.o.requires
.PHONY : CMakeFiles/rtrace.dir/requires

CMakeFiles/rtrace.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rtrace.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rtrace.dir/clean

CMakeFiles/rtrace.dir/depend:
	cd /Users/templeprice/Desktop/programming/cs/photonmapper/kdtree && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/templeprice/Desktop/programming/cs/photonmapper /Users/templeprice/Desktop/programming/cs/photonmapper /Users/templeprice/Desktop/programming/cs/photonmapper/kdtree /Users/templeprice/Desktop/programming/cs/photonmapper/kdtree /Users/templeprice/Desktop/programming/cs/photonmapper/kdtree/CMakeFiles/rtrace.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rtrace.dir/depend

