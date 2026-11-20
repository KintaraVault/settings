# Settings

The System Settings application for Cutefish Desktop.

<img src="screenshots/settings.png" alt="Settings" width="937">

## Dependencies

Settings is built on [cutefish-toolkit](https://github.com/cutefishos/cutefish-toolkit)
and the clients of [cutefish-framework](https://github.com/cutefishos/cutefish-framework);
install those first.

Arch/Manjaro Dependencies:
```shell
sudo pacman -S cmake pkgconf icu libxcrypt fontconfig libxcursor libxkbcommon expat systemd-libs
```

Debian/Ubuntu Dependencies:
```shell
sudo apt install cmake pkg-config libicu-dev libcrypt-dev libfontconfig-dev libxcursor-dev libxkbcommon-dev libexpat1-dev libsystemd-dev
```

For the Printers & Scanners pane, also install `libcups` (Arch/Manjaro) or
`libcups2-dev` (Debian/Ubuntu). A running CUPS service is needed to manage printers.

## Build

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

With `BUILD_TESTING=ON`, `build/tests/ui/settings-capture <directory> [page ...]`
writes light and dark previews at 100% and 150% to an existing directory.
For example, pass `appearance` to capture only that pane.

## License

BSD-3-Clause; see [LICENSE](LICENSE).
