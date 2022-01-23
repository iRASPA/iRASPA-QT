Name:           iraspa
Version:        2.2.4
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

BuildRequires:  gcc11-c++, gcc-c++, qt6-tools-devel, qt6-core-devel, qt6-concurrent-devel, qt6-opengl-devel, qt6-base-devel, qt6-openglwidgets-devel, qt6-platformsupport-devel-static, ffmpeg-4-libavcodec-devel, ffmpeg-4-libavdevice-devel, ffmpeg-4-libavfilter-devel, ffmpeg-4-libavformat-devel, ffmpeg-4-libavresample-devel, ffmpeg-4-libavutil-devel, ffmpeg-4-libpostproc-devel, ffmpeg-4-libswresample-dev, ffmpeg-4-libswscale-devel, xz-devel, zlib-devel, opencl-headers, ocl-icd-devel, python3-devel
Requires:       libQt6Core6, libQt6DBus6, libQt6Gui6, ffmpeg-4, libpython3_9-1_0, liblzma5, zlib

%prep
%setup -q -n %{name}

%build
qmake6 iraspa.pro
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
