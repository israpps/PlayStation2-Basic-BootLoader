---
sort: 2
---

# Features

All that PS2BBL has to offer

```
  ~ WORK IN PROGRESS ON THIS PAGE ~
```


## Emergency mode

If something breaks on your setup but PS2BBL still boots, just hold R1+START.
It will trigger emergency mode

where PS2BBL will try to boot `RESCUE.ELF` from USB device Root.


## Properly sistem initialization

### For PS2

- OSD, OSD settings and some extra facilities are loaded.
- All modules listed on default IOP Boot configuration are loaded on startup.
- CDVD boot certification is properly performed
- Remote control will be enabled if possible
- OSD Initialization is done in a way the Kernel Patches for `SCPH-10000` and `SCPH-15000` take effect

### For PSX

- Memory mode is set to 32mb limit, as it's described to be the best method for running homebrew (IOP remains using it's juicy 8mb)

- Special PSX disc drive modules are loaded (`PCDVDMAN` & `PCDVDFS`) so the bootloader can set the disc reader to PS2 mode

## Running homebrew

Unsigned executables can be freely loaded from memory card, USB (both FAT32 and EXFAT) and even on-board software can be loaded (`rom0`)

## Running Discs

PS2BBL can run PS1, PS2 and DVD Discs

### PS2 discs

PS2 discs can be loaded with or without PS2LOGO via configuration or special commands _(Insert happy moment for mechaPWN users)_

## Space usage

PS2BBL has an aproximated size of 112kb
Wich means a system update setup that covers all models compatbible with system updates will take aproximately 1mb

## Embedded USB drivers

Unlike FreeMcBoot, PS2BBL has USB drivers embedded in binary, lowering your chances of loosing access to homebrew in case of data loss.

and to make things better...
the impact on program size was just ~`3kb`!
