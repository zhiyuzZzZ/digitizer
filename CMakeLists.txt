cmake_minimum_required(VERSION 3.10)
project(Digitization)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找ROOT包
find_package(ROOT REQUIRED COMPONENTS Core RIO Hist Tree Graf Graf3d Gpad)
include(${ROOT_USE_FILE})

# 设置包含目录
include_directories(include)

# 设置源文件
set(SOURCES
    src/DetectorParameters.cpp
    src/DigitizationBase.cpp
    src/ScintillationDigitizer.cpp
    src/SiPMDigitizer.cpp
    src/ADCDigitizer.cpp
    src/TotalDigitizer.cpp
    src/DigitizationManager.cpp
)

# 创建库
add_library(digitization_lib SHARED ${SOURCES})
target_link_libraries(digitization_lib ${ROOT_LIBRARIES})

# 创建可执行文件
add_executable(digitize main.cpp)
target_link_libraries(digitize digitization_lib)

# 安装规则
install(TARGETS digitize DESTINATION bin)
install(TARGETS digitization_lib DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

# 添加测试
#enable_testing()
#add_subdirectory(tests)

# 拷贝脚本到构建目录
#file(GLOB SCRIPTS "scripts/*.sh" "scripts/*.C")
#file(COPY ${SCRIPTS} DESTINATION ${CMAKE_BINARY_DIR}/scripts)

# 输出配置信息
message(STATUS "ROOT version: ${ROOT_VERSION}")
message(STATUS "ROOT include dir: ${ROOT_INCLUDE_DIRS}")
message(STATUS "ROOT libraries: ${ROOT_LIBRARIES}") 