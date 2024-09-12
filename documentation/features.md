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

where PS2BBL will try to boot `RESCUE.ELF` from USB device Root on an endless loop.

## Compatibility 



| Model | Status | Notes |
| ----- | ------ | ----- |
| SCPH-10000 (1.00 boot rom) | WORKING | Needs kernel patch update |
| SCPH-10000 (1.01 boot rom) | WORKING | Needs kernel patch update |
| SCPH-15000 | WORKING ||
| SCPH-18000 | WORKING ||
| SCPH-30xxx | WORKING ||
| SCPH-30xxxR| WORKING ||
| SCPH-35xxx | WORKING ||
| SCPH-37000 | WORKING ||
| SCPH-390xx | WORKING ||
| SCPH-500xx | WORKING ||
| SCPH-550xx | WORKING ||
| SCPH-70xxx | WORKING ||
| SCPH-750xx | WORKING ||
| SCPH-770xx | WORKING ||
| SCPH-790xx | WORKING ||
| SCPH-900xx (2.20 boot rom) | WORKING ||
| SCPH-900xx (2.30 boot rom) | WORKING | will work, but you cannot auto boot as system update |
| PX-300 (PS2TV) | NOT CONFIRMED | should work, but you cannot auto boot as system update |
| DESR-xxxx (PSX) | WORKING | |
| DTL-H10000 | UNKNOWN | should work, but this model is invulnerable to every digital exploit |
| DTL-H301xx | WORKING | |
| Namco system 246 (COH-Hxxxxx) | **IN PROGRESS** || 
| Namco system 256 (COH-Hxxxxx) | **IN PROGRESS** || 
| Konami python | UNKNOWN || 
| DTL-T10000 (TOOL) | UNKNOWN || 
| DTL-T15000 (TOOL w/ perf analyzer)  | UNKNOWN || 


## Proper sistem initialization

### For all systems

- OSD, OSD settings and some extra facilities are loaded.
- All modules listed on default IOP Boot configuration are loaded on startup.
- CDVD boot certification is properly performed
- Remote control will be enabled if possible
- OSD Initialization is done in a way the Kernel Patches for `SCPH-10000` and `SCPH-15000` take effect


### For PSX-DESR
<!---
- Memory mode is set to 32mb limit, as it's described to be the best method for running homebrew (IOP remains using it's juicy 8mb)
--->
- PSX disc tray is enforced into PS2 mode at boot. allowing the usage of PlayStation and PlayStation 2 games even if the DVR laser is not operational

## Modchip Compatibility

unlike FreeMcBoot, PS2BBL has higher compatibility with modchipped consoles because it does not patch the OSDSYS on RAM.

## Running Discs

PS2BBL can run PS1, PS2 and DVD Discs.

simply execute the `$CDVD` (or `$CDVD_NO_PS2LOGO`) commands to run a disc.

It does not matter if you insert the disc before or after the command is executed

### PS2 discs

PS2 discs can be loaded with or without PS2LOGO via configuration or special commands _(Insert happy moment for mechaPWN users)_

## Space usage

PS2BBL has an aproximated size of `80 kb`
Wich means a system update setup that covers all common PS2 models compatbible with system updates will take aproximately a little bit more than `650 kb` (add `80 kb` to the count if you want to add PSX-DESR system update)

## Embedded USB drivers

Unlike FreeMcBoot, PS2BBL has USB drivers embedded in binary, lowering your chances of loosing access to homebrew in case of data loss.

and to make things better...
the impact on program size was just ~`3 kb`!

## Applications Execution

Unsigned ELF files can be executed from the following devices
- `mc0:/`: Memory Card 1
- `mc1:/`: Memory Card 2
- `mc?:/`: Pseudo-device used to search on both Memory Cards ports
- `mass:/`: first compatible usb device that was mapped by the USBMASS driver
- `massX:/`: MX4SIO SD Card
- `rom0:`: console main ROM memory. holds software (such as `OSDSYS` and `TESTMODE`), system information, configurations and lots of IRX modules.
- `hdd0:PARTITION:pfs:PATH_TO_ELF`: PFS partition of internal HDD


## Differences with FreeMcBoot

PS2BBL | FreeMcBoot
------ | ----------
Support EXFAT and FAT32 devices | Support FAT32 devices
Open Source | Closed Source
High compatibility with modchips | Low compatibility with modchip due to `rom0:OSDSYS` manipulation
Loads apps from mc usb on the default build. but it can also support iLink and MX4SIO by using the appropiate versions | loads apps from mc and usb
Has Variants that support PSX DESR | Has Variants that support PSX DESR
Has Variants that support namco system 2x6 | does not support system 2x6
space usage of aproximately 90kb (at best) | space usage of aproximately 80kb
Provides ready to use package for Consoles that need OpenTuna | Fusing it with OpenTuna can be a bit complicated and leaves FreeMcBoot prone to execute itself endlessly on some situations
Loads config from usb, mc or the path where the program is located | Loads config from usb or mc
