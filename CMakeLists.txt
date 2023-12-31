cmake_minimum_required(VERSION 3.5) # CMake install : https://cmake.org/download/
project(bluetooth LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "c:/Qt/Qt5.14.2/5.14.2/msvc2017_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt5 COMPONENTS Widgets  Bluetooth REQUIRED) # Qt COMPONENTS
# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Bluetooth) #寻找Bluetooth模块
# find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Bluetooth) #寻找Bluetooth模块

aux_source_directory(./src srcs)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(${PROJECT_NAME}
    # WIN32 # If you need a terminal for debug, please comment this statement 
    ${srcs} 
) 
# target_link_libraries(bluetooth_serial_host_computer PRIVATE
#     Qt${QT_VERSION_MAJOR}::Core
#     Qt${QT_VERSION_MAJOR}::Widgets
#     Qt${QT_VERSION_MAJOR}::Bluetooth #添加蓝牙
#     )

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt5::Widgets
    Qt5::Bluetooth # 添加蓝牙
) # Qt5 Shared Library