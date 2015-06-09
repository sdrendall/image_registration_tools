PROJECT(image_registration_tools)
cmake_minimum_required(VERSION 2.7)

FIND_PACKAGE(ITK)
IF(ITK_FOUND)
    INCLUDE(${ITK_USE_FILE})
ENDIF(ITK_FOUND)

ADD_EXECUTABLE(image_to_image_registration image_to_image_registration.cpp)
ADD_EXECUTABLE(slice_atlas slice_atlas.cpp)

TARGET_LINK_LIBRARIES(image_to_image_registration ${ITK_LIBRARIES})
TARGET_LINK_LIBRARIES(slice_atlas ${ITK_LIBRARIES})

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
