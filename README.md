# PlayStation Basic Bootloader

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/4ea4628e3d444807bf5df8430a327c5b)](https://www.codacy.com/gh/israpps/PlayStation2-Basic-BootLoader/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=israpps/PlayStation2-Basic-BootLoader&amp;utm_campaign=Badge_Grade)
[![CI](https://github.com/israpps/PlayStation2-Basic-BootLoader/actions/workflows/CI.yml/badge.svg?branch=main)](https://github.com/israpps/PlayStation2-Basic-BootLoader/actions/workflows/CI.yml)
![GitHub all releases](https://img.shields.io/github/downloads/israpps/PlayStation2-Basic-BootLoader/total?logo=GitHub&logoColor=white)

A simple PS2 (and PSX-DESR) bootloader that handles system init and ELF programs execution (amongst other things)

## Program usage

### Installation

the program can be used as a system update or a bare ELF loaded by *tuna exploit family, modchips DEV1/INFMAN, etc.
it's totally up to you

the only thing the program needs is their config files and a bunch of IRX drivers

all of them loaded from `mc?:/PS2BBL/` 

config file should be named `CONFIG.INI` and the neede IRX files are:

- `USBD.IRX`
- `BDM.IRX`
- `BDMFS_FATFS.IRX`
- `USBMASS_BD.IRX`

### Configuration

supported commands:
- `$CDVD`: execute disc respecting the value passed to `SKIP_PS2LOGO` on config
- `$CDVD_NO_PS2LOGO`: execute disc enforcing the `SKIP_PS2LOGO` to `1`
- `$CREDITS`: displays credits (duh). and shows compilation date and associated git commit hash
- `$RUNKELF:KELFPATH`: executes the file path specified in `KELFPATH` as a PS2 KELF

#### Defining a launch key
a launch key is a configuration entry wich defines the path to a program to be executed when a specific key is pressed, for each key, there are `3` paths, wich are searched for in order

the sintax for a launch key is:
```
LK_@_E#
```

where `@` is the KEY (`AUTO`, `SELECT`, `L3`, `R3`, `START`, `UP`, `RIGHT`, `DOWN`, `LEFT`, `L2`, `R2`, `L1`, `R1`, `TRIANGLE`, `CIRCLE`, `CROSS`, `SQUARE`) all of them refer to a button of the joystick excluding `AUTO`

and `#` is a number from `1` to `3`

#### Config Parameters
the program supports the following config entries

- `SKIP_PS2LOGO`: if `0` `rom0:PS2LOGO` loads the PS2 game. if `1` the disc handler executes the game main program by itself (useful for MechaPWN)
- `KEY_READ_WAIT_TIME`: time (in miliseconds) that the program should wait for a key press before arbitrarly loading the `AUTO` launch keys

#### Example

```ini
#PlayStation2 Basic Bootloader config file
SKIP_PS2LOGO = 0
KEY_READ_WAIT_TIME = 3000
LK_AUTO_E1 = mc?:/APPS/OPNPS2LD.ELF
LK_AUTO_E2 = mc?:/OPL/OPNPS2LD.ELF
LK_AUTO_E3 = mass:/OPNPS2LD.ELF

LK_L1_E1 = mc?:/BOOT/ULE.ELF
LK_L1_E2 = mc?:/APPS/ULE.ELF
LK_L1_E3 = mc:?/BOOT/BOOT.ELF

LK_SELECT_E1 = rom0:TESTMODE
LK_START_E1 = rom0:OSDSYS

LK_CROSS_E1 = $CDVD
LK_TRIANGLE_E1 = $RUNKELF:mc?:/BAEXEC-SYSTEM/osdmain.elf
```
## Known bugs/issues

you tell me ;)

## Credits

- thanks to @SP193 for the OSD initialization libraries, wich serve as the foundation for this project
- thanks asmblur, for encouraging me to make this monster on latest sdk
- thanks to @uyjulian and @fjtrujy for always helping me
