cmake_minimum_required(VERSION 2.8.3)
project(screen_grab)

## Find dependencies
find_package(catkin REQUIRED COMPONENTS
  dynamic_reconfigure
  message_generation
  sensor_msgs
  std_msgs
  rospy
)

## First: call python setup before anything message-related
catkin_python_setup()

## Define service and dynamic reconfigure
add_service_files(
  FILES
  GetScreenshot.srv
)

generate_dynamic_reconfigure_options(
  cfg/ScreenGrab.cfg
)

generate_messages(
  DEPENDENCIES
  sensor_msgs
  std_msgs
)

## Declare package
catkin_package(
  CATKIN_DEPENDS message_runtime rospy sensor_msgs std_msgs dynamic_reconfigure
)

## Include directories
include_directories(
  ${catkin_INCLUDE_DIRS}
)

## Install Python script
catkin_install_python(PROGRAMS
  src/screen_grab/main.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
