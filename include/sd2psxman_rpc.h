#if !defined(SD2PSXMAN_H) && defined(SD2PSX)
#define SD2PSXMAN_H


/**
 * @brief Initialize RPC Server for communication with SD2PSXMAN module
 * 
 * @return int nonzero on error
 */
int sd2psxman_init(void);


/**
 * @brief Send command to SD2PSX to unmount the boot card, and mount last used normal card
 * 
 * @param port memory card port to send the command into
 * @param slot memory card slot to send the command into
 * @return int RPC command send status (inherent to the success or not of the action requested)
 */
int sd2psx_umount_bootcard(int port, int slot);

/**
 * @brief Send command to SD2PSX to mount the specified VMC slot on the current channel
 * 
 * @param port memory card port to send the command into
 * @param slot memory card slot to send the command into
 * @param VMCslot slot number of the VMC to be used
 * @return int RPC command send status (inherent to the success or not of the action requested)
 */
int sd2psx_switch_card_slot(int port, int slot, int VMCslot);

/**
 * @brief Send command to SD2PSX to mount the specified VMC Channel on the current slot
 * 
 * @param port memory card port to send the command into
 * @param slot memory card slot to send the command into
 * @param channel channel number to be used
 * @return int RPC command send status (inherent to the success or not of the action requested)
 */
int sd2psx_switch_card_channel(int port, int slot, int channel);

/**
 * @brief Send command to SD2PSX to mount the specified VMC slot on the specified channel
 * 
 * @param port memory card port to send the command into
 * @param slot memory card slot to send the command into
 * @param VMCslot VMC Slot number to be used
 * @param channel channel number to be used
 * @return int 
 */
int sd2psx_switch_card_full(int port, int slot, int VMCslot, int channel);

/**
 * @brief Send command to SD2PSX to mount VMC based on the provided Game ID
 * 
 * @param port 
 * @param slot 
 * @param ID 
 * @return int 
 */
int sd2psx_switch_card_by_gameID(int port, int slot, const char* ID);

#endif