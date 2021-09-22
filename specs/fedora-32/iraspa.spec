Name:           iraspa
Version:        2.1.5
Release:        1%{?dist}
Summary:        A molecular visualizer/editor
Group:          Sciences/Chemistry
License:        GPLv3+
Source0:        %{name}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-root
Packager:       David Dubbeldam
Url:            [https://www.iraspa.org]
Vendor:         David Dubbeldam

%description
iRASPA is a visualization package (with editing capabilities) aimed at
material science. Examples of materials are metals, metal-oxides, ceramics,
biomaterials, zeolites, clays, and metal-organic frameworks. iRASPA leverages
the latest visualization technologies with stunning performance. It can
handle large structures (hundreds of thousands of atoms), including ambient
occlusion, with high frame rates.

BuildRequires:  gcc-c++, dbus-devel, qt5-qtbase-devel, qt5-qttools-devel, qt5-qtwebengine-devel, qt5-qtdeclarative-devel, python3-devel, ffmpeg-devel, xz-devel, zlib-devel, opencl-headers, ocl-icd-devel
Requires:       dbus-libs, qt5-qtbase, qt5-qtbase-gui, qt5-qtwebchannel, qt5-qtlocation, qt5-qtwebengine, qt5-qtdeclarative, ocl-icd, platform-python-devel, xz-devel, zlib, ffmpeg-libs

%prep
%setup -q -n %{name}

%build
qmake-qt5 iraspa.pro
make %{?_smp_mflags} BINDIR=%{_bindir}

%install
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/share/applications
mkdir -p $RPM_BUILD_ROOT/usr/share/pixmaps
mkdir -p $RPM_BUILD_ROOT/usr/share/%{name}
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/16x16/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/32x32/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/48x48/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/64x64/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/128x128/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/256x256/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/512x512/apps
install -m 755 %{name} $RPM_BUILD_ROOT/usr/bin/%{name}
install -m 644 datafiles/databasecoremof.irspdoc $RPM_BUILD_ROOT/usr/share/%{name}
install -m 644 datafiles/databasecoremofddec.irspdoc $RPM_BUILD_ROOT/usr/share/%{name}
install -m 644 datafiles/databaseiza.irspdoc $RPM_BUILD_ROOT/usr/share/%{name}
install -m 644 datafiles/libraryofstructures.irspdoc $RPM_BUILD_ROOT/usr/share/%{name}
install -m 644 datafiles/license-gpl.pdf $RPM_BUILD_ROOT/usr/share/%{name}
install -m 644 datafiles/acknowledgedlicenses.pdf $RPM_BUILD_ROOT/usr/share/%{name}
install -m 644 datafiles/%{name}.desktop $RPM_BUILD_ROOT/usr/share/applications
install -m 644 datafiles/%{name}.png $RPM_BUILD_ROOT/usr/share/pixmaps
install -m 644 datafiles/iraspa.iconset/icon_16x16.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/16x16/apps/%{name}.png
install -m 644 datafiles/iraspa.iconset/icon_32x32.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/32x32/apps/%{name}.png
install -m 644 datafiles/iraspa.iconset/icon_48x48.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/48x48/apps/%{name}.png
install -m 644 datafiles/iraspa.iconset/icon_64x64.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/64x64/apps/%{name}.png
install -m 644 datafiles/iraspa.iconset/icon_128x128.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/128x128/apps/%{name}.png
install -m 644 datafiles/iraspa.iconset/icon_256x256.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/256x256/apps/%{name}.png
install -m 644 datafiles/iraspa.iconset/icon_512x512.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/512x512/apps/%{name}.png

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%{_datadir}/%{name}/libraryofstructures.irspdoc
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.png
%{_datadir}/%{name}/databasecoremof.irspdoc
%{_datadir}/%{name}/databasecoremofddec.irspdoc
%{_datadir}/%{name}/databaseiza.irspdoc
%{_datadir}/%{name}/acknowledgedlicenses.pdf
%{_datadir}/%{name}/license-gpl.pdf
%{_datadir}/icons/hicolor/16x16/apps/%{name}.png
%{_datadir}/icons/hicolor/32x32/apps/%{name}.png
%{_datadir}/icons/hicolor/48x48/apps/%{name}.png
%{_datadir}/icons/hicolor/64x64/apps/%{name}.png
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png
%{_datadir}/icons/hicolor/256x256/apps/%{name}.png
%{_datadir}/icons/hicolor/512x512/apps/%{name}.png

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR

%changelog
    * Mon Sep 28 2020 dubbelda
    - Initial build
