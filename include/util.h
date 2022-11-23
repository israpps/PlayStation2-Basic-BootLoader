/**
 * @brief checks for file existence
 * @param filepath path to the file to check
 * @returns 1 if exists, 0 if not
*/
int exist(char *filepath);

/**
 * @brief waits a specified ammount of time, usually used to provide the famous 'USB delay', wLaunchELF uses 3, KELFBinder used 5?
*/
void delay(int count);

/**
 * @brief load an IRX file into memory from a file
 * @param path path to the IRX
 * @param arg_len argc for IRX
 * @param args argv[][] for IRX
 * @param mod_res see SifExecModuleBuffer documentation on SDK for this
 * @returns nonzero on failure, check SifExecModuleBuffer documentation on SDK
*/
int loadIRXFile(char *path, u32 arg_len, const char *args, int *mod_res);

int get_CNF_string(char **CNF_p_p,
                   char **name_p_p,
                   char **value_p_p);