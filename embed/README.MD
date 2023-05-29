# embedded files folder

- `ioprp.img`: IOPRP image with custom `IOPBTCONF` (__IOP__ __B__OO__T__ configuration file), crafted specifically to make the PSX-DESR load `rom0:PCDVDMAN` and `rom0:PCDVDFS`. wich is used by PS2BBL at startup to enforce disc reader into PS2 mode [^IOPBOOTCONF]
- `icons/*.sys`: the `icon.sys` files for the `B?DATA-SYSTEM`, used in this program by the OSD history updater


## IOPRP Internals
`IOPBOOTCNF`: The list of modules contained inside the IOPRP image is
```c
@800
SYSMEM
LOADCORE
EXCEPMAN
INTRMANP
INTRMANI
SSBUSC
DMACMAN
TIMEMANP
TIMEMANI
SYSCLIB
HEAPLIB
EECONF
THREADMAN
VBLANK
IOMAN
MODLOAD
ROMDRV
STDIO
SIFMAN
IGREETING
SIFCMD
REBOOT
LOADFILE
PCDVDMAN
PCDVDFSV
SIFINIT
FILEIO
SECRMAN
EESYNC
```
