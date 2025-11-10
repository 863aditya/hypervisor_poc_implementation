cmake_minimum_required(VERSION 3.10)
project(MyDbProject VERSION 1.0)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PROJECT_SOURCES
    main.cpp
    db/db.cpp
    mini_ssh/mini_ssh.cpp
    mini_ssh/shell.c      # shell.c is now added as a separate C source file
    mini_ssh/server.cpp
    hypervisor_main/hypervisor.cpp
    instance_main/instance.cpp
)

# --- 1. Find the libpqxx header files and library ---
# Note: This is less robust than find_package, but is a common workaround.
find_path(PQXX_INCLUDE_DIR pqxx/pqxx HINTS /usr/local/include /usr/include)
find_library(PQXX_LIBRARY pqxx HINTS /usr/local/lib /usr/lib)

# --- 2. Link your executable ---
# add_executable(my_program main.cpp db/db.cpp)
add_executable(my_program ${PROJECT_SOURCES})

# Link using the manually found variables and also link the underlying libpq
target_link_libraries(my_program PRIVATE ${PQXX_LIBRARY} pq)

# Add the include path
target_include_directories(my_program PRIVATE ${PQXX_INCLUDE_DIR} db)