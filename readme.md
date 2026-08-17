# iRASPA-QT

iRASPA is a visualization package (with editing capabilities) aimed at material science. Examples of materials are metals, metal-oxides, ceramics, biomaterials, zeolites, clays, and metal-organic frameworks. iRASPA leverages the latest visualization technologies with stunning performance. iRASPA extensively utilizes GPU computing. For example, void-fractions and surface areas can be computed in a fraction of a second for small/medium structures and in a few seconds for very large unit cells. It can handle large structures (hundreds of thousands of atoms), including ambient occlusion, with high frame rates.

Main features of iRASPA are:
* structure creation and editing.
* creating high-quality pictures and movies.
* ambient occlusion and high-dynamic range rendering.
* collage of structures.
* (transparent) adsorption surfaces.
* text-annotation.
* primitives like cylinders, spheres, and polygonal prisms.
* cell replicas and supercells.
* symmetry operations like space group and primitive cell detection.
* screening of structures using user-defined predicates.
* GPU-computation of void-fraction and surface areas in a matter of seconds.

Input formats:
* CIF.
* mmCIF.
* PDB.
* XYZ.
* VASP POSCAR/CONTCAR.

Output:
* CIF-, mmCIF-, PDB-, POSCAR-, or XYZ-files for structures.
* 8 bits, RGB, loss-less TIFF for pictures.
* mp4 (h264, h265, v9) for movies.

