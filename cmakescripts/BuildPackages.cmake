# This file is included from the top-level CMakeLists.txt.  We just store it
# here to avoid cluttering up that file.

string(TOLOWER ${CMAKE_PROJECT_NAME} CMAKE_PROJECT_NAME_LC)


#
# Linux RPM and DEB
#

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

set(RPMARCH ${CPU_TYPE})
if(${CPU_TYPE} STREQUAL "x86_64")
	set(DEBARCH amd64)
else()
	set(DEBARCH ${CPU_TYPE})
endif()

if(TVNC_BUILDJAVA)
	set(TVNC_BUILDJAVA 1)
else()
	set(TVNC_BUILDJAVA 0)
endif()

if(NOT TVNC_JAVADIR)
	set(TVNC_JAVADIR ${CMAKE_INSTALL_PREFIX}/java)
endif()

configure_file(release/makerpm.in pkgscripts/makerpm)
configure_file(release/${CMAKE_PROJECT_NAME_LC}.spec.in
	pkgscripts/${CMAKE_PROJECT_NAME_LC}.spec @ONLY)

add_custom_target(rpm sh pkgscripts/makerpm
	SOURCES pkgscripts/makerpm)

configure_file(release/makedpkg.in pkgscripts/makedpkg)
configure_file(release/deb-control.in pkgscripts/deb-control)

add_custom_target(deb sh pkgscripts/makedpkg
	SOURCES pkgscripts/makedpkg)

endif() # Linux


#
# Windows installer (NullSoft Installer)
#

if(WIN32)

if(BITS EQUAL 64)
	set(INST_NAME ${CMAKE_PROJECT_NAME}64-${VERSION})
	set(INST_DEFS -DWIN64)
else()
	set(INST_NAME ${CMAKE_PROJECT_NAME}-${VERSION})
endif()

set(INST_DEPENDS vncviewer putty)
if(TVNC_BUILDJAVA)
	set(INST_DEFS ${INST_DEFS} "-DJAVA")
	set(INST_DEPENDS ${INST_DEPENDS} java)
endif()

if(MSVC_IDE)
	set(INSTALLERDIR ${CMAKE_CFG_INTDIR})
	set(INST_DEFS ${INST_DEFS} "-DBUILDDIR=${INSTALLERDIR}\\")
else()
	set(INSTALLERDIR .)
	set(INST_DEFS ${INST_DEFS} "-DBUILDDIR=")
endif()

configure_file(release/@CMAKE_PROJECT_NAME@.iss.in
	pkgscripts/@CMAKE_PROJECT_NAME@.iss)

add_custom_target(installer
	iscc -o${INSTALLERDIR} ${INST_DEFS} -F${INST_NAME}
		pkgscripts/@CMAKE_PROJECT_NAME@.iss
	DEPENDS ${INST_DEPENDS}
	SOURCES pkgscripts/@CMAKE_PROJECT_NAME@.iss)

endif() # WIN32


#
# Mac DMG
#

if(APPLE AND TVNC_BUILDJAVA)

set(DEFAULT_PACKAGEMAKER_PATH /Developer/Applications/Utilities)
set(PACKAGEMAKER_PATH ${DEFAULT_PACKAGEMAKER_PATH} CACHE PATH
	"Directory containing PackageMaker.app (default: ${DEFAULT_PACKAGEMAKER_PATH})")

string(REGEX REPLACE "/" ":" TVNC_MACPREFIX ${CMAKE_INSTALL_PREFIX})
string(REGEX REPLACE "^:" "" TVNC_MACPREFIX ${TVNC_MACPREFIX})

configure_file(release/makemacpkg.in pkgscripts/makemacpkg @ONLY)
configure_file(release/makemacapp.in pkgscripts/makemacapp)
configure_file(release/Info.plist.in pkgscripts/Info.plist)
configure_file(release/Info.plist.app.in pkgscripts/Info.plist.app)
configure_file(release/Description.plist.in pkgscripts/Description.plist)
configure_file(release/uninstall.in pkgscripts/uninstall)
configure_file(release/uninstall.applescript.in pkgscripts/uninstall.applescript)

add_custom_target(dmg sh pkgscripts/makemacpkg
	SOURCES pkgscripts/makemacpkg)

endif() # APPLE


#
# Generic
#

configure_file(release/makesrctarball.in pkgscripts/makesrctarball)

add_custom_target(srctarball sh pkgscripts/makesrctarball
	SOURCES pkgscripts/makesrctarball)

configure_file(release/makesrpm.in pkgscripts/makesrpm)

add_custom_target(srpm sh pkgscripts/makesrpm
	SOURCES pkgscripts/makesrpm)
