# 3 june 2016

find_package(PkgConfig REQUIRED)
pkg_check_modules(QT REQUIRED Qt5Widgets)

list(APPEND _LIBUI_SOURCES
	qt5/alloc.cpp
	qt5/area.cpp
	qt5/box.cpp
	qt5/button.cpp
	qt5/checkbox.cpp
	qt5/colorbutton.cpp
	qt5/combobox.cpp
	qt5/control.cpp
	qt5/datetimepicker.cpp
	qt5/debug.cpp
	qt5/draw.cpp
	qt5/drawmatrix.cpp
	qt5/drawpath.cpp
	qt5/drawtext.cpp
	qt5/editablecombo.cpp
	qt5/entry.cpp
	qt5/form.cpp
	qt5/fontbutton.cpp
	qt5/grid.cpp
	qt5/group.cpp
	qt5/label.cpp
	qt5/main.cpp
	qt5/menu.cpp
	qt5/multilineentry.cpp
	qt5/progressbar.cpp
	qt5/radiobuttons.cpp
	qt5/separator.cpp
	qt5/slider.cpp
	qt5/spinbox.cpp
	qt5/stddialogs.cpp
	qt5/tab.cpp
	qt5/text.cpp
	qt5/window.cpp
)
set(_LIBUI_SOURCES ${_LIBUI_SOURCES} PARENT_SCOPE)

list(APPEND _LIBUI_INCLUDEDIRS
	qt
)
set(_LIBUI_INCLUDEDIRS _LIBUI_INCLUDEDIRS PARENT_SCOPE)

set(_LIBUI_CFLAGS
	${QT_CFLAGS}
PARENT_SCOPE)

set(_LIBUI_LDFLAGS "${_LIBUI_LDFLAGS} ${QT_LDFLAGS}" PARENT_SCOPE)


# set(_LIBUI_LIBS
# 	"${QT_LDFLAGS}" Qt5::Widgets
# PARENT_SCOPE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_DEBUG")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_DEBUG")
endif()
