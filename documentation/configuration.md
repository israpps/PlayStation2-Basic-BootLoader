---
sort: 1
---

# Configuration

here you will learn all the configurable aspects of PS2BBL.

PS2BBL looks for it's config file on the following paths, on this specific order:

```
./CONFIG.INI

mx4sio:/PS2BBL/CONFIG.INI

hdd0:/__sysconf/PS2BBL/CONFIG.INI

mass:/PS2BBL/CONFIG.INI

mc0:/SYS-CONF/CONFIG.INI

mc1:/SYS-CONF/CONFIG.INI
```
> note that hdd and mx4sio paths are only sesrched if you use a version of PS2BBL compatible with those devices

### PSX Configuration

Users of PSX (`DESR` models) will be capable of using a PSX-dedicated configuration file wich will be looked for in this path:
```
mc?:/SYS-CONF/XCONFIG.INI
```
this config file has level 2 priority. this means that it will be always prioritized unless you have a dedicated config file on the japanese system update folder (`mc?:/BIEXEC-SYSTEM/CONFIG.INI`)

## Launch keys

a launch key is a configuration entry wich defines the path to a program or command to be executed when a specific key is pressed. for each key there are `3` paths wich are searched for in order

the sintax for a launch key is:

```
LK_@_E#
```

where `@` is the KEY (`AUTO`, `SELECT`, `L3`, `R3`, `START`, `UP`, `RIGHT`, `DOWN`, `LEFT`, `L2`, `R2`, `L1`, `R1`, `TRIANGLE`, `CIRCLE`, `CROSS`, `SQUARE`) all of them refer to a button of the joystick excluding `AUTO`

and `#` is a number from `1` to `3`

example:
- `LK_SELECT_E1`: first application to be looked for if SELECT is pressed
- `LK_SELECT_E2`: apllication to be looked for if previous one was not found
- `LK_SELECT_E3`: last apllication to be looked for if previous one was not found

```tip
when adding multiple paths, set the first path on USB or other removable media, and second or third path on a storage device that is always there (HDD, MC, ROM, etc..)
This way you can setup a default version of the application and at the same time provide updated versions on the USB (so you can fall back to the old one by just removing the USB)
```


## Config Parameters

the program supports the following config entries

- `SKIP_PS2LOGO`: wether to run ps2 games via `rom0:PS2LOGO` or by running the game executable itself
- `KEY_READ_WAIT_TIME`: time (in miliseconds) that the program should wait for a key press before arbitrarly loading the `AUTO` launch keys
- `OSDHISTORY_READ`: wether to change or not the program logo color (color is calculated with OSD play history record from memory card)
- `EJECT_TRAY`: wether to eject the console tray or not the first time the disc handler detects the tray is empty
- `LOGO_DISPLAY`: config for logo Display:
  + `0`: don't display logo and console info
  + `1`: display console info
  + `2`: display logo and console info
- `LOAD_IRX_E#`: defines a path for an IRX file to be uploaded into the I/O CPU. `#` means any number.


## supported commands

- `$CDVD`: execute disc respecting the value passed to `SKIP_PS2LOGO` on config
- `$CDVD_NO_PS2LOGO`: execute disc enforcing the `SKIP_PS2LOGO` to `1`
- `$CREDITS`: displays credits (duh). and shows compilation date and associated git commit hash
- `$RUNKELF:KELFPATH`: executes the file path specified in `KELFPATH` as a PS2 KELF
- `$OSDSYS`: Executes the OSDSYS program with special argumments to skip Memory card & HDD updates, and goes straight to mc browser. this is very usefull for users wich install PS2BBL as system update and wants to access the memory card browser of OSDSYS (__WARNING__: USELESS ON PSX-DESR Because it's OSDSYS has no menu)
- `$HDDCHECKER`: **Only available when Internal HDD support is enabled**. it performs the same integrity checks than FreeHdBoot/HDD-OSD MBR Programs (S.M.A.R.T status, sector errors, damaged partitions and HDD connection status)

## configuration sample

```ini
# PlayStation2 Basic Bootloader config file
# configurations:
SKIP_PS2LOGO = 0
EJECT_TRAY = 1
OSDHISTORY_READ = 1
KEY_READ_WAIT_TIME = 4000
LOGO_DISPLAY = 2
LOAD_IRX_E1 = mass:/PS2BBL/EXAMPLE.IRX
# applications:
LK_AUTO_E1 = mc?:/APPS/OPNPS2LD.ELF
LK_AUTO_E2 = mc?:/OPL/OPNPS2LD.ELF
LK_AUTO_E3 = mass:/OPNPS2LD.ELF
LK_L1_E1 = mc?:/BOOT/ULE.ELF
LK_L1_E2 = mc?:/APPS/ULE.ELF
LK_L1_E3 = mc:?/BOOT/BOOT.ELF
LK_SELECT_E1 = rom0:TESTMODE
LK_START_E1 = $OSDSYS
LK_CROSS_E1 = $CDVD
LK_TRIANGLE_E1 = $CDVD_NO_PS2LOGO
```

## largest config file
The following example file shows all the possible entries that the program can use:

```ini
# PlayStation2 Basic Bootloader config file
# configurations:
EJECT_TRAY = 1
OSDHISTORY_READ = 1
KEY_READ_WAIT_TIME = 4000
SKIP_PS2LOGO = 1
LOGO_DISPLAY = 2

LK_AUTO_E1 = ...
LK_AUTO_E2 = ...
LK_AUTO_E3 = ...
LK_SELECT_E1 = ...
LK_SELECT_E2 = ...
LK_SELECT_E3 = ...
LK_L3_E1 = ...
LK_L3_E2 = ...
LK_L3_E3 = ...
LK_R3_E1 = ...
LK_R3_E2 = ...
LK_R3_E3 = ...
LK_START_E1 = ...
LK_START_E2 = ...
LK_START_E3 = ...
LK_UP_E1 = ...
LK_UP_E2 = ...
LK_UP_E3 = ...
LK_RIGHT_E1 = ...
LK_RIGHT_E2 = ...
LK_RIGHT_E3 = ...
LK_DOWN_E1 = ...
LK_DOWN_E2 = ...
LK_DOWN_E3 = ...
LK_LEFT_E1 = ...
LK_LEFT_E2 = ...
LK_LEFT_E3 = ...
LK_L2_E1 = ...
LK_L2_E2 = ...
LK_L2_E3 = ...
LK_R2_E1 = ...
LK_R2_E2 = ...
LK_R2_E3 = ...
LK_L1_E1 = ...
LK_L1_E2 = ...
LK_L1_E3 = ...
LK_R1_E1 = ...
LK_R1_E2 = ...
LK_R1_E3 = ...
LK_TRIANGLE_E1 = ...
LK_TRIANGLE_E2 = ...
LK_TRIANGLE_E3 = ...
LK_CIRCLE_E1 = ...
LK_CIRCLE_E2 = ...
LK_CIRCLE_E3 = ...
LK_CROSS_E1 = ...
LK_CROSS_E2 = ...
LK_CROSS_E3 = ...
LK_SQUARE_E1 = ...
LK_SQUARE_E2 = ...
LK_SQUARE_E3 = ...
```
