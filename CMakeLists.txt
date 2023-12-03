cmake_minimum_required(VERSION 3.10)

project(GTKExample C)

# Find GTK3 package
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Add the include directories for GTK3
include_directories(${GTK3_INCLUDE_DIRS})

# Link against GTK3 libraries
link_directories(${GTK3_LIBRARY_DIRS})
link_libraries(${GTK3_LIBRARIES})

# Set compiler flags
add_definitions(${GTK3_CFLAGS_OTHER})

# Define the executable
add_executable(gtk_example system_monitor.c)

# Link the executable against GTK3 libraries
target_link_libraries(gtk_example ${GTK3_LIBRARIES})
