Cowprobe-rp : Firmware
======================

This folder contains the code of the main cowprbe-rp firmware. The current
(initial) version is very basic with only serial port available (based on
the code of another project: cowprobe).

To compile this firmware, the lastest stable version of RP2040 SDK (1.4.0) must
be installed first (see raspberry-pi website). The compiler used for development
is GCC 10.3.

Features and TODO
-----------------

- [x] Virtual communication port (usb-cdc) bridge to physical UART
- [x] CMSIS-DAP interface to SWD target
  - [x] USB interface (WiP)
  - [ ] DAP commands and data transfer
- [x] Configure IOs
  - [ ] Mode SWD
  - [ ] Mode JTAG
- [ ] Network
- [ ] Plugins

License
-------

This firmware is free software. You can use it (and modify, and redistribute)
under terms of the GNU General Public License version 3 (GPL-v3).
See [LICENSE.md](LICENSE.md).