![](https://raw.githubusercontent.com/iRASPA/iRASPA-QT/master/iraspa/screenshots/linux/screenshot1.png)
*Screenshot of iRASPA*

Building:
* iRASPA builds with CMake (3.21 or newer) and Qt 6 (Qt 5.6 or newer still works).
* configure and build: 'cmake -S . -B build -DCMAKE_BUILD_TYPE=Release' followed by 'cmake --build build'.
* install: 'cmake --install build --prefix /usr/local'.
* the renderer backends are selected with '-DIRASPA_USE_OPENGL=ON|OFF' and '-DIRASPA_USE_VULKAN=ON|OFF'; both are enabled by default. A binary with both probes at runtime and uses Vulkan when a device enumerates, otherwise OpenGL. The Vulkan backend requires 'glslangValidator' to compile the shaders.
* the Qt 5 help browser can be built against QtWebEngine with '-DIRASPA_USE_WEBENGINE=ON'; it is off by default. Qt 6 builds always use a plain QWidget there.
* unit tests: configure with '-DIRASPA_BUILD_TESTS=ON' and run them with 'ctest --test-dir build'.
* dependencies: Qt (base, tools), OpenCL, Python 3, FFmpeg, liblzma, zlib, and Vulkan (MoltenVK on macOS). x264 is only linked on Windows. QtWebEngine is optional and Qt 5 only.
* the 'iraspa' sub-directory is a self-contained CMake project as well, which is what the packaging scripts below build.
* 'CMakePresets.json' collects the configurations that are used regularly. Run 'cmake --list-presets' to see them, then for example 'cmake --preset macos-apple-silicon', 'cmake --build --preset macos-apple-silicon' and 'ctest --preset macos-apple-silicon'. Linux package presets are named 'linux-debian-11' through 'linux-debian-13', 'linux-ubuntu-18' through 'linux-ubuntu-26', and 'linux-arch'; the same preset is used for x86_64 and arm64.

Version numbers:
* the version lives in 'iraspa/CMakeLists.txt' as 'IRASPA_VERSION'; CPack, the macOS bundle and the '-DIRASPA_VERSION=' override used by the release workflow all derive from it.
* configuring with '-DIRASPA_VERSION=2.4.0' overrides the literal and, being a cache entry, keeps overriding it until it is cleared with 'cmake -U IRASPA_VERSION -S . -B build'.
* 'python bump_version.py <major|minor|patch>' or 'python bump_version.py 2.4.0' rewrites it together with the about box, the snap, the two PKGBUILDs, the topmost debian changelog entry and the RPM spec files. It fails loudly if any of those no longer match, so a file that grows or loses a version number does not silently drift.
* the build number in the about box ('Version 2.3.7 (build 142)') is a separate counter and is not touched.

Package creation:
* GitHub releases: run the 'create-binary-packages' workflow ('.github/workflows/create-release.yml') from the Actions tab. It tags the release and attaches packages for Debian 11/12/13, Ubuntu 18.04/20.04/22.04/24.04/26.04 and Arch Linux, each on x86_64 and arm64. The version that is entered there is passed to CMake as '-DIRASPA_VERSION=' and ends up in the package name.
   * Debian and Ubuntu jobs use the matching 'linux-debian-*' / 'linux-ubuntu-*' preset, which enables 'IRASPA_BUILD_DEB_PACKAGE' so that 'ninja package' produces 'iraspa_<version>_<arch>.deb' through CPack.
   * Arch jobs use 'linux-arch' and 'makepkg' from 'arch/PKGBUILD'.
   * both renderer backends are compiled in everywhere except Ubuntu 18.04, where glslang was never packaged. The binary picks Vulkan at runtime when a device is present and falls back to OpenGL otherwise.
   * to build a single target without cutting a release, leave 'create-release' unchecked and set 'target' to 'debian-12', 'ubuntu-18', 'arch', or another matrix name.
   * to reproduce a Debian/Ubuntu job locally: 'cmake --preset linux-ubuntu-24 -DIRASPA_VERSION=2.3.7' followed by 'cmake --build --preset linux-ubuntu-24-package'.
* Debian packages, use: 'debuild -i -us -uc -b -j8'.
   * topline changelog for Ubuntu 21.04: 'iraspa (2.3.7-ubuntu-21.04) hirsute; urgency=low'.
   * topline changelog for Ubuntu 20.04: 'iraspa (2.3.7-ubuntu-20.04) focal; urgency=low'.
   * topline changelog for Ubuntu 18.04: 'iraspa (2.3.7-ubuntu-18.04) bionic; urgency=low'.
   * topline changelog for Ubuntu 16.04: 'iraspa (2.3.7-ubuntu-16.04) xenial; urgency=low'.
   * topline changelog for Ubuntu 14.04: 'iraspa (2.3.7-ubuntu-14.04) trusty; urgency=low'.
   * topline changelog for Debian 10.6: 'iraspa (2.3.7-debian-10.6) unstable; urgency=medium'.
   * topline changelog for Manjaro 20.1: 'iraspa (2.3.7-manjaro-20.1) mikah; urgency=low'.
   * topline changelog for MX linux 19.2: 'iraspa (2.3.7-mxlinux-19.2) patito; urgency=low'.
   * topline changelog for MX linux 19.3: 'iraspa (2.3.7-mxlinux-19.3) patito; urgency=low'.
   * topline changelog for MX linux 19.4: 'iraspa (2.3.7-mxlinux-19.4) patito; urgency=low'.
   * topline changelog for Mint 19.3: 'iraspa (2.3.7-mint-19.3) tricia; urgency=low'.
   * topline changelog for Mint 20: 'iraspa (2.3.7-mint-20) ulyana; urgency=low'.
   * topline changelog for Mint 20.1: 'iraspa (2.3.7-mint-20.1) ulyssa; urgency=low'.
   * topline changelog for Mint 20.2: 'iraspa (2.3.7-mint-20.2) uma; urgency=low'.
   * topline changelog for Popos 20.04: 'iraspa (2.3.7-popos-20.04) focal; urgency=low'.
   * topline changelog for Elementary OS 5.1: 'iraspa (2.3.7-elementaryos-5.1) hera; urgency=low'.
* Snap package: in the 'iraspa' directory, run 'snapcraft'  (or 'snapcraft --use-lxd' for software virtualization when kvm is not working).
* Arch packages:
   * from a GitHub release tarball, use 'arch/from-github/PKGBUILD' and run 'makepkg'.
   * from a checkout, copy 'arch/PKGBUILD' next to a tarball of the repository root named 'iraspa.tar.gz' (it has to contain 'CMakePresets.json') and run 'makepkg', or put the checkout in 'src/' and run 'makepkg -e'.
   * both PKGBUILDs configure with the 'linux-arch' preset and install with 'cmake --install'.
* RPM packages:
   * 'mkdir ~/rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}'.
   * create a compressed tar archive of the 'iraspa' sub-directory: 'tar -zcvf iraspa.tar.gz iraspa'.
   * put the compressed tar archive of the source into the '~/rpmbuild/SOURCES' directory.
   * put spec-file into the '~/rpmbuild/SPECS' directory.
   * build the RPM package from the '~/rpmbuild/SPECS' directory using: 'rpmbuild -ba iraspa.spec'.
   * a binary RPM-package will be created in the '~/rpmbuild/RPMS/x86_64' directory.
   * a source SRPM-package will be created in the '~/rpmbuild/SRPMS/x86_64' directory.
   * opensuse15.2 use: 'rpmbuild --define 'dist .opensuse15.2' -ba iraspa.spec'.
   * opensuse42.3 use: 'rpmbuild --define 'dist .opensuse42.3' -ba iraspa.spec'.
   * opensuse tumbleweed use: 'rpmbuild --define 'dist .opensuse_tumbleweed' -ba iraspa.spec'.
   * fedora: sudo dnf -y install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
* Windows-10:
   * open the 'iraspa' directory as a CMake project in visual studio, or configure it from the command line with the vcpkg toolchain file: 'cmake -S iraspa -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake'.
   * to run, copy the content of the directory 'datafiles' to the 'debug' or 'release' build directories.
* Windows-10 store ('windowsstore' directory): 
   * add the directory 'iraspa' that contains all the required Qt files, license and acknowledgement pdfs, and structure databases that will be copied into the appx windows-10 package. 
   * add the directory 'Images' with all the various icons.
   * add 'IRASPAPackaging_StoreKey.pfx'.
   * add 'IRASPAPackaging_TemporaryKey.pfx'.
   * add 'Package.appxmanifest'.
   * add 'Package.StoreAssociation.xml'.
