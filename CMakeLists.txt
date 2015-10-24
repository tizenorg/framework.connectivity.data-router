CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(data-router C)

SET(SRCS src/dr-main.c
src/dr-modem.c
src/dr-usb.c
src/dr-noti-handler.c
src/dr-util.c
src/dr-common.c
src/dr-parser.c
src/dr-ipc.c
)


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED glib-2.0
				gio-2.0
				dbus-1
				dlog vconf
				capi-network-serial
				)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -fPIC -pie")
ENDFOREACH(flag)
#SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -finstrument-functions")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "/usr/bin")
SET(EXEC_PREFIX "\${prefix}")
SET(INCLUDEDIR "/usr/include")
SET(VERSION 1.0)

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{FACTORYFS}\"")
ADD_DEFINITIONS("-DTARGET")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data-router.rule DESTINATION /etc/smack/accesses.d)

ADD_SUBDIRECTORY(test)
