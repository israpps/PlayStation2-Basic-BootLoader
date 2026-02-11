// Initialize PADMAN RPC service
void PadInitPads(void);

// Deinit PADMAN RPC service
void PadDeinitPads(void);


/// Read pad data without additional checks
int ReadPadStatus_raw(int port, int slot);

/// `ReadPadStatus_raw()` on both ports simultaneosly
int ReadCombinedPadStatus_raw(void);

/// @brief Read pad data
/// @note this function stores the pad data from last call, will only return the pad data if it differs from old status
int ReadPadStatus(int port, int slot);

/// `ReadPadStatus()` on both ports simultaneosly
int ReadCombinedPadStatus(void);
