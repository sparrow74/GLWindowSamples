
Name:       dali-nativegl-library
Summary:    dali natvie gl libary to bind NUI
Version:    0.0.1
Release:    1
Group:      DALI/NUI
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: 	dali-nativegl-library.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glesv2)

%{!?TZ_SYS_RO_SHARE: %global TZ_SYS_RO_SHARE /usr/share}

%description
An dali nativegl library to bind with NUI application.


%package devel
Summary:  dali natvie gl libary to bind NUI (Development)
Requires: %{name} = %{version}-%{release}

%description devel
%devel_desc

%if 0%{?gcov:1}
%package -n dali-nativegl-library-gcov
Summary: dali-nativegl-library gcov data package
Group: Development/Libraries

%description -n dali-nativegl-library-gcov
dali-nativegl-library gcno data for gcov
%endif

%prep
%setup -q
cp %{SOURCE1001} .

%build
export CFLAGS+=" -DEFL_BETA_API_SUPPORT "
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%if 0%{?gcov:1}
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER} -DDALINATIVEGLGCOV=ON \
%else
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER} -DDALINATIVEGL=OFF \
%endif

make %{?jobs:-j%jobs}

%if 0%{?gcov:1}
mkdir -p gcov-obj
find . -name '*.gcno' -exec cp '{}' gcov-obj ';'
%endif

%install
%make_install

%if 0%{?gcov:1}
mkdir -p %{buildroot}%{_datadir}/gcov/obj
install -m 0644 gcov-obj/* %{buildroot}%{_datadir}/gcov/obj
%endif

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%{_libdir}/libdali-nativegl-library.so.*

%files devel
%manifest %{name}.manifest
%{_includedir}/ui/dali-nativegl-library.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libdali-nativegl-library.so
%exclude %{_includedir}/ui/config.h

%if 0%{?gcov:1}
%files -n dali-nativegl-library-gcov
%{_datadir}/gcov/obj/*
%endif
