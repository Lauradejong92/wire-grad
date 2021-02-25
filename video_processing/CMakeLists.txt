#cmake_minimum_required(VERSION 3.0.2)
cmake_minimum_required(VERSION 2.8)
project(video_processing)

#export OpenCV_DIR=~/opencv-master/OpenCV/cmake
#CMAKE_PREFIX_PATH(
#~/opencv-master/OpenCV)
#list(APPEND CMAKE_PREFIX_PATH "~/opencv-master/OpenCV")
#set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ~/opencv-master/OpenCV)
#set(OpenCV_DIR=/~/opencv-master/cmake)
#cmake -DOpenCV_DIR=~/opencv-master/OpenCV ..

find_package(#catkin REQUIRED COMPONENTS
OpenCV REQUIRED
#  roscpp

#  rospy
#  std_msgs
)

catkin_package()

include_directories(
  ${catkin_INCLUDE_DIRS}
	#~/opencv-master/OpenCV

)

add_executable( DisplayImage src/DisplayImage.cpp )
target_link_libraries( DisplayImage ${OpenCV_LIBS} )

add_executable( DetectImage src/DetectImage.cpp )
target_link_libraries( DetectImage ${OpenCV_LIBS} )
