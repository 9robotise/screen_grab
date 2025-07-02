cmake_minimum_required(VERSION 2.8.3)
project(screen_grab)

find_package(catkin REQUIRED COMPONENTS
  dynamic_reconfigure
  message_generation
  sensor_msgs
  std_msgs
  rospy
)

## Service and dynamic reconfigure definitions
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

catkin_package(
  CATKIN_DEPENDS message_runtime rospy sensor_msgs std_msgs dynamic_reconfigure
)

include_directories(
  ${catkin_INCLUDE_DIRS}
)

catkin_python_setup()

## Install Python script
catkin_install_python(PROGRAMS
  src/screen_grab/main.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
