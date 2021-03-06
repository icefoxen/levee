/*
 * LEVEE, or Captain Video;  A vi clone
 *
 * Copyright (c) 1982-2019 David L Parsons
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, without or
 * without modification, are permitted provided that the above
 * copyright notice and this paragraph are duplicated in all such
 * forms and that any documentation, advertising materials, and
 * other materials related to such distribution and use acknowledge
 * that the software was developed by David L Parsons (orc@pell.portland.or.us).
 * My name may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */
#include "levee.h"
#include "extern.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/*
 * look up a tag
 */
int
find_tag(char *tag, int sztag, Tag *ret)
{
    FILE *tags = fopen("tags", "r");
    static char tagline[120];
    char *filename;
    char *pattern;
    int count=0;

    if ( tags == 0 ) {
	errno = EBADF;
	return 0;
    }

    while ( fgets(tagline, sizeof tagline, tags) ) {
	if ( memcmp(tagline, tag, sztag) == 0 && tagline[sztag] == '\t' ) {
	    filename = strtok(&tagline[sztag+1], "\t");
	    pattern = strtok(NULL, "\n");

	    fclose(tags);

	    if ( filename && pattern ) {
		ret->filename = filename;
		ret->pattern = pattern;
		return 1;
	    }
	    errno = EINVAL;
	    return 0;
	}
    }
    fclose(tags);
    errno = ENOENT;
    return 0;
}


int
gototag(int fileptr, char *pattern)
{
    int samefile = (fileptr == filenm);
    int wasmagic = magic;

    extern int low;	/* from editcor.c, which is where findbounds lives */

    magic = 0;
    if ( samefile ) {
	findbounds(pattern);
	if ( low < 0 ) {
	    magic = wasmagic;
	    return EOF;
	}
	curr = low;
    }
    else if ( fileptr != F_UNSET ) {
	startcmd = pattern;
	doinput(fileptr);
    }
    magic = wasmagic;
    return samefile ? SAMEFILE : DIFFERENTFILE;
}


#define NR_CAMEFROM 20
static Camefrom tagstack[NR_CAMEFROM];
static int tag_ptr = 0;


void
push_tag(int filenm, int curr)
{
    if ( filenm == F_UNSET )
	return;

    if ( tag_ptr >= NR_CAMEFROM )
	moveleft((void*)&tagstack[1],
		 (void*)&tagstack[0],
		 (tag_ptr-1) * sizeof tagstack[0]);

    tagstack[tag_ptr].fileno = filenm;
    tagstack[tag_ptr].cursor = curr;

    if ( tag_ptr < NR_CAMEFROM )
	++tag_ptr;
}

Camefrom *
pop_tag()
{
    if ( tag_ptr <= 0 )
	return 0;

    return &tagstack[--tag_ptr];
}

void
zero_tagstack()
{
    tag_ptr = 0;
}
