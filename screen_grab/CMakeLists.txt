cmake_minimum_required(VERSION 2.8.3)
project(screen_grab)

## Find catkin macros and libraries
find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  dynamic_reconfigure
  image_transport
  message_generation
  nodelet
  roscpp
  roslint
  sensor_msgs
  std_msgs
)

find_package(OpenCV REQUIRED COMPONENTS highgui)

roslint_cpp()

################################################
## Declare ROS messages, services and actions ##
################################################

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

###################################
## catkin specific configuration ##
###################################

catkin_package(
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS message_runtime dynamic_reconfigure nodelet roscpp sensor_msgs std_msgs
  DEPENDS OpenCV
)

###########
## Build ##
###########

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}
  src/${PROJECT_NAME}.cpp
)

add_dependencies(${PROJECT_NAME}
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
)

target_link_libraries(${PROJECT_NAME}
  ${catkin_LIBRARIES}
  X11
)

add_executable(${PROJECT_NAME}_node
  src/${PROJECT_NAME}_node.cpp
)

add_dependencies(${PROJECT_NAME}_node
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
)

target_link_libraries(${PROJECT_NAME}_node
  ${PROJECT_NAME}
)

add_library(image_show
  src/image_show.cpp
)

target_link_libraries(image_show
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
)

#############
## Install ##
#############

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
  nodelet_${PROJECT_NAME}.xml
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(DIRECTORY launch/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
  PATTERN ".svn" EXCLUDE
)
