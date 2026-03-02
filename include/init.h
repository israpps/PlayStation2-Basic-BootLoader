#ifndef INIT_H
#define INIT_H

#ifdef UDPTTY
extern inline void udptty_start();
#define UDPTTY_STARTUP() udptty_start()
#else
#define UDPTTY_STARTUP()
#endif

#ifdef FILEXIO
extern inline void load_filexio();
#define FILEXIO_STARTUP() load_filexio()
#else
#define FILEXIO_STARTUP()
#endif

extern inline void bdm_usb();
#define BDM_USB_STARTUP() bdm_usb()
#define SIO2_MC_PAD_STARTUP() iop_init_sio2_related()
// general init stuff

void init_osd_generic();
void init_iop_patches();
void CDVDBootCertify(u8 romver[16]);
void InitPSX();

// IOP Related stuff

int LoadUSBIRX(void);
void iop_init_sio2_related();
#define BGR_BLUE  0xFF0000
#define BGR_GREEN 0x00FF00
#define BGR_RED   0x0000FF
#endif
