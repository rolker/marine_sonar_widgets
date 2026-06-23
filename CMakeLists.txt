# Copyright 2026 University of New Hampshire
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#    * Neither the name of the University of New Hampshire nor the names of its
#      contributors may be used to endorse or promote products derived from
#      this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.8)
project(marine_sonar_widgets)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(marine_acoustic_msgs REQUIRED)
find_package(marine_colormap REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets Gui OpenGL)

# QWidget / QOpenGLWidget classes carry Q_OBJECT macros, so MOC is required.
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(${PROJECT_NAME}_HEADERS
  include/${PROJECT_NAME}/color_map.hpp
  include/${PROJECT_NAME}/gpu_color_map.hpp
  include/${PROJECT_NAME}/ping_pairer.hpp
  include/${PROJECT_NAME}/row_extractor.hpp
  include/${PROJECT_NAME}/topic_filter.hpp
  include/${PROJECT_NAME}/waterfall_buffer.hpp
  include/${PROJECT_NAME}/waterfall_model.hpp
  include/${PROJECT_NAME}/waterfall_widget.hpp
  include/${PROJECT_NAME}/echogram_widget.hpp
  include/${PROJECT_NAME}/ping.hpp
)

set(${PROJECT_NAME}_SOURCES
  src/color_map.cpp
  src/gpu_color_map.cpp
  src/ping_pairer.cpp
  src/row_extractor.cpp
  src/topic_filter.cpp
  src/waterfall_buffer.cpp
  src/waterfall_model.cpp
  src/waterfall_widget.cpp
  src/echogram_widget.cpp
  src/ping.cpp
)

# SHARED so the library is position-independent and can link into downstream
# shared libraries / rqt plugins (a default STATIC archive lacks -fPIC and
# fails to link into a .so). Shared is also the ROS 2 convention.
add_library(${PROJECT_NAME} SHARED
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS}
)

# Require C99 and C++17 — propagates to anyone who links this library.
target_compile_features(${PROJECT_NAME} PUBLIC c_std_99 cxx_std_17)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(${PROJECT_NAME}
  marine_acoustic_msgs
  marine_colormap
)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Gui Qt5::OpenGL)

install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/
  DESTINATION include
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)

  # --- Pure-logic tests: compile the unit-under-test source directly. ---

  ament_add_gtest(test_decode_samples
    test/test_decode_samples.cpp
    src/waterfall_model.cpp
  )
  target_include_directories(test_decode_samples PRIVATE include)
  ament_target_dependencies(test_decode_samples marine_acoustic_msgs)

  ament_add_gtest(test_combine_rows
    test/test_combine_rows.cpp
    src/waterfall_model.cpp
  )
  target_include_directories(test_combine_rows PRIVATE include)
  ament_target_dependencies(test_combine_rows marine_acoustic_msgs)

  ament_add_gtest(test_ground_resample
    test/test_ground_resample.cpp
    src/waterfall_model.cpp
  )
  target_include_directories(test_ground_resample PRIVATE include)
  ament_target_dependencies(test_ground_resample marine_acoustic_msgs)

  ament_add_gtest(test_single_beam_extractor
    test/test_single_beam_extractor.cpp
    src/row_extractor.cpp
    src/waterfall_model.cpp
  )
  target_include_directories(test_single_beam_extractor PRIVATE include)
  ament_target_dependencies(test_single_beam_extractor marine_acoustic_msgs)

  ament_add_gtest(test_waterfall_buffer
    test/test_waterfall_buffer.cpp
    src/waterfall_buffer.cpp
  )
  target_include_directories(test_waterfall_buffer PRIVATE include)
  ament_target_dependencies(test_waterfall_buffer marine_acoustic_msgs)

  ament_add_gtest(test_color_map
    test/test_color_map.cpp
    src/color_map.cpp
  )
  target_include_directories(test_color_map PRIVATE include)
  ament_target_dependencies(test_color_map marine_acoustic_msgs marine_colormap)

  ament_add_gtest(test_ping_pairer
    test/test_ping_pairer.cpp
    src/ping_pairer.cpp
    src/waterfall_model.cpp
  )
  target_include_directories(test_ping_pairer PRIVATE include)
  ament_target_dependencies(test_ping_pairer marine_acoustic_msgs)

  ament_add_gtest(test_topic_filter
    test/test_topic_filter.cpp
    src/topic_filter.cpp
  )
  target_include_directories(test_topic_filter PRIVATE include)

  # ping.cpp delegates decoding to marine_sonar_widgets::decode_samples, which
  # lives in waterfall_model.cpp — compile it in so the test links standalone.
  ament_add_gtest(test_ping
    test/test_ping.cpp
    src/ping.cpp
    src/waterfall_model.cpp
  )
  target_include_directories(test_ping PRIVATE include)
  ament_target_dependencies(test_ping marine_acoustic_msgs)

  # --- Widget / GPU tests: link the main library for MOC metaobjects + GL. ---

  ament_add_gtest(test_waterfall_widget
    test/test_waterfall_widget.cpp
  )
  target_include_directories(test_waterfall_widget PRIVATE include)
  ament_target_dependencies(test_waterfall_widget marine_acoustic_msgs marine_colormap)
  # Link the main library so the widget's MOC metaobjects are available.
  # Gui/OpenGL for QOpenGLWidget + the offscreen GL context the test creates.
  target_link_libraries(test_waterfall_widget
    ${PROJECT_NAME} Qt5::Widgets Qt5::Gui Qt5::OpenGL)
  set_tests_properties(test_waterfall_widget PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LIBGL_ALWAYS_SOFTWARE=1")

  # Offscreen-GL CPU/GPU colormap parity test. Self-skips when no GL context can
  # be created, so it never fails on a headless runner without software GL.
  ament_add_gtest(test_gpu_color_map
    test/test_gpu_color_map.cpp
  )
  target_include_directories(test_gpu_color_map PRIVATE include)
  ament_target_dependencies(test_gpu_color_map marine_colormap)
  target_link_libraries(test_gpu_color_map
    ${PROJECT_NAME} Qt5::Widgets Qt5::Gui Qt5::OpenGL)
  set_tests_properties(test_gpu_color_map PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LIBGL_ALWAYS_SOFTWARE=1")

  # Offscreen echogram widget smoke/robustness test. Links the main library so
  # the widget's MOC metaobjects are available; the GPU render path needs an
  # offscreen GL context (software GL when no GPU is present).
  ament_add_gtest(test_echogram_widget
    test/test_echogram_widget.cpp
  )
  target_include_directories(test_echogram_widget PRIVATE include)
  ament_target_dependencies(test_echogram_widget marine_acoustic_msgs)
  target_link_libraries(test_echogram_widget
    ${PROJECT_NAME} Qt5::Widgets Qt5::Gui Qt5::OpenGL)
  set_tests_properties(test_echogram_widget PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LIBGL_ALWAYS_SOFTWARE=1")

  ament_lint_auto_find_test_dependencies()
endif()

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_include_directories(include)
ament_export_dependencies(marine_acoustic_msgs marine_colormap
  Qt5Widgets Qt5Gui Qt5OpenGL)

ament_package()
