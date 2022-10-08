#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "util.h"


int exist(char *filepath)
{
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

int get_CNF_string( char **CNF_p_p,
                    char **name_p_p,
                    char **value_p_p)
{
	 char *np, *vp, *tp = *CNF_p_p;

start_line:
	while ((*tp <= ' ') && (*tp > '\0'))
		tp += 1;  //Skip leading whitespace, if any
	if (*tp == '\0')
		return 0;   //but exit at EOF
	np = tp;        //Current pos is potential name
	if (*tp < 'A')  //but may be a comment line
	{               //We must skip a comment line
		while ((*tp != '\r') && (*tp != '\n') && (*tp > '\0'))
			tp += 1;      //Seek line end
		goto start_line;  //Go back to try next line
	}

	while ((*tp >= 'A') || ((*tp >= '0') && (*tp <= '9')))
		tp += 1;  //Seek name end
	if (*tp == '\0')
		return 0;  //but exit at EOF

	while ((*tp <= ' ') && (*tp > '\0'))
		*tp++ = '\0';  //zero&skip post-name whitespace
	if (*tp != '=')
		return 0;  //exit (syntax error) if '=' missing
	*tp++ = '\0';  //zero '=' (possibly terminating name)

	while ((*tp <= ' ') && (*tp > '\0')       //Skip pre-value whitespace, if any
	       && (*tp != '\r') && (*tp != '\n')  //but do not pass the end of the line
	       && (*tp != '\7')                   //allow ctrl-G (BEL) in value
	)
		tp += 1;
	if (*tp == '\0')
		return 0;  //but exit at EOF
	vp = tp;       //Current pos is potential value

	while ((*tp != '\r') && (*tp != '\n') && (*tp != '\0'))
		tp += 1;  //Seek line end
	if (*tp != '\0')
		*tp++ = '\0';  //terminate value (passing if not EOF)
	while ((*tp <= ' ') && (*tp > '\0'))
		tp += 1;  //Skip following whitespace, if any

	*CNF_p_p = tp;    //return new CNF file position
	*name_p_p = np;   //return found variable name
	*value_p_p = vp;  //return found variable value
	return 1;         //return control to caller
}  //Ends get_CNF_string