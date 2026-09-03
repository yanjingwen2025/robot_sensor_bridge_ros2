cmake_minimum_required(VERSION 3.8)
project(sensor_gateway)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/GetSensorStats.srv"
)

# 添加可执行文件
add_executable(sensor_gateway_node src/sensor_gateway_node.cpp)
ament_target_dependencies(sensor_gateway_node rclcpp std_msgs sensor_msgs)

target_link_libraries(sensor_gateway_node 
  ${PROJECT_NAME}__rosidl_typesupport_cpp
)

add_executable(data_processor_node src/data_processor_node.cpp)
ament_target_dependencies(data_processor_node 
  rclcpp 
  std_msgs
)

add_executable(service_client_node src/service_client_node.cpp)
ament_target_dependencies(service_client_node 
  rclcpp
)
target_link_libraries(service_client_node 
  ${PROJECT_NAME}__rosidl_typesupport_cpp
)

# 安装可执行文件到 lib 目录
install(TARGETS
  sensor_gateway_node
  data_processor_node
  service_client_node
  DESTINATION lib/${PROJECT_NAME}
)


# 安装服务接口
install(DIRECTORY srv/
  DESTINATION share/${PROJECT_NAME}/srv
)

ament_package()
