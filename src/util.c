#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <tamtypes.h>
#include <loadfile.h>
#include <malloc.h>
#include <errno.h>
#include "util.h"
#include "debugprintf.h"
int exist(char *filepath)
{
    if (filepath == NULL)
        return 0;
    int fdn;

    fdn = open(filepath, O_RDONLY);
    if (fdn < 0)
        return 0;

    close(fdn);

    return 1;
}

void delay(int count)
{
    int i;
    for (i = 0; i < count; i++) {
        int ret;

        ret = 0x01000000;
        while (ret--)
            asm("nop\nnop\nnop\nnop");
    }
}

int loadIRXFile(char *path, u32 arg_len, const char *args, int *mod_res)
{
    FILE *fp;
    unsigned char *IRX;
    int IRX_SIZE, RET = -1;
    if (!strncmp(path, "mc?", 3)) {
        path[2] = '0';
        if (!exist(path))
            path[2] = '1';
    }
    if (!exist(path))
    {
        DPRINTF("%s: %s NOT found\n", __func__, path);
        return -ENOENT;
    } else {
        DPRINTF("%s: %s found\n", __func__, path);
    }
    fp = fopen(path, "r");
    fseek(fp, 0, SEEK_END);
    IRX_SIZE = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    IRX = (unsigned char *)malloc(IRX_SIZE);
    if (IRX == NULL) {
        fclose(fp);
        return -ENOMEM;
    }

    if (fread(IRX, 1, IRX_SIZE, fp) == IRX_SIZE) {
        RET = SifExecModuleBuffer(IRX, IRX_SIZE, arg_len, args, mod_res);
    } else {
        RET = -EIO;
    }
    fclose(fp);
    if (IRX != NULL)
        free(IRX);
    return RET;
}

int get_CNF_string(char **CNF_p_p,
                   char **name_p_p,
                   char **value_p_p)
{
    char *np, *vp, *tp = *CNF_p_p;

start_line:
    while ((*tp <= ' ') && (*tp > '\0'))
        tp += 1; // Skip leading whitespace, if any
    if (*tp == '\0')
        return 0;  // but exit at EOF
    np = tp;       // Current pos is potential name
    if (*tp < 'A') // but may be a comment line
    {              // We must skip a comment line
        while ((*tp != '\r') && (*tp != '\n') && (*tp > '\0'))
            tp += 1;     // Seek line end
        goto start_line; // Go back to try next line
    }

    while ((*tp >= 'A') || ((*tp >= '0') && (*tp <= '9')))
        tp += 1; // Seek name end
    if (*tp == '\0')
        return 0; // but exit at EOF

    while ((*tp <= ' ') && (*tp > '\0'))
        *tp++ = '\0'; // zero&skip post-name whitespace
    if (*tp != '=')
        return 0; // exit (syntax error) if '=' missing
    *tp++ = '\0'; // zero '=' (possibly terminating name)

    while ((*tp <= ' ') && (*tp > '\0')      // Skip pre-value whitespace, if any
           && (*tp != '\r') && (*tp != '\n') // but do not pass the end of the line
           && (*tp != '\7')                  // allow ctrl-G (BEL) in value
    )
        tp += 1;
    if (*tp == '\0')
        return 0; // but exit at EOF
    vp = tp;      // Current pos is potential value

    while ((*tp != '\r') && (*tp != '\n') && (*tp != '\0'))
        tp += 1; // Seek line end
    if (*tp != '\0')
        *tp++ = '\0'; // terminate value (passing if not EOF)
    while ((*tp <= ' ') && (*tp > '\0'))
        tp += 1; // Skip following whitespace, if any

    *CNF_p_p = tp;   // return new CNF file position
    *name_p_p = np;  // return found variable name
    *value_p_p = vp; // return found variable value
    return 1;        // return control to caller
} // Ends get_CNF_string