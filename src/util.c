#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <tamtypes.h>
#include <loadfile.h>
#include <malloc.h>
#ifdef HDD
#include <assert.h>
#endif

#include "util.h"

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

#ifdef HDD
//By fjtrujy
char **str_split(char *a_str, const char a_delim)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (result) {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

/**
 * @brief  method returns true if it can extract needed info from path, otherwise false.
 * In case of true, it also updates mountString, mountPoint and newCWD parameters
 * It splits path by ":", and requires a minimum of 3 elements
 * Example: if path = hdd0:__common:pfs:/retroarch/ then
 * mountString = "pfs:"
 * mountPoint = "hdd0:__common"
 * newCWD = pfs:/retroarch/
 * return true
*/
int getMountInfo(char *path, char *mountString, char *mountPoint, char *newCWD)
{
    int expected_items = 4;
    int i = 0;
    char *items[expected_items];
    char **tokens = str_split(path, ':');

    if (!tokens)
        return 0;

    for (i = 0; *(tokens + i); i++) {
        if (i < expected_items) {
            items[i] = *(tokens + i);
        } else {
            free(*(tokens + i));
        }
    }

    if (i < 3)
        return 0;

    if (mountPoint != NULL)
        sprintf(mountPoint, "%s:%s", items[0], items[1]);

    if (mountString != NULL)
        sprintf(mountString, "%s:", items[2]);

    if (newCWD != NULL)
        sprintf(newCWD, "%s:%s", items[2], i > 3 ? items[3] : "");

    free(items[0]);
    free(items[1]);
    free(items[2]);

    if (i > 3)
        free(items[3]);

    return 1;
}
#endif
