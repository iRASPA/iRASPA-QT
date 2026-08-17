# CPack configuration for the binary packages that .github/workflows/create-release.yml
# uploads. Nothing here affects a plain "cmake --build . --target install".

if(NOT IRASPA_BUILD_DEB_PACKAGE)
  return()
endif()

if(NOT UNIX OR APPLE)
  message(FATAL_ERROR "IRASPA_BUILD_DEB_PACKAGE only makes sense on Linux.")
endif()

set(CPACK_PACKAGE_NAME "iraspa")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt")
set(CPACK_PACKAGE_CONTACT "David Dubbeldam <d.dubbeldam@uva.nl>")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Molecular visualizer/editor for material science")
set(CPACK_PACKAGE_DESCRIPTION
  "iRASPA is a visualization package (with editing capabilities) aimed at material science. Examples of materials are metals, metal-oxides, ceramics, biomaterials, zeolites, clays, and metal-organic frameworks. iRASPA leverages the latest visualization technologies with high performance. It can handle large structures (hundreds of thousands of atoms), including ambient occlusion, with high frame rates.")
set(CPACK_STRIP_FILES ON)

# The install() destinations are relative, so this is what turns them into
# /usr/bin, /usr/share/applications and so on inside the package.
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_NAME "iraspa")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "David Dubbeldam <d.dubbeldam@uva.nl>")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${PROJECT_HOMEPAGE_URL}")

# Produces iraspa_<version>_<arch>.deb.
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

# The Qt, FFmpeg and Python runtime packages are named differently on every
# Ubuntu release we build for, so let dpkg-shlibdeps read the dependencies off
# the linked binary instead of maintaining a list per distribution. This needs
# dpkg-dev in the build environment.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
