# NOTICE

We regret to inform our users that the Guayadeque project has reached the end of its development journey and is no longer actively maintained.\
After years of dedicated work by the open-source community, this beloved music player and library organizer will no longer receive updates or support.\
We want to extend our gratitude to all the contributors and users who made Guayadeque a part of their music experience.\
While it may no longer be actively developed, we hope that it continues to serve its purpose for those who choose to use it.\
Thank you for your support throughout the years.

If anyone wishes to continue the development and support of Guayadeque, please feel free to contact me.

# General

Guayadeque Music Player 0.5.0\
Juan Rios anonbeat@gmail.com\
see LICENSE

Please email with bugs, suggestions, requests, translations to anonbeat@gmail.com\
or post them in our forums http://guayadeque.org

Special Thanks to Mrmotinjo (Stefan Bogdanovic http://evilsun.carbonmade.com)\
for the icon and splash designed for guayadeque.

---

# Build

Need installed cmake, wxWidgets 3.0, gstreamer1.0, sqlite3, libwxsqlite3, taglib, libcurl, libdbus-1, libgio

It's been developed in XUbuntu.

---

## Dependencies

### Ubuntu (pre 20.0):

```bash
sudo apt install libgdk-pixbuf2.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.0-dev libwxbase3.0-dev libtag-extras-dev cmake binutils
```

---

### Ubuntu 20.04, Linux Mint 20

```bash
sudo apt install libgdk-pixbuf2.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.0-dev libwxbase3.0-dev libtag1-dev libtag-extras-dev libcurl4-gnutls-dev libdbus-1-dev libjsoncpp-dev cmake binutils
```

---

### Ubuntu 22.04, Linux Mint 21

```bash
sudo apt install libgdk-pixbuf2.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.0-dev libwxbase3.0-dev libtag1-dev libtag-extras-dev libcurl4-gnutls-dev libdbus-1-dev libjsoncpp-dev gettext cmake binutils
```

---

### Ubuntu 24.04, Linux Mint 22

```bash
sudo apt install libgdk-pixbuf2.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.2-dev libtag1-dev libcurl4-gnutls-dev libdbus-1-dev libjsoncpp-dev gettext cmake binutils
```

---

### Mageia 9

```
sudo urpmi gstreamer1.0-devtools lib64wx_gtk3u_wxsqlite3_3.2-devel lib64taglib-devel lib64sqlite3-devel lib64curl-devel lib64dbus-devel lib64gio2.0_0 lib64jsoncpp-devel cmake binutils
```

---

### Arch Linux

```bash
sudo pacman -S wxgtk3 gstreamer gst-plugins-base gst-plugins-good sqlite wxsqlite3 taglib curl dbus gdk-pixbuf2 jsoncpp cmake
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

##### Ubuntu, Linux Mint

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

---

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

---
