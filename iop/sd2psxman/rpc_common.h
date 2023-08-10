#ifndef SD2PSXMAN_RPC_COMMONDEF
#define SD2PSXMAN_RPC_COMMONDEF

#define SD2PSXMAN_IRX 0xB0355 // 'S' + 'S' + '2' + 'P' + 'S' + 'X' + 'M' + 'A' + 'N' + 0xB00B5

enum PS2IPS_RPC_ID {
    SD2PSX_QUIT_BOOTCARD = 1,
    SD2PSX_SWITCH_CARD_SLOT,
    SD2PSX_SWITCH_CARD_CHANNEL,
    SD2PSX_SWITCH_CARD_FULL,
    SD2PSX_SWITCH_CARD_SLOT_BY_GAMEID,
    SD2PSX_SEND_CMD,
    SD2PSX_CMD_COUNT
};

typedef struct _sd2psxman_rpc_pkt
{
    char GAMEID[20+1]; // ELF ID, used to switch VMC based on this
    int cardID;
    int channelID;
    int SD2PSX_Slot;
    int SD2PSX_Port;
}sd2psxman_rpc_data_t;
#define SD2PSXMAN_PKT_SIZE sizeof(sd2psxman_rpc_data_t)


#endif