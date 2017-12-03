#
#   Copyright 2017   Jan Kammerath
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

Name:           iptvx
Version:        0.5
Release:        1
Summary:		iptv player and streamer
License:        Apache License 2.0
Group:          Productivity/Multimedia/Video/Players
Url:            http://iptvx.org
Source:         %{name}-%{version}.tar.gz

# Build requirements
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:	pkgconfig(webkit2gtk-4.0)
BuildRequires:	pkgconfig(libconfig)
BuildRequires:	pkgconfig(libvlc)
BuildRequires:	pkgconfig(libcurl)
BuildRequires:	pkgconfig(libxml-2.0)
BuildRequires:	pkgconfig(sdl)
BuildRequires:	pkgconfig(SDL_image)
BuildRequires:	pkgconfig(json-c)

# Define build root
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

# Runtime requirements
Requires:  		pkgconfig(glib-2.0)
Requires:		pkgconfig(webkit2gtk-4.0)
Requires:		pkgconfig(libconfig)
Requires:		pkgconfig(libvlc)
Requires:		pkgconfig(libcurl)
Requires:		pkgconfig(libxml-2.0)
Requires:		pkgconfig(sdl)
Requires:		pkgconfig(SDL_image)
Requires:		pkgconfig(json-c)

%description
IPTV player and streamer for Linux that allows to play any stream that LibVLC can play, offers an overlay based on WebKit using HTML5, JavaScript and CSS and uses XMLTV data for EPG information. It allows the playback of URLs, files and can grab URLs from shell scripts. XMLTV EPG data can be downloaded directly from URLs or grabbed from shell scripts.

%prep
%setup

%build
make

%install
mkdir -p %{buildroot}/usr/bin/
cp bin/iptvx %{buildroot}/usr/bin/iptvx
chmod 755 %{buildroot}/usr/bin/iptvx
mkdir -p %{buildroot}/etc/iptvx
cp cfg/iptvx.conf %{buildroot}/etc/iptvx/iptvx.conf
cp cfg/channels.conf %{buildroot}/etc/iptvx/channels.conf
mkdir -p %{buildroot}/var/iptvx/data/epg
mkdir -p %{buildroot}/var/iptvx/data/logo
cp app %{buildroot}/var/iptvx/ -R

%files
%{buildroot}/usr/bin/iptvx
%{buildroot}/etc/iptvx/
%{buildroot}/var/iptvx/
