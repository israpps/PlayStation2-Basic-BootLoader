---
sort: 1
---

# Configuration

here you will learn all the configurable aspects of PS2BBL.

PS2BBL looks for it's config file on the following paths, on this specific order:

```
mass:/PS2BBL/CONFIG.INI

mc0:/PS2BBL/CONFIG.INI

mc1:/PS2BBL/CONFIG.INI
```

## Launch keys

a launch key is a configuration entry wich defines the path to a program or command to be executed when a specific key is pressed. for each key there are `3` paths wich are searched for in order

the sintax for a launch key is:

```
LK_@_E#
```

where `@` is the KEY (`AUTO`, `SELECT`, `L3`, `R3`, `START`, `UP`, `RIGHT`, `DOWN`, `LEFT`, `L2`, `R2`, `L1`, `R1`, `TRIANGLE`, `CIRCLE`, `CROSS`, `SQUARE`) all of them refer to a button of the joystick excluding `AUTO`

and `#` is a number from `1` to `3`

example:
```
LK_SELECT_E1
```
is the first path to look for if `SELECT` is pressed

## Config Parameters

the program supports the following config entries

- `SKIP_PS2LOGO`: wether to run ps2 games via `rom0:PS2LOGO` or by running the game executable itself
- `KEY_READ_WAIT_TIME`: time (in miliseconds) that the program should wait for a key press before arbitrarly loading the `AUTO` launch keys
- `OSDHISTORY_READ`: wether to change or not the program logo color (color is calculated with OSD play history record from memory card)
- `EJECT_TRAY`: wether to eject the console tray or not the first time the disc handler detects the tray is empty
## supported commands

- `$CDVD`: execute disc respecting the value passed to `SKIP_PS2LOGO` on config
- `$CDVD_NO_PS2LOGO`: execute disc enforcing the `SKIP_PS2LOGO` to `1`
- `$CREDITS`: displays credits (duh). and shows compilation date and associated git commit hash
- `$RUNKELF:KELFPATH`: executes the file path specified in `KELFPATH` as a PS2 KELF
- `$OSDSYS`: Executes the OSDSYS program with special argumments to skip Memory card & HDD updates, and goes straight to mc browser. this is very usefull for users wich install PS2BBL as system update and wants to access the memory card browser of OSDSYS (__WARNING__: USELESS ON PSX-DESR Because it's OSDSYS has no menu)


## configuration sample

```ini
# PlayStation2 Basic Bootloader config file
# configurations:
SKIP_PS2LOGO = 0
EJECT_TRAY = 1
OSDHISTORY_READ = 1
KEY_READ_WAIT_TIME = 4000
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
