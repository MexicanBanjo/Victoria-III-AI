cmake_minimum_required(VERSION 3.10)
project(vic3_economic_ai_sim C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -g)

include_directories(src)

# Simulation + AI binary
add_executable(vic3_sim
    src/sim/main.c
    src/sim/world.c
    src/sim/building.c
    src/sim/pop.c
    src/ai/ai_engine.c
)

# If you later add tests, you can add them here as more executables.
