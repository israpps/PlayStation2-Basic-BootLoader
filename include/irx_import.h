#ifndef IRX_IMPORTS
#define IRX_IMPORTS


#define IMPORT_BIN2C(_n)       \
    extern unsigned char _n[]; \
    extern unsigned int size_##_n


// --------------- IRX/IOPRP extern --------------- //
IMPORT_BIN2C(sio2man_irx);
IMPORT_BIN2C(mcman_irx);
IMPORT_BIN2C(mcserv_irx);
IMPORT_BIN2C(padman_irx);

#ifdef PSX
IMPORT_BIN2C(psx_ioprp);
#endif

#ifdef FILEXIO
IMPORT_BIN2C(iomanX_irx);
IMPORT_BIN2C(fileXio_irx);
#endif

#ifdef HDD
IMPORT_BIN2C(poweroff_irx);
IMPORT_BIN2C(ps2atad_irx);
IMPORT_BIN2C(ps2hdd_irx);
IMPORT_BIN2C(ps2fs_irx);
#endif

#ifdef UDPTTY
IMPORT_BIN2C(ps2ip_irx);
IMPORT_BIN2C(udptty_irx);
IMPORT_BIN2C(netman_irx);
IMPORT_BIN2C(smap_irx);
#endif

#ifdef MX4SIO
IMPORT_BIN2C(mx4sio_bd_irx);
#ifdef USE_ROM_SIO2MAN
#error MX4SIO needs Homebrew SIO2MAN to work
#endif
#endif

#ifdef DEV9
IMPORT_BIN2C(ps2dev9_irx);
#endif

#ifdef HAS_EMBEDDED_IRX
IMPORT_BIN2C(usbd_irx);
#ifdef NO_BDM
IMPORT_BIN2C(usb_mass_irx);
#else
IMPORT_BIN2C(bdm_irx);
IMPORT_BIN2C(bdmfs_fatfs_irx);
IMPORT_BIN2C(usbmass_bd_irx);
#endif // NO_BDM
#endif // HAS_EMBEDDED_IRX

#endif