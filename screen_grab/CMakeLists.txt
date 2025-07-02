cmake_minimum_required(VERSION 2.8.3)
project(screen_grab)

## Catkin and system dependencies
find_package(catkin REQUIRED COMPONENTS
  dynamic_reconfigure
  message_generation
  sensor_msgs
  std_msgs
  rospy
  roscpp
  nodelet
)

find_package(OpenCV REQUIRED COMPONENTS highgui)

## Message/service definitions
add_service_files(
  FILES
  GetScreenshot.srv
)

generate_messages(
  DEPENDENCIES
  sensor_msgs
  std_msgs
)

generate_dynamic_reconfigure_options(
  cfg/ScreenGrab.cfg
)

## Declare catkin package
catkin_package(
  CATKIN_DEPENDS message_runtime rospy roscpp sensor_msgs std_msgs dynamic_reconfigure nodelet
  DEPENDS OpenCV
)

## Include directories
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

## Optional: C++ library and node (if still used)
add_library(${PROJECT_NAME}
  src/screen_grab.cpp
)

add_executable(${PROJECT_NAME}_node
  src/screen_grab_node.cpp
)

target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} X11)
target_link_libraries(${PROJECT_NAME}_node ${PROJECT_NAME})

## Optional: image_show nodelet
add_library(image_show
  src/image_show.cpp
)

target_link_libraries(image_show
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
)

## Python setup
catkin_python_setup()

## Install Python ROS node
catkin_install_python(PROGRAMS
  src/screen_grab/main.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

## Install other files
install(TARGETS
  image_show
  ${PROJECT_NAME}
  ${PROJECT_NAME}_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
)

install(FILES
  nodelet_image_show.xml
  nodelet_screen_grab.xml
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(DIRECTORY launch/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
  PATTERN ".svn" EXCLUDE
)
