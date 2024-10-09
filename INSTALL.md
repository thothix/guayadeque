# Guayadeque Music Player

Guayadeque is a lightweight and easy-to-use music player and music collection organizer
that can easily manage large music collections and supports smart playlists.
In the technical side, it's written in C++, uses the wxWidget toolkit and the
Gstreamer media framework.

Tiago T Barrionuevo [<thothix@protonmail.com>](mailto:thothix@protonmail.com)  
see [LICENSE](LICENSE)

- [Github](https://github.com/thothix/guayadeque)
- [Latest release](https://github.com/thothix/guayadeque/releases/latest)

---

# Build

Need installed cmake, g++, wxWidgets 3.0/3.2, gstreamer1.0, sqlite3, libwxsqlite3, taglib, jsoncpp, libcurl, libdbus-1, libgio

It's been developed in XUbuntu and Linux Mint.

## Dependencies

### Ubuntu (pre 20.0):

```bash
sudo apt install libgdk-pixbuf2.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.0-dev libwxbase3.0-dev libtag-extras-dev libcurl4-gnutls-dev libdbus-1-dev libjsoncpp-dev libicu-dev cmake g++ binutils
```

### Ubuntu 20.04, Linux Mint 20

```bash
sudo apt install libgdk-pixbuf2.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.0-dev libwxbase3.0-dev libtag1-dev libtag-extras-dev libcurl4-gnutls-dev libdbus-1-dev libjsoncpp-dev libicu-dev cmake g++ binutils
```

### Ubuntu 22.04, Linux Mint 21

```bash
sudo apt install libgdk-pixbuf2.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.0-dev libwxbase3.0-dev libtag1-dev libtag-extras-dev libcurl4-gnutls-dev libdbus-1-dev libjsoncpp-dev libicu-dev gettext cmake g++ binutils
```

### Ubuntu 24.04, Linux Mint 22

```bash
sudo apt install libgdk-pixbuf2.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.2-dev libtag1-dev libcurl4-gnutls-dev libdbus-1-dev libjsoncpp-dev libicu-dev gettext cmake g++ binutils
```

### Debian 12

```bash
sudo apt install libgdk-pixbuf2.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.2-dev libtag1-dev libcurl4-gnutls-dev libdbus-1-dev libjsoncpp-dev libicu-dev cmake
```

### Mageia 9

```bash
sudo urpmi gstreamer1.0-devtools lib64wx_gtk3u_wxsqlite3_3.2-devel lib64taglib-devel lib64sqlite3-devel lib64curl-devel lib64dbus-devel lib64gio2.0_0 lib64jsoncpp-devel libicu-devel cmake binutils
```

### Arch Linux

```bash
sudo pacman -S wxgtk3 gstreamer gst-plugins-base gst-plugins-good sqlite wxsqlite3 taglib curl dbus gdk-pixbuf2 jsoncpp libicu cmake
```

---

### Optional dependencies

#### Extra audio playback support

- `gstreamer1.0-libav`: Provides support for a wide range of audio and video codecs, enabling Guayadeque to play more media formats (like `DSD/DSF`).
- `gstreamer1.0-plugins-bad`: Includes additional GStreamer plugins for enhanced functionality, but may have stability or licensing issues.
- `gstreamer1.0-plugins-ugly`: Offers support for certain audio and video formats that may have distribution or patent issues in some countries.

#### Other

- `libgpod-dev`: Offers comprehensive support for managing and interacting with iPod devices.
- `gvfs`: Allows Guayadeque to access and manage files through GVFS (GNOME Virtual File System), which is useful for handling remote or virtual file systems.

##### Ubuntu, Linux Mint, Debian 12

```bash
sudo apt install libgpod-dev gstreamer1.0-libav gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly
```

##### Arch Linux

```bash
sudo pacman -S libgpod gst-libav gst-plugins-bad gst-plugins-ugly gvfs
```

---

## Build

### Normal build

```bash
./build
sudo make install
```

### Faster build on multi-core systems

#### Old cmake versions

```bash
./build -j$(nproc) -j$(nproc)
sudo make install
```

#### New cmake versions

```bash
./build "" -j$(nproc)
sudo make install
```
