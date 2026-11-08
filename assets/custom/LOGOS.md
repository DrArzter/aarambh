# Logos and trademarks

Aarambh ships the logos below so that every listed system looks right in the
boot menu out of the box. They belong to their owners. They are **not**
covered by Aarambh's license (GPL-3.0-or-later), and Aarambh is not affiliated
with or endorsed by any of these projects or companies. Each logo is used only
to identify its own operating system in the menu.

| id | System | File |
|----|--------|------|
| `ubuntu` | Ubuntu | `assets/custom/ubuntu-logo.svg` |
| `debian` | Debian | `assets/custom/debian-logo.svg` |
| `linuxmint` | Linux Mint | `assets/custom/linuxmint-logo.svg` |
| `pop` | Pop!_OS | `assets/custom/pop-logo.svg` |
| `elementary` | elementary OS | `assets/custom/elementary-logo.svg` |
| `zorin` | Zorin OS | `assets/custom/zorin-logo.svg` |
| `kali` | Kali Linux | `assets/custom/kali-logo.svg` |
| `deepin` | deepin | `assets/custom/deepin-logo.svg` |
| `fedora` | Fedora | `assets/custom/fedora-logo.svg` |
| `rhel` | Red Hat Enterprise Linux | `assets/custom/rhel-logo.svg` |
| `centos` | CentOS Stream | `assets/custom/centos-logo.svg` |
| `rocky` | Rocky Linux | `assets/custom/rocky-logo.svg` |
| `opensuse-tumbleweed` | openSUSE Tumbleweed | `assets/custom/opensuse-tumbleweed-logo.svg` |
| `opensuse-leap` | openSUSE Leap | `assets/custom/opensuse-leap-logo.svg` |
| `opensuse-slowroll` | openSUSE Slowroll | `assets/custom/opensuse-slowroll-logo.svg` |
| `arch` | Arch Linux | `assets/custom/arch-logo.svg` |
| `manjaro` | Manjaro | `assets/custom/manjaro-logo.svg` |
| `endeavouros` | EndeavourOS | `assets/custom/endeavouros-logo.svg` |
| `garuda` | Garuda Linux | `assets/custom/garuda-logo.svg` |
| `cachyos` | CachyOS | `assets/custom/cachyos-logo.svg` |
| `artix` | Artix Linux | `assets/custom/artix-logo.png` |
| `gentoo` | Gentoo | `assets/custom/gentoo-logo.svg` |
| `void` | Void Linux | `assets/custom/void-logo.svg` |
| `nixos` | NixOS | `assets/custom/nixos-logo.svg` |
| `windows` | Windows | `assets/custom/windows-logo.svg` |
| `linux` | Linux | `assets/custom/linux-logo.svg` |

`scripts/make-assets.sh` turns each file into a white mask (`logos/<id>.png`)
and draws the wallpapers and Plymouth images from it. The Firmware, Restart
and Power Off icons are drawn by the script itself and are part of Aarambh.

## Trademarks

Linux is the registered trademark of Linus Torvalds. Ubuntu is a trademark of
Canonical Ltd. Fedora and Red Hat Enterprise Linux are trademarks of Red Hat,
Inc. Windows is a trademark of Microsoft Corporation. Debian is a trademark of
Software in the Public Interest, Inc. openSUSE is a trademark of SUSE LLC. All
other names and logos are the property of their respective owners.

The Tux penguin (`linux-logo.svg`) was originally drawn by Larry Ewing
(lewing@isc.tamu.edu) using The GIMP.

## For owners

If you own one of these marks and want it changed or removed, please open an
issue and it will be taken care of.

## Adding another system

1. Save its logo as `assets/custom/<id>-logo.svg` (or `.png`), where `<id>` is
   the `ID=` from `/etc/os-release`.
2. Add a line for it to `assets/distros.conf`.
3. Run `make assets` and `make install`.

Use the logo from the project's official brand or press page where one exists,
and follow its usage guidelines. A one-color logo on a transparent background
works best.