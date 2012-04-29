cmake_minimum_required(VERSION 2.8)
project(foobar CXX)
set(version 0.1)

include(FindPkgConfig)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(NIHPP REQUIRED nihpp)

include_directories(src/ ./)
add_definitions(-std=gnu++0x
   -DVERSION="${version}"
   -DGETTEXT_PACKAGE="${CMAKE_PROJECT_NAME}"
   -DPROGRAMNAME_LOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale"
   ${GTKMM_CFLAGS} ${NIHPP_CFLAGS})

add_executable(imgpacker
  src/imgpack/algorithm/bin-packer.cc
  src/imgpack/algorithm/bin-packer.hh
  src/imgpack/algorithm/rectangles.cc
  src/imgpack/algorithm/rectangles.hh
  src/imgpack/application.cc
  src/imgpack/application.hh
  src/imgpack/gtkui/collage-viewer.cc
  src/imgpack/gtkui/collage-viewer.hh
  src/imgpack/gtkui/gtk-application.cc
  src/imgpack/gtkui/gtk-application.hh
  src/imgpack/gtkui/image-list.cc
  src/imgpack/gtkui/image-list.hh
  src/imgpack/gtkui/main-window.cc
  src/imgpack/gtkui/main-window.hh
  src/imgpack/gtkui/pixbuf-loader.cc
  src/imgpack/gtkui/pixbuf-loader.hh
  src/imgpack/util/async-operation.cc
  src/imgpack/util/async-operation.hh
  src/imgpack/util/logger.cc
  src/imgpack/util/logger.hh
  src/imgpack/util/thread-pool.cc
  src/imgpack/util/thread-pool.hh
  src/main.cc)

target_link_libraries(imgpacker ${GTKMM_LDFLAGS} -lasprintf)

install(TARGETS imgpacker RUNTIME DESTINATION bin)

add_custom_target(run
  COMMAND ${CMAKE_BINARY_DIR}/imgpacker
  DEPENDS imgpacker)
