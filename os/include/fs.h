/* ---------------------------------------------------------------- */
/* - fs.h: Filesystem declarations                                - */
/* ---------------------------------------------------------------- */

#ifndef _FS_H
#define _FS_H

void *fs_lookup(unsigned char *name);
int fs_dirindex(char *name, int index);
int fs_dirlength(void);

#endif


