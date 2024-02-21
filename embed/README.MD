# embedded files folder

- `ioprp.img`: IOPRP image with custom `IOPBTCONF` (_IOP BooT CONFiguration file_), crafted specifically to make the PSX-DESR load `rom0:PCDVDMAN` and `rom0:PCDVDFS`. wich is used by PS2BBL at startup to enforce disc reader into PS2 mode [^IOPBOOTCONF]
- `ioman_ioprp.img`: IOPRP image containing homebrew replacements for the FILEIO and IOMAN drivers. used for COH models compatibility
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
