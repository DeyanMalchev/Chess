cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(main  "src/main.cpp" "src/AI.cpp" "src/Board.cpp" "src/Game.cpp"  "src/MoveValidator.cpp" "src/Piece.cpp" "src/Pieces.cpp" "src/TextureManager.cpp" "src/UI.cpp")

target_include_directories(main PRIVATE src) 
target_compile_features(main PRIVATE cxx_std_17)
find_package(Threads REQUIRED)
target_link_libraries(main PRIVATE SFML::Graphics Threads::Threads)

# Automatically copy assets folder to the build output directory
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/bin/assets
    $<TARGET_FILE_DIR:main>/assets
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)