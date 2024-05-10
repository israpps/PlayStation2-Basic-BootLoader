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

✔️: Works  
❓: not confirmed  
❔: not explicitly confirmed but SHOULD work  
❌: Incompatible    
⚙️: In development


| Model | Status | Notes |
| ----- | ------ | ----- |
| SCPH-10000 (1.00 boot rom) | ✔️ | Needs kernel patch update |
| SCPH-10000 (1.01 boot rom) | ✔️ | Needs kernel patch update |
| SCPH-15000 | ✔️ ||
| SCPH-18000 | ✔️ ||
| SCPH-30xxx | ✔️ ||
| SCPH-30xxxR| ✔️ ||
| SCPH-35xxx | ✔️ ||
| SCPH-37000 | ✔️ ||
| SCPH-390xx | ✔️ ||
| SCPH-500xx | ✔️ ||
| SCPH-550xx | ✔️ ||
| SCPH-70xxx | ✔️ ||
| SCPH-750xx | ✔️ ||
| SCPH-770xx | ✔️ ||
| SCPH-790xx | ✔️ ||
| SCPH-900xx (2.20 boot rom) | ✔️ ||
| SCPH-900xx (2.30 boot rom) | ✔️ | will work, but you cannot auto boot as system update |
| PX-300 (PS2TV) | ❔ | should work, but you cannot auto boot as system update |
| DESR-xxxx (PSX) | ✔️ | |
| DTL-H10000 | ❓ | should work, but this model is invulnerable to every digital exploit |
| DTL-Hxxxxx | ❔ | |
| Namco system 246 (COH-Hxxxxx) | ⚙️ || 
| Namco system 256 (COH-Hxxxxx) | ⚙️ || 
| Konami python | ❓ || 
| DTL-T10000 (TOOL) | ❓ || 
| DTL-T15000 (TOOL w/ perf analyzer)  | ❓ || 


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
