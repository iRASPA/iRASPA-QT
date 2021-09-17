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

Package creation:
* Debian packages, use: 'debuild -i -us -uc -b -j8'.
   * topline changelog for Ubuntu 20.04: 'iraspa (2.0.0-ubuntu-20.04) focal; urgency=low'.
   * topline changelog for Ubuntu 18.04: 'iraspa (2.0.0-ubuntu-18.04) bionic; urgency=low'.
   * topline changelog for Ubuntu 16.04: 'iraspa (2.0.0-ubuntu-16.04) xenial; urgency=low'.
   * topline changelog for Ubuntu 14.04: 'iraspa (2.0.0-ubuntu-14.04) trusty; urgency=low'.
   * topline changelog for Debian 10.6: 'iraspa (2.0.0-debian-10.6) unstable; urgency=medium'.
   * topline changelog for Manjaro 20.1: 'iraspa (2.0.0-manjaro-20.1) mikah; urgency=low'.
   * topline changelog for MX linux 19.2: 'iraspa (2.0.0-mxlinux-19.2) patito; urgency=low'.
   * topline changelog for Mint 19.3: 'iraspa (2.0.0-mint-19.3) tricia; urgency=low'.
   * topline changelog for Mint 20: 'iraspa (2.0.0-mint-20) ulyana; urgency=low'.
   * topline changelog for Popos 20.04: 'iraspa (2.0.0-popos-20.04) focal; urgency=low'.
   * topline changelog for Elementary OS 5.1: 'iraspa (2.0.0-elementaryos-5.1) hera; urgency=low'.
* Snap package: in the 'iraspa' directory, run 'snapcraft'  (or 'snapcraft --use-lxd' for software virtualization when kvm is not working).
* Arch packages: 
   * create a compressed tar archive of the 'iraspa' sub-directory: 'tar -zcvf iraspa.tar.gz iraspa'.
   * put the compressed tar archive of the source into the 'arch' directory.
   * in the 'arch' directory, run 'makepkg'
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
* Windows-10:
   * the 'iraspa.vcxproj' has been generated using the pro-file import in visual studio 2019.
   * to run, copy the content of the directory 'datafiles' to the 'debug' or 'release' visual studio directories.
* Windows-10 store ('windowsstore' directory): 
   * add the directory 'iraspa' that contains all the required Qt files, license and acknowledgement pdfs, and structure databases that will be copied into the appx windows-10 package. 
   * add the directory 'Images' with all the various icons.
   * add 'IRASPAPackaging_StoreKey.pfx'.
   * add 'IRASPAPackaging_TemporaryKey.pfx'.
   * add 'Package.appxmanifest'.
   * add 'Package.StoreAssociation.xml'.
