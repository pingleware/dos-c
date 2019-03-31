/****************************************************************/
/*                                                              */
/*                          fatfs.c                             */
/*                           DOS-C                              */
/*                                                              */
/*                 FAT File System I/O Functions                */
/*                                                              */
/*                      Copyright (c) 1995                      */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/****************************************************************/

#include "../../hdr/portab.h"
#include "globals.h"


#ifdef VERSION_STRINGS
BYTE *RcsId = "$Header:   D:/dos-c/src/fs/fatfs.c_v   1.4   29 May 1996 21:15:16   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/fs/fatfs.c_v  $
 * 
 *    Rev 1.4   29 May 1996 21:15:16   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.3   19 Feb 1996  3:20:10   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.2   01 Sep 1995 17:48:40   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:50:24   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:04:46   patv
 * Initial revision.
 */

/*                                                                      */
/*      function prototypes                                             */
/*                                                                      */
BOOL psep(COUNT);
VOID trim_path(BYTE FAR *s);
struct f_node FAR *xlt_fd(COUNT);
COUNT xlt_fnp(struct f_node FAR *);
struct f_node FAR *split_path(BYTE FAR *, BYTE *, BYTE *, BYTE *);
BOOL find_fname(struct f_node FAR *, BYTE *, BYTE *);
date dos_getdate(VOID);
time dos_gettime(VOID);
BOOL find_free(struct f_node FAR *);
UWORD find_fat_free(struct f_node FAR *);
VOID wipe_out(struct f_node FAR *);
BOOL last_link(struct f_node FAR *);
BOOL extend(struct f_node FAR *);
COUNT extend_dir(struct f_node FAR *);
BOOL first_fat(struct f_node FAR *);
COUNT map_cluster(struct f_node FAR *, COUNT);


/************************************************************************/
/*                                                                      */
/*      Internal file handlers - open, create, read, write, close, etc. */
/*                                                                      */
/************************************************************************/

/* Open a file given the path. Flags is 0 for read, 1 for write and 2   */
/* for update.                                                          */
/* Returns an integer file desriptor or a negative error code           */

COUNT 
dos_open (BYTE FAR *path, COUNT flag)
{
	REG struct f_node FAR *fnp;
	COUNT i;
	BYTE FAR *fnamep;
	BYTE dname[NAMEMAX];
	BYTE fname[FNAME_SIZE], fext[FEXT_SIZE];

	/* First test the flag to see if the user has passed a valid    */
	/* file mode...                                                 */
	if(flag < 0 || flag > 2)
		return DE_INVLDACC;

	/* first split the passed dir into comopnents (i.e. - path to   */
	/* new directory and name of new directory.                     */
	if((fnp = split_path(path, dname, fname, fext)) == NULL)
	{
		dir_close(fnp);
		return DE_PATHNOTFND;
	}

	/* Look for the file. If we can't find it, just return a not    */
	/* found error.                                                 */
	if(!find_fname(fnp, fname, fext))
	{
		dir_close(fnp);
		return DE_FILENOTFND;
	}

	/* Set the fnode to the desired mode                            */
	fnp -> f_mode = flag;

	/* Initialize the rest of the fnode.                            */
	fnp -> f_offset = 0l;
	fnp -> f_highwater = fnp -> f_dir.dir_size;

	fnp -> f_back = LONG_LAST_CLUSTER;
	fnp -> f_cluster = fnp -> f_dir.dir_start;

	fnp -> f_flags.f_dmod = FALSE;
	fnp -> f_flags.f_dnew = FALSE;
	fnp -> f_flags.f_ddir = FALSE;

	return xlt_fnp(fnp);
}


BOOL fcmp(s1, s2, n)
BYTE FAR *s1, FAR *s2;
COUNT n;
{
	while(n--)
		if(*s1++ != *s2++)
			return FALSE;
	return TRUE;
}


#ifndef IPL
BOOL fcmp_wild(s1, s2, n)
BYTE FAR *s1, FAR *s2;
COUNT n;
{
	while(n--)
	{
		if(*s1 == '?')
		{
			++s1, ++s2;
			continue;
		}
		if(*s1++ != *s2++)
			return FALSE;
	}
	return TRUE;
}
#endif


/*                                                                      */
/* Another MS-DOS wart emulation - converts ASCII lc to uc for file     */
/* name references. Should be converted to a portable version after     */
/* v1.0 is released.                                                    */
/*                                                                      */
VOID 
touc (BYTE FAR *s, COUNT n)
{
	while(n--)
	{
		if(*s >= 'a' && *s <= 'z')
			*s -= ('a' - 'A');
		++s;
	}
}


COUNT 
dos_close (COUNT fd)
{
	struct f_node FAR *fnp;

	/* Translate the fd into a useful pointer                       */
	fnp = xlt_fd(fd);

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit        */
	/* note: an invalid fd is indicated by a 0 return               */
	if(fnp == (struct f_node FAR *)0 || fnp -> f_count <= 0)
		return DE_INVLDHNDL;
	if(fnp -> f_mode != RDONLY)
	{
		fnp -> f_dir.dir_size = fnp -> f_highwater;
		fnp -> f_flags.f_dmod = TRUE;
	}
	fnp -> f_flags.f_ddir = TRUE;
	
	dir_close(fnp);
	return SUCCESS;
}


/*                                                                      */
/* split a path into it's component directory and file name             */
/*                                                                      */
static struct f_node FAR *
split_path (BYTE FAR *path, BYTE *dname, BYTE *fname, BYTE *fext)
{
	REG COUNT i;
	BYTE *fnamep;
	REG struct f_node FAR *fnp;
	struct dosnames DosName;

	/* Split the path into a drive, directory and a file name. If   */
	/* no directory is specified, use the current directory.        */
	if(DosNames(path, (struct dosnames FAR *)&DosName) != SUCCESS)
		return (struct f_node FAR *)0;

	/* If the path is null, we to default to the current            */
	/* directory...                                                 */
	sprintf(dname, "%c:%s", 'A' + DosName.dn_drive,
		 *DosName.dn_path == '\0' ? "." : DosName.dn_path);

	/* Also, set our scratch pointer to the file name so that we    */
	/* can convert it to a directory format.                        */
	fnamep = DosName.dn_name;

	/* Translate the path into a useful pointer                     */
	fnp = dir_open((BYTE FAR *)dname);

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit...     */
	/* note: an invalid fd is indicated by a 0 return               */
	if(fnp == (struct f_node FAR *)0 || fnp -> f_count <= 0)
	{
		dir_close(fnp);
		return (struct f_node FAR *)0;
	}

	/* Convert the name into an absolute name for comparison...     */

	/* first the file name with trailing spaces...                  */
	for(i = 0; i < FNAME_SIZE; i++)
	{
		if(*fnamep != '\0' && *fnamep != '.')
			fname[i] = *fnamep++;
		else
			break;
	}
	for( ; i < FNAME_SIZE; i++)
		fname[i] = ' ';

	/* and the extension (don't forget to add trailing spaces)...   */
	if(*fnamep == '.')
		++fnamep;
	for(i = 0; i < FEXT_SIZE; i++)
	{
		if(*fnamep != '\0')
			fext[i] = *fnamep++;
		else
			break;
	}
	for( ; i < FEXT_SIZE; i++)
		fext[i] = ' ';

	/* convert the names to upper case (DOS requirement)            */
	upMem((BYTE FAR *)dname, strlen(dname));
	upMem((BYTE FAR *)fname, FNAME_SIZE);
	upMem((BYTE FAR *)fext, FEXT_SIZE);

	return fnp;
}



static BOOL 
find_fname (struct f_node FAR *fnp, BYTE *fname, BYTE *fext)
{
	BOOL found = FALSE;

	while(dir_read(fnp) == DIRENT_SIZE)
	{
		if(fnp -> f_dir.dir_name[0] != '\0')
		{
			if(fnp -> f_dir.dir_name[0] == DELETED)
				continue;
			if(fcmp((BYTE FAR *)fname, (BYTE FAR *)fnp -> f_dir.dir_name, FNAME_SIZE)
			&& fcmp((BYTE FAR *)fext, (BYTE FAR *)fnp -> f_dir.dir_ext, FEXT_SIZE))
			{
				found = TRUE;
				break;
			}
		}
	}
	return found;
}


#ifndef IPL
COUNT 
dos_creat (BYTE FAR *path, COUNT attrib)
{
	REG struct f_node FAR *fnp;
	BYTE dname[NAMEMAX];
	BYTE fname[FNAME_SIZE];
	BYTE fext[FEXT_SIZE];

	/* first split the passed dir into comopnents (i.e. -   */
	/* path to new directory and name of new directory      */
	if((fnp = split_path(path, dname, fname, fext)) == NULL)
	{
		dir_close(fnp);
		return DE_PATHNOTFND;
	}

	/* Check that we don't have a duplicate name, so if we  */
	/* find one, truncate it.                               */
	if(find_fname(fnp, fname, fext))
	{
		/* The only permissable attribute is archive,   */
		/* check for any other bit set. If it is, give  */
		/* an access error.                             */
		if(fnp -> f_dir.dir_attrib & ~D_ARCHIVE)
		{
			dir_close(fnp);
			return DE_ACCESS;
		}

		/* Release the existing files FAT and set the   */
		/* length to zero, effectively truncating the   */
		/* file to zero.                                */
		wipe_out(fnp);

	}
	else
	{
		BOOL is_free;
		REG COUNT idx;
		struct buffer FAR *bp;
		BYTE FAR *p;

		/* Reset the directory by a close followed by   */
		/* an open                                      */
		fnp -> f_flags.f_dmod = FALSE;
		dir_close(fnp);
		fnp = dir_open((BYTE FAR *)dname);

		/* Get a free f_node pointer so that we can use */
		/* it in building the new file.                 */
		/* Note that if we're in the root and we don't  */
		/* find an empty slot, we need to abort.        */
		if(!(is_free = find_free(fnp)) && (fnp -> f_flags.f_droot))
		{
			fnp -> f_flags.f_dmod = FALSE;
			dir_close(fnp);
			return DE_TOOMANY;
		}

		/* Otherwise just expand the directory          */
		else if(!is_free && !(fnp -> f_flags.f_droot))
		{
			COUNT ret;

			if((ret = extend_dir(fnp)) != SUCCESS)
				return ret;
		}

		/* put the fnode's name into the directory.             */
		bcopy(fname, (BYTE *)fnp -> f_dir.dir_name, FNAME_SIZE);
		bcopy(fext, (BYTE *)fnp -> f_dir.dir_ext, FEXT_SIZE);
	}
	/* Set the fnode to the desired mode                    */
	/* Updating the directory entry first.                  */
	fnp -> f_mode = RDWR;

	fnp -> f_dir.dir_size = 0l;
	fnp -> f_dir.dir_start = FREE;
	fnp -> f_dir.dir_attrib = attrib | D_ARCHIVE;
	fnp -> f_dir.dir_time = dos_gettime();
	fnp -> f_dir.dir_date = dos_getdate();

	fnp -> f_flags.f_dmod = TRUE;
	fnp -> f_flags.f_dnew = FALSE;
	fnp -> f_flags.f_ddir = TRUE;
	if(dir_write(fnp) != DIRENT_SIZE)
	{
		release_f_node(fnp);
		return DE_ACCESS;
	}

	/* Now change to file                                   */
	fnp -> f_offset = 0l;
	fnp -> f_highwater = 0l;

	fnp -> f_back = LONG_LAST_CLUSTER;
	fnp -> f_cluster = fnp -> f_dir.dir_start = FREE;
	fnp -> f_flags.f_dmod = TRUE;
	fnp -> f_flags.f_dnew = FALSE;
	fnp -> f_flags.f_ddir = FALSE;

	return xlt_fnp(fnp);
}


COUNT 
dos_delete (BYTE FAR *path)
{
	REG struct f_node FAR *fnp;
	BYTE dir[NAMEMAX];
	BYTE fname[FNAME_SIZE];
	BYTE fext[FEXT_SIZE];

	/* first split the passed dir into comopnents (i.e. -   */
	/* path to new directory and name of new directory      */
	if((fnp = split_path(path, dir, fname, fext)) == NULL)
	{
		dir_close(fnp);
		return DE_PATHNOTFND;
	}

	/* Check that we don't have a duplicate name, so if we  */
	/* find one, it's an error.                             */
	if(find_fname(fnp, fname, fext))
	{
		/* The only permissable attribute is archive,   */
		/* check for any other bit set. If it is, give  */
		/* an access error.                             */
		if(fnp -> f_dir.dir_attrib & ~D_ARCHIVE)
		{
			dir_close(fnp);
			return DE_ACCESS;
		}

		/* Ok, so we can delete. Start out by           */
		/* clobbering all FAT entries for this file     */
		/* (or, in English, truncate the FAT).          */
		wipe_out(fnp);
		fnp -> f_dir.dir_size = 0l;
		*(fnp -> f_dir.dir_name) = DELETED;

		/* The directory has been modified, so set the  */
		/* bit before closing it, allowing it to be     */
		/* updated                                      */
		fnp -> f_flags.f_dmod = TRUE;
		dir_close(fnp);

		/* SUCCESSful completion, return it             */
		return SUCCESS;
	}
	else
	{
		/* No such file, return the error               */ 
		dir_close(fnp);
		return DE_FILENOTFND;
	}
}


COUNT 
dos_rmdir (BYTE FAR *path)
{
	REG struct f_node FAR *fnp;
	REG struct f_node FAR *fnp1;
	BYTE dir[NAMEMAX];
	BYTE fname[FNAME_SIZE];
	BYTE fext[FEXT_SIZE];
	BOOL found;

	/* first split the passed dir into comopnents (i.e. -   */
	/* path to new directory and name of new directory      */
	if((fnp = split_path(path, dir, fname, fext)) == NULL)
	{
		dir_close(fnp);
		return DE_PATHNOTFND;
	}

	/* Check that we're not trying to remove the root!      */
	if((path[0] == '\\') && (path[1] == NULL))
	{
		dir_close(fnp);
		return DE_ACCESS;
	}

	/* Check that we don't have a duplicate name, so if we  */
	/* find one, it's an error.                             */
	if(find_fname(fnp, fname, fext))
	{
		/* The only permissable attribute is directory, */
		/* check for any other bit set. If it is, give  */
		/* an access error.                             */
		if(fnp -> f_dir.dir_attrib & ~D_DIR)
		{
			dir_close(fnp);
			return DE_ACCESS;
		}

		/* Check that the directory is empty. Only the  */
		/* "." and ".." are permissable.                */
		fnp -> f_flags.f_dmod = FALSE;
		fnp1 = dir_open((BYTE FAR *)path);
		dir_read(fnp1);
		if(fnp1 -> f_dir.dir_name[0] != '.')
		{
			dir_close(fnp);
			return DE_ACCESS;
		}

		dir_read(fnp1);
		if(fnp1 -> f_dir.dir_name[0] != '.')
		{
			dir_close(fnp);
			return DE_ACCESS;
		}

		/* Now search through the directory and make certain    */
		/* that there are no entries.                           */
		found = FALSE;
		while(dir_read(fnp1) == DIRENT_SIZE)
		{
			if(fnp1 -> f_dir.dir_name[0] == '\0')
				break;
			if(fnp1 -> f_dir.dir_name[0] == DELETED)
				continue;
			else
			{
				found = TRUE;
				break;
			}
		}

		dir_close(fnp1);
		/* If anything was found, exit with an error.   */
		if(found)
		{
			dir_close(fnp);
			return DE_ACCESS;
		}

		/* Ok, so we can delete. Start out by           */
		/* clobbering all FAT entries for this file     */
		/* (or, in English, truncate the FAT).          */
		wipe_out(fnp);
		fnp -> f_dir.dir_size = 0l;
		*(fnp -> f_dir.dir_name) = DELETED;

		/* The directory has been modified, so set the  */
		/* bit before closing it, allowing it to be     */
		/* updated                                      */
		fnp -> f_flags.f_dmod = TRUE;
		dir_close(fnp);

		/* SUCCESSful completion, return it             */
		return SUCCESS;
	}
	else
	{
		/* No such file, return the error               */
		dir_close(fnp);
		return DE_FILENOTFND;
	}
}


COUNT dos_rename(path1, path2)
BYTE FAR *path1, FAR *path2;
{
	REG struct f_node FAR *fnp1;
	REG struct f_node FAR *fnp2;
	BYTE dir1[NAMEMAX];
	BYTE fname1[FNAME_SIZE];
	BYTE fext1[FEXT_SIZE];
	BYTE dir2[NAMEMAX];
	BYTE fname2[FNAME_SIZE];
	BYTE fext2[FEXT_SIZE];
	BOOL is_free;

	/* first split the passed target into compnents (i.e. - path to */
	/* new file name and name of new file name                      */
	if((fnp2 = split_path(path2, dir2, fname2, fext2)) == NULL)
	{
		dir_close(fnp2);
		return DE_PATHNOTFND;
	}

	/* Check that we don't have a duplicate name, so if we find     */
	/* one, it's an error.                                          */
	if(find_fname(fnp2, fname2, fext2))
	{
		dir_close(fnp2);
		return DE_ACCESS;
	}

	/* next split the passed source into compnents (i.e. - path to  */
	/* old file name and name of old file name                      */
	if((fnp1 = split_path(path1, dir1, fname1, fext1)) == NULL)
	{
		dir_close(fnp1);
		dir_close(fnp2);
		return DE_PATHNOTFND;
	}

	/* Reset the directory by a close followed by an open           */
	fnp2 -> f_flags.f_dmod = FALSE;
	dir_close(fnp2);
	fnp2 = dir_open((BYTE FAR *)dir2);

	/* Now find a free slot to put the file into.                   */
	/* If it's the root and we don't have room, return an error.    */
	if(!(is_free = find_free(fnp2)) && (fnp2 -> f_flags.f_droot))
	{
		fnp2 -> f_flags.f_dmod = FALSE;
		dir_close(fnp1);
		dir_close(fnp2);
		return DE_TOOMANY;
	}

	/* Otherwise just expand the directory                          */
	else if(!is_free && !(fnp2 -> f_flags.f_droot))
	{
		COUNT ret;

		if(extend_dir(fnp2) != SUCCESS)
			return ret;
	}

	if(!find_fname(fnp1, fname1, fext1))
	{
		/* No such file, return the error                       */
		dir_close(fnp1);
		dir_close(fnp2);
		return DE_FILENOTFND;
	}

	/* put the fnode's name into the directory.                     */
	bcopy(fname2, (BYTE *)fnp2 -> f_dir.dir_name, FNAME_SIZE);
	bcopy(fext2, (BYTE *)fnp2 -> f_dir.dir_ext, FEXT_SIZE);

	/* Set the fnode to the desired mode                            */
	fnp2 -> f_dir.dir_size = fnp1 -> f_dir.dir_size;
	fnp2 -> f_dir.dir_start = fnp1 -> f_dir.dir_start;
	fnp2 -> f_dir.dir_attrib = fnp1 -> f_dir.dir_attrib;
	fnp2 -> f_dir.dir_time = fnp1 -> f_dir.dir_time;
	fnp2 -> f_dir.dir_date = fnp1 -> f_dir.dir_date;

	/* The directory has been modified, so set the bit before       */
	/* closing it, allowing it to be updated.                       */
	fnp1 -> f_flags.f_dmod = fnp2 -> f_flags.f_dmod = TRUE;
	fnp1 -> f_flags.f_dnew = fnp2 -> f_flags.f_dnew = FALSE;
	fnp1 -> f_flags.f_ddir = fnp2 -> f_flags.f_ddir = TRUE;

	fnp2 -> f_highwater = fnp2 -> f_offset = fnp1 -> f_dir.dir_size;

	/* Ok, so we can delete this one. Save the file info.           */
	fnp1 -> f_dir.dir_size = 0l;
	*(fnp1 -> f_dir.dir_name) = DELETED;

	dir_close(fnp1);
	dir_close(fnp2);

	/* SUCCESSful completion, return it                             */
	return SUCCESS;
}


/*                                                              */
/* wipe out all FAT entries for create, delete, etc.            */
/*                                                              */
static VOID 
wipe_out (struct f_node FAR *fnp)
{
	REG UWORD st, next;
	struct dpb *dpbp = fnp -> f_dpb;

	/* if already free or not valid file, just exit         */
	if((fnp == NULL) || (fnp -> f_dir.dir_start == FREE))
		return;

	/* if there are no FAT entries, just exit               */
	if(fnp -> f_dir.dir_start == FREE)
		return;

	/* Loop from start until either a FREE entry is         */
	/* encountered (due to a fractured file system) of the  */
	/* last cluster is encountered.                         */
	for(st = fnp -> f_dir.dir_start ;
	  st != LONG_LAST_CLUSTER && st != LAST_CLUSTER;)
	{
		/* get the next cluster pointed to              */
		next = next_cluster(dpbp, st);

		/* just exit if a damaged file system exists    */
		if(next == FREE)
			return;

		/* zap the FAT pointed to                       */
		link_fat(dpbp, st, FREE);

		/* and the start of free space pointer          */
		if((dpbp -> dpb_cluster == UNKNCLUSTER)
		 || (dpbp -> dpb_cluster > st))
			dpbp -> dpb_cluster = st;

		/* and just follow the linked list              */
		st = next;
	}
}


static BOOL 
find_free (struct f_node FAR *fnp)
{
	while(dir_read(fnp) == DIRENT_SIZE)
	{
		if(fnp -> f_dir.dir_name[0] == '\0'
		 || fnp -> f_dir.dir_name[0] == DELETED)
		{
			return TRUE;
		}
	}
	return !fnp -> f_flags.f_dfull;
}

/*                                                              */
/* dos_getdate for the file date                                */
/*                                                              */
date dos_getdate()
{
#ifndef NOTIME
# ifndef IPL
	BYTE WeekDay, Month, MonthDay;
	COUNT Year;
	date Date;

	/* First - get the system date set by either the user   */
	/* on start-up or the CMOS clock                        */
	DosGetDate((BYTE FAR *)&WeekDay,
	 (BYTE FAR *)&Month,
	 (BYTE FAR *)&MonthDay,
	 (COUNT FAR *)& Year);
	Date = DT_ENCODE(Month, MonthDay, Year - EPOCH_YEAR);
	return Date;
# else
	return 0;
# endif
#else
	return 0;
#endif
}



/*                                                              */
/* dos_gettime for the file time                                */
/*                                                              */
time dos_gettime()
{
#ifndef NOTIME
# ifndef IPL
	BYTE Hour, Minute, Second, Hundredth;
	time Time;
	BYTE h;
	
	/* First - get the system time set by either the user   */
	/* on start-up or the CMOS clock                        */
	DosGetTime((BYTE FAR *)&Hour,
	 (BYTE FAR *)&Minute,
	 (BYTE FAR *)&Second,
	 (BYTE FAR *)&Hundredth);
	h = Second * 10 + ((Hundredth + 5) / 10);
	Time = TM_ENCODE(Hour, Minute, h);
	return Time;
# else
	return 0;
# endif
#else
	return 0;
#endif
}


#ifndef IPL
/*                                                              */
/* dos_getftime for the file time                               */
/*                                                              */
BOOL dos_getftime(fd, dp, tp)
COUNT fd;
date FAR *dp;
time FAR *tp;
{
	struct f_node FAR *fnp;

	/* Translate the fd into an fnode pointer, since all internal   */
	/* operations are achieved through fnodes.                      */
	fnp = xlt_fd(fd);

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit        */
	/* note: an invalid fd is indicated by a 0 return               */
	if(fnp == (struct f_node FAR *)0 || fnp -> f_count <= 0)
		return FALSE;

	/* Get the date and time from the fnode and return              */
	*dp = fnp -> f_dir.dir_date;
	*tp = fnp -> f_dir.dir_time;

	return TRUE;
}


/*                                                              */
/* dos_setftime for the file time                               */
/*                                                              */
BOOL dos_setftime(fd, dp, tp)
COUNT fd;
date FAR *dp;
time FAR *tp;
{
	struct f_node FAR *fnp;

	/* Translate the fd into an fnode pointer, since all internal   */
	/* operations are achieved through fnodes.                      */
	fnp = xlt_fd(fd);

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit        */
	/* note: an invalid fd is indicated by a 0 return               */
	if(fnp == (struct f_node FAR *)0 || fnp -> f_count <= 0)
		return FALSE;

	/* Set the date and time from the fnode and return              */
	fnp -> f_dir.dir_date = *dp;
	fnp -> f_dir.dir_time = *tp;

	return TRUE;
}


/*                                                              */
/* dos_getfsize for the file time                               */
/*                                                              */
LONG 
dos_getcufsize (COUNT fd)
{
	struct f_node FAR *fnp;

	/* Translate the fd into an fnode pointer, since all internal   */
	/* operations are achieved through fnodes.                      */
	fnp = xlt_fd(fd);

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit        */
	/* note: an invalid fd is indicated by a 0 return               */
	if(fnp == (struct f_node FAR *)0 || fnp -> f_count <= 0)
		return -1l;

	/* Return the file size                                         */
	return fnp -> f_highwater;
}


/*                                                              */
/* dos_getfsize for the file time                               */
/*                                                              */
LONG 
dos_getfsize (COUNT fd)
{
	struct f_node FAR *fnp;

	/* Translate the fd into an fnode pointer, since all internal   */
	/* operations are achieved through fnodes.                      */
	fnp = xlt_fd(fd);

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit        */
	/* note: an invalid fd is indicated by a 0 return               */
	if(fnp == (struct f_node FAR *)0 || fnp -> f_count <= 0)
		return -1l;

	/* Return the file size                                         */
	return fnp -> f_dir.dir_size;
}


/*                                                              */
/* dos_setfsize for the file time                               */
/*                                                              */
BOOL 
dos_setfsize (COUNT fd, LONG size)
{
	struct f_node FAR *fnp;

	/* Translate the fd into an fnode pointer, since all internal   */
	/* operations are achieved through fnodes.                      */
	fnp = xlt_fd(fd);

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit        */
	/* note: an invalid fd is indicated by a 0 return               */
	if(fnp == (struct f_node FAR *)0 || fnp -> f_count <= 0)
		return FALSE;

	/* Change the file size                                         */
	fnp -> f_dir.dir_size = size;
	fnp -> f_highwater = size;
	return TRUE;
}
#endif


/*                                                              */
/* Find free cluster in disk FAT table                          */
/*                                                              */
static UWORD 
find_fat_free (struct f_node FAR *fnp)
{
	REG UWORD idx;

	/* Start from optimized lookup point for start of FAT   */
	if(fnp -> f_dpb -> dpb_cluster != UNKNCLUSTER)
		idx =  fnp -> f_dpb -> dpb_cluster;
	else
		idx = 2;

	/* Search the FAT table looking for the first free      */
	/* entry.                                               */
	for( ; idx < fnp -> f_dpb -> dpb_size; idx++)
	{
		if(next_cluster(fnp -> f_dpb, idx) == FREE)
			break;
	}

	/* No empty clusters, disk is FULL!                     */
	if(idx >= fnp -> f_dpb -> dpb_size)
	{
		fnp -> f_dpb -> dpb_cluster = UNKNCLUSTER;
		dir_close(fnp);
		return LONG_LAST_CLUSTER;
	}

	/* return the free entry                                */
	fnp -> f_dpb -> dpb_cluster = idx;
	return idx;
}



/*                                                              */
/* crate a directory - returns success or a negative error      */
/* number                                                       */
/*                                                              */
COUNT 
dos_mkdir (BYTE FAR *dir)
{
	REG struct f_node FAR *fnp;
	REG COUNT idx;
	struct buffer FAR *bp;
	BYTE FAR *p;
	UWORD free_fat;
	UWORD parent;
	BYTE dname[NAMEMAX];
	BYTE fname[FNAME_SIZE];
	BYTE fext[FEXT_SIZE];
	struct dirent newent;

	/* first split the passed dir into comopnents (i.e. -   */
	/* path to new directory and name of new directory      */
	if((fnp = split_path(dir, dname, fname, fext)) == NULL)
	{
		dir_close(fnp);
		return DE_PATHNOTFND;
	}

	/* Check that we don't have a duplicate name, so if we  */
	/* find one, it's an error.                             */
	if(find_fname(fnp, fname, fext))
	{
		dir_close(fnp);
		return DE_ACCESS;
	}
	else
	{
		BOOL is_free;

		/* Reset the directory by a close followed by   */
		/* an open                                      */
		fnp -> f_flags.f_dmod = FALSE;
		parent = fnp -> f_dirstart;
		dir_close(fnp);
		fnp = dir_open((BYTE FAR *)dname);

		/* Get a free f_node pointer so that we can use */
		/* it in building the new file.                 */
		/* Note that if we're in the root and we don't  */
		/* find an empty slot, we need to abort.        */
		if(!(is_free = find_free(fnp)) && (fnp -> f_flags.f_droot))
		{
			fnp -> f_flags.f_dmod = FALSE;
			dir_close(fnp);
			return DE_TOOMANY;
		}

		/* Otherwise just expand the directory          */
		else if(!is_free && !(fnp -> f_flags.f_droot))
		{
			COUNT ret;

			if(extend_dir(fnp) != SUCCESS)
				return ret;
		}

		/* put the fnode's name into the directory.             */
		bcopy(fname, (BYTE *)fnp -> f_dir.dir_name, FNAME_SIZE);
		bcopy(fext, (BYTE *)fnp -> f_dir.dir_ext, FEXT_SIZE);

		/* Set the fnode to the desired mode                            */
		fnp -> f_mode = WRONLY;
		fnp -> f_back = LONG_LAST_CLUSTER;

		fnp -> f_dir.dir_size = 0l;
		fnp -> f_dir.dir_start = FREE;
		fnp -> f_dir.dir_attrib = D_DIR;
		fnp -> f_dir.dir_time = dos_gettime();
		fnp -> f_dir.dir_date = dos_getdate();

		fnp -> f_flags.f_dmod = TRUE;
		fnp -> f_flags.f_dnew = FALSE;
		fnp -> f_flags.f_ddir = TRUE;

		fnp -> f_highwater = 0l;
		fnp -> f_offset = 0l;
	}

	/* get an empty cluster, so that we make it into a      */
	/* directory.                                           */
	free_fat = find_fat_free(fnp);

	/* No empty clusters, disk is FULL! Translate into a    */
	/* useful error message.                                */
	if(free_fat == LONG_LAST_CLUSTER)
	{
		dir_close(fnp);
		return DE_HNDLDSKFULL;
	}

	/* Mark the cluster in the FAT as used                  */
	fnp -> f_dir.dir_start = fnp -> f_cluster = free_fat;
	link_fat(fnp -> f_dpb, (UCOUNT)free_fat, LONG_LAST_CLUSTER);

	/* Craft the new directory. Note that if we're in a new */
	/* directory just under the root, ".." pointer is 0.    */
	bp = getblock((LONG)clus2phys(free_fat,
		fnp -> f_dpb -> dpb_clssize,
		fnp -> f_dpb -> dpb_data),
		fnp -> f_dpb -> dpb_unit);
	if(bp == NULL)
	{
		dir_close(fnp);
		return DE_BLKINVLD;
	}

	/* Create the "." entry                                 */
	bcopy(".       ", (BYTE *)newent.dir_name, FNAME_SIZE);
	bcopy("   ", (BYTE *)newent.dir_ext, FEXT_SIZE);
	newent.dir_attrib = D_DIR;
	newent.dir_time = dos_gettime();
	newent.dir_date = dos_getdate();
	newent.dir_start = free_fat;
	newent.dir_size = 0l;

	/* And put it out                                       */
	putdirent((struct dirent FAR *)&newent, (BYTE FAR *)bp ->b_buffer);

	/* create the ".." entry                                */
	bcopy("..      ", (BYTE *)newent.dir_name, FNAME_SIZE);
	newent.dir_start = parent;

	/* and put it out                                       */
	putdirent((struct dirent FAR *)&newent, (BYTE FAR *)&bp -> b_buffer[DIRENT_SIZE]);

	/* fill the rest of the block with zeros                */
	for(p = (BYTE FAR *)&bp -> b_buffer[2 *DIRENT_SIZE];
	 p < &bp -> b_buffer[BUFFERSIZE]; )
		*p++ = NULL;

	/* Mark the block to be written out                     */
	bp -> b_update = TRUE;

	/* clear out the rest of the blocks in the cluster      */
	for(idx = 1; idx < fnp -> f_dpb -> dpb_clssize; idx++)
	{
		REG COUNT i;

		bp = getblock(
			(LONG)clus2phys(fnp -> f_dir.dir_start,
			fnp -> f_dpb -> dpb_clssize,
			fnp -> f_dpb -> dpb_data) + idx,
			fnp -> f_dpb -> dpb_unit);
		if(bp == NULL)
		{
			dir_close(fnp);
			return DE_BLKINVLD;
		}
		for(i = 0, p = (BYTE FAR *)bp -> b_buffer; i < BUFFERSIZE; i++)
			*p++ = NULL;
		bp -> b_update = TRUE;
	}

	/* flush the drive buffers so that all info is written  */
	flush_buffers((COUNT)(fnp -> f_dpb -> dpb_unit));

	/* Close the directory so that the entry is updated     */
	fnp -> f_flags.f_dmod = TRUE;
	dir_close(fnp);

	return SUCCESS;
}
#endif


BOOL 
last_link (struct f_node FAR *fnp)
{
	return (((UWORD)fnp -> f_cluster == (UWORD)LONG_LAST_CLUSTER)
	 || ((UWORD)fnp -> f_cluster == (UWORD)LAST_CLUSTER));
}


#ifndef IPL
static BOOL 
extend (struct f_node FAR *fnp)
{
	UWORD free_fat;

	/* get an empty cluster, so that we use it to extend the file.  */
	free_fat = find_fat_free(fnp);

	/* No empty clusters, disk is FULL! Translate into a useful     */
	/* error message.                                               */
	if(free_fat == LONG_LAST_CLUSTER)
		return FALSE;

	/* Now that we've found a free FAT entry, mark it as the last   */
	/* entry and save.                                              */
	link_fat(fnp -> f_dpb, (UCOUNT)fnp -> f_back, free_fat);
	fnp -> f_cluster = free_fat;
	link_fat(fnp -> f_dpb, (UCOUNT)free_fat, LONG_LAST_CLUSTER);

	/* Mark the directory so that the entry is updated              */
	fnp -> f_flags.f_dmod = TRUE;
	return TRUE;
}


static COUNT 
extend_dir (struct f_node FAR *fnp)
{
	REG COUNT idx;

	if(!extend(fnp))
	{
		dir_close(fnp);
		return DE_HNDLDSKFULL;
	}

	/* clear out the rest of the blocks in the cluster              */
	for(idx = 0; idx < fnp -> f_dpb -> dpb_clssize; idx++)
	{
		REG COUNT i;
		REG BYTE FAR *p;
		REG struct buffer FAR *bp;

		bp = getblock(
			(LONG)clus2phys(fnp -> f_cluster,
			fnp -> f_dpb -> dpb_clssize,
			fnp -> f_dpb -> dpb_data) + idx,
			fnp -> f_dpb -> dpb_unit);
		if(bp == NULL)
		{
			dir_close(fnp);
			return DE_BLKINVLD;
		}
		for(i = 0, p = (BYTE FAR *)bp -> b_buffer; i < BUFFERSIZE; i++)
			*p++ = NULL;
		bp -> b_update = TRUE;
	}

	if(!find_free(fnp))
	{
		dir_close(fnp);
		return DE_HNDLDSKFULL;
	}

	/* flush the drive buffers so that all info is written          */
	flush_buffers((COUNT)(fnp -> f_dpb -> dpb_unit));

	return SUCCESS;

}


static BOOL 
first_fat (struct f_node FAR *fnp)
{
	UWORD free_fat;

	/* get an empty cluster, so that we make it into a file.        */
	free_fat = find_fat_free(fnp);

	/* No empty clusters, disk is FULL! Translate into a useful     */
	/* error message.                                               */
	if(free_fat == LONG_LAST_CLUSTER)
		return FALSE;

	/* Now that we've found a free FAT entry, mark it as the last   */
	/* entry and save it.                                           */
	fnp -> f_dir.dir_start = free_fat;
	link_fat(fnp -> f_dpb, (UCOUNT)free_fat, LONG_LAST_CLUSTER);

	/* Mark the directory so that the entry is updated              */
	fnp -> f_flags.f_dmod = TRUE;
	return TRUE;
}
#endif


COUNT 
map_cluster (REG struct f_node FAR *fnp, COUNT mode)
{
	ULONG idx;
	WORD clssize, secsize;

	/* Set internal index and cluster size.                 */
	idx = fnp -> f_offset;

	/* The variable clssize will be used later.             */
	secsize = fnp -> f_dpb -> dpb_secsize;
	clssize = secsize * fnp -> f_dpb -> dpb_clssize;

#ifndef IPL
	/* If someone did a seek, but no writes have occured, we will   */
	/* need to initialize the fnode.                                */
	if((mode == XFR_WRITE) && (fnp -> f_dir.dir_start == FREE))
		if(!first_fat(fnp))
			return DE_HNDLDSKFULL;
#endif

	/* Now begin the linear search. The relative cluster is         */
	/* maintained as part of the set of physical indices. It is     */
	/* also the highest order index and is mapped directly into     */
	/* physical cluster. Our search is performed by pacing an index */
	/* up to the relative cluster position where the index falls    */
	/* within the cluster.                                          */
	/*                                                              */
	/* NOTE: make sure your compiler does not optimize for loop     */
	/* tests to the loop exit. We need to fall out immediately for  */
	/* files whose length < cluster size.                           */
	for(fnp -> f_cluster = fnp -> f_flags.f_ddir ?
				fnp -> f_dirstart :
				fnp -> f_dir.dir_start;
	  idx >= clssize;
	   idx -= clssize)
	{
		/* If this is a read and the next is a LAST_CLUSTER,    */
		/* then we are going to read past EOF, return zero read */
		if((mode == XFR_READ) && last_link(fnp))
			return DE_SEEK;
#ifndef IPL
	/* expand the list if we're going to write and have run into    */
	/* the last cluster marker.                                     */
		else if((mode == XFR_WRITE) && last_link(fnp))
		{
			if(!extend(fnp))
			{
				dir_close(fnp);
				return DE_HNDLDSKFULL;
			}
		}
#endif
		else
		{
			fnp -> f_back = fnp -> f_cluster;
			fnp -> f_cluster = next_cluster(fnp -> f_dpb,fnp -> f_cluster);
		}
	}
	return SUCCESS;
}




UCOUNT 
rdwrblock (COUNT fd, VOID FAR *buffer, UCOUNT count, COUNT mode, COUNT *err)
{
	REG struct f_node FAR *fnp;
	REG struct buffer FAR *bp;
	UCOUNT xfr_cnt = 0, ret_cnt = 0;
	LONG idx;
	WORD secsize;
	UCOUNT to_xfer = count;

#ifdef DEBUG
	if(bDumpRdWrParms)
	{
		printf("rdwrblock: mode = %s\n",
		 mode == XFR_WRITE ? "WRITE" : "READ");
		printf(" fd   buffer     count\n --   ------     -----\n");
		printf(" %02d   %04x:%04x   %d\n",
		 fd, (COUNT)FP_SEG(buffer), (COUNT)FP_OFF(buffer), count);
	}
#endif
	/* Translate the fd into an fnode pointer, since all internal   */
	/* operations are achieved through fnodes.                      */
	fnp = xlt_fd(fd);

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit        */
	/* note: an invalid fd is indicated by a 0 return               */
	if(fnp == (struct f_node FAR *)0 || fnp -> f_count <= 0)
	{
		*err = DE_INVLDHNDL;
		return 0;
	}

	/* Test that we are really about to do a data transfer. If the  */
	/* count is zero and the mode is XFR_READ, just exite. (Any     */
	/* read with a count of zero is a nop).                         */
	/*                                                              */
	/* A write (mode is XFR_WRITE) is a special case, it sets the   */
	/* file length to the current length (truncates it).            */
	/*                                                              */
	/* NOTE: doing this up front saves a lot of headaches later.    */
	if(count == 0)
	{
		if(mode == XFR_WRITE)
			fnp -> f_highwater = fnp -> f_offset;
		{
			*err = SUCCESS;
			return 0;
		}
	}

	/* Another test is to check for a seek past EOF on an XFR_READ  */
	/* operation.                                                   */
	if(mode == XFR_READ
	 && !fnp -> f_flags.f_ddir
	  && (fnp -> f_offset >= fnp -> f_dir.dir_size))
	{
		*err = SUCCESS;
		return 0;
	}

	/* test that we have a valid mode for this fnode                */
	switch(mode)
	{
	case XFR_READ:
		if(fnp -> f_mode != RDONLY && fnp -> f_mode != RDWR)
		{
			*err = DE_INVLDACC;
			return 0;
		}
		break;

#ifndef IPL
	case XFR_WRITE:
		if(fnp -> f_mode != WRONLY && fnp -> f_mode != RDWR)
		{
			*err = DE_INVLDACC;
			return 0;
		}
		break;
#endif
	default:
		*err = DE_INVLDACC;
		return 0;
	}

	/* The variable secsize will be used later.                     */
	secsize = fnp -> f_dpb -> dpb_secsize;

	/* Adjust the far pointer from user space tp supervisor space   */
	buffer = adjust_far((VOID FAR *)buffer);

	/* Do the data transfer. Use block transfer methods so that we  */
	/* can utilize memory management in more complex DOS/NT         */
	/* versions.                                                    */
	while(ret_cnt < count)
	{
		/* Position the file to the fnode's pointer position. This is   */
		/* done by updating the fnode's cluster, block (sector) and     */
		/* byte offset so that read or write becomes a simple data move */
		/* into or out of the block data buffer.                        */
		if(fnp -> f_offset == 0l)
		{
#ifndef IPL
			/* For the write case, a newly created file     */
			/* will have a start cluster of FREE. If we're  */
			/* doing a write and this is the first time     */
			/* through, allocate a new cluster to the file. */
			if((mode == XFR_WRITE)
			 && (fnp -> f_dir.dir_start == FREE))
				if(!first_fat(fnp))
				{
					dir_close(fnp);
					*err = DE_HNDLDSKFULL;
					return ret_cnt;
				}
#endif
			/* complete the common operations of            */
			/* initializing to the starting cluster and     */
			/* setting all offsets to zero.                 */
			fnp -> f_cluster = fnp -> f_dir.dir_start;
			fnp -> f_back = LONG_LAST_CLUSTER;
			fnp -> f_sector = 0;
			fnp -> f_boff = 0;
		}

		/* The more difficult scenario is the (more common)     */
		/* file offset case. Here, we need to take the fnode's  */
		/* offset pointer (f_offset) and translate it into a    */
		/* relative cluster position, cluster block (sector)    */
		/* offset (f_sector) and byte offset (f_boff). Once we  */
		/* have this information, we need to translate the      */
		/* relative cluster position into an absolute cluster   */
		/* position (f_cluster). This is unfortunate because it */
		/* requires a linear search through the file's FAT      */
		/* entries. It made sense when DOS was originally       */
		/* designed as a simple floppy disk operating system    */
		/* where the FAT was contained in core, but now         */
		/* requires a search through the FAT blocks.            */
		/*                                                      */
		/* The algorithm in this function takes advantage of    */
		/* the blockio block buffering scheme to simplify the   */
		/* task.                                                */
		else
			switch(map_cluster(fnp, mode))
			{
			case DE_SEEK:
				dir_close(fnp);
				return ret_cnt;

			default:
				dir_close(fnp);
				*err = DE_HNDLDSKFULL;
				return ret_cnt;

			case SUCCESS:
				break;
			}

#ifndef IPL
		/* XFR_WRITE case only - if we're at the end, the next  */
		/* FAT is an EOF marker, so just extend the file length */
		if(mode == XFR_WRITE && last_link(fnp))
			if(!extend(fnp))
			{
				dir_close(fnp);
				*err = DE_HNDLDSKFULL;
				return ret_cnt;
			}
#endif

		/* Compute the block within the cluster and the offset  */
		/* within the block.                                    */
		fnp -> f_sector =
		 (fnp -> f_offset / secsize) & fnp -> f_dpb -> dpb_clsmask;
		fnp -> f_boff = fnp -> f_offset % secsize;

#ifdef DSK_DEBUG
	printf("%d links; dir offset %ld, starting at cluster %d\n",
		fnp -> f_count,
		fnp -> f_diroff,
		fnp -> f_cluster);
#endif

		/* Do an EOF test and return whatever was transferred   */
		/* but only for regular files in XFR_READ mode          */
		if((mode == XFR_READ) && !(fnp -> f_flags.f_ddir)
		  && (fnp -> f_offset >= fnp -> f_dir.dir_size))
		{
			*err = SUCCESS;
			return ret_cnt;
		}

		/* Get the block we need from cache                     */
		bp = getblock(
			(LONG)clus2phys(fnp -> f_cluster,
				fnp -> f_dpb -> dpb_clssize,
				fnp -> f_dpb -> dpb_data) + fnp -> f_sector,
			fnp -> f_dpb -> dpb_unit);
		if(bp == (struct buffer *)0)
		{
			*err = DE_BLKINVLD;
			return ret_cnt;
		}

		/* transfer a block                                     */
		/* Transfer size as either a full block size, or the    */
		/* requested transfer size, whichever is smaller.       */
		/* Then compare to what is left, since we can transfer  */
		/* a maximum of what is left.                           */
		switch(mode)
		{
		case XFR_READ:
			if(fnp -> f_flags.f_ddir)
				xfr_cnt = min(to_xfer, secsize - fnp -> f_boff);
			else
				xfr_cnt = min(min(to_xfer,
				 secsize - fnp -> f_boff),
				  fnp -> f_dir.dir_size - fnp -> f_offset);
			fbcopy((BYTE FAR *)&bp -> b_buffer[fnp -> f_boff],
			 buffer,
			  xfr_cnt);
			break;

#ifndef IPL
		case XFR_WRITE:
			xfr_cnt = min(to_xfer, secsize - fnp -> f_boff);
			fbcopy(buffer,
			 (BYTE FAR *)&bp -> b_buffer[fnp -> f_boff],
			  xfr_cnt);
			bp -> b_update = TRUE;
			break;
#endif

		default:
			*err =  DE_INVLDACC;
			return ret_cnt;
		}

		/* update pointers and counters                         */
		ret_cnt += xfr_cnt;
		to_xfer -= xfr_cnt;
		fnp -> f_offset += xfr_cnt;
		buffer = add_far((VOID FAR *)buffer, (ULONG)xfr_cnt);
		if(mode == XFR_WRITE && (fnp -> f_offset > fnp -> f_highwater))
			fnp -> f_highwater = fnp -> f_offset;
	}
	*err = SUCCESS;
	return ret_cnt;
}

COUNT 
dos_read (COUNT fd, VOID FAR *buffer, UCOUNT count)
{
	COUNT err, xfr;

	xfr = rdwrblock(fd, buffer, count, XFR_READ, &err);
	return err != SUCCESS ? err : xfr;
}


#ifndef IPL
COUNT 
dos_write (COUNT fd, VOID FAR *buffer, UCOUNT count)
{
	COUNT err, xfr;

	xfr = rdwrblock(fd, buffer, count, XFR_WRITE, &err);
	return err != SUCCESS ? err : xfr;
}
#endif


/* Position the file pointer to the desired offset                      */
/* Returns a long current offset or a negative error code               */

LONG 
dos_lseek (COUNT fd, LONG foffset, COUNT origin)
{
	REG struct f_node FAR *fnp;

	/* Translate the fd into a useful pointer                       */

	fnp = xlt_fd(fd);

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit                */
	/* note: an invalid fd is indicated by a 0 return               */

	if(fnp == (struct f_node FAR *)0 || fnp -> f_count <= 0)
		return (LONG)DE_INVLDHNDL;

	/* now do the actual lseek adjustment to the file poitner       */

	switch(origin)
	{
	/* offset from beginning of file                                */
	case 0:
		return fnp -> f_offset = foffset;

	/* offset from current location                                 */
	case 1:
		return fnp -> f_offset += foffset;

	/* offset from eof +                                            */
	/* return file size if past eof                                 */
	case 2:
		return fnp -> f_offset
			= fnp -> f_offset + foffset > fnp -> f_dir.dir_size
			? fnp -> f_dir.dir_size
			: fnp -> f_dir.dir_size + foffset;

	/* default to an invalid function                               */
	default:
		return (LONG)DE_INVLDFUNC;
	}
}


UWORD 
dos_free (struct dpb *dpbp)
{
	/* There's an unwritten rule here. All fs       */
	/* cluster start at 2 and run to max_cluster+2  */
	REG UWORD i, cnt = 0;
	UWORD max_cluster = (dpbp -> dpb_size * dpbp -> dpb_clssize - dpbp -> dpb_data + 1) / dpbp -> dpb_clssize + 2;

	if(dpbp -> dpb_nfreeclst != UNKNCLUSTER)
		return dpbp -> dpb_nfreeclst;
	else
	{
		for(i = 2; i < max_cluster; i++)
		{
			if(next_cluster(dpbp, i) == 0)
				++cnt;
		}
		dpbp -> dpb_nfreeclst = cnt;
		return cnt;
	}
}


#ifndef IPL
VOID 
dos_pwd (struct dpb *dpbp, BYTE FAR *s)
{
	fsncopy((BYTE FAR *)&dpbp -> dpb_path[1], s, 64);
}
#endif


static BOOL 
psep (COUNT c)
{
	return (c == '\\' || c == '/' || c == '\0');
}


VOID 
trim_path (BYTE FAR *s)
{
	BYTE FAR *p, FAR *base = s;

	/* trim all multiple path seperators first              */
	for(s = base; *s != '\0'; ++s)
		while(psep(s[0]) && s[1] != '\0' && psep(s[1]))
			fscopy(&s[1], s);
	s = base;

	/* Handle special root case where . and .. point to     */
	/* root                                                 */
	if(s[0] == '\\')
	{
		/* look for common '.'                          */
		while(s[1] == '.')
		{
			if(psep(s[2]) && (s[3] == '\0'))
				fscopy(&s[3], &s[1]);
			else if(s[2] == '.' && psep(s[3]))
				fscopy(&s[4], &s[1]);
			else
				break;
		}
		++s;
	}

	/* now step through path, compressing . and ..          */
	while(*s != '\0')
	{
		p = s;
		while(!psep(*s))
			++s;
		if(*s == '\0')
			break;
		++s;
		while(s[0] == '.')
		{
			/* special trailing '.'                 */
			if(s[1] == '\0')
			{
				*--s = '\0';
				break;
			}
			else if(psep(s[1]) && (s[1] != '\0'))
			{
				fscopy(&s[2], &s[0]);
			}
			/* special trailing ".."                */
			else if(s[1] == '.' && (s[2] == '\0'))
			{
				*p = '\0';
			}
			else if(s[1] == '.' && psep(s[2]) && (s[2] != '\0'))
			{
				/* copy the tail over the previous path */
				/* segment.                             */
				fscopy(&s[3], p);

				/* Now that we've copied the tail into  */
				/* the '..' position, move p back. Skip */
				/* over the last seperator.             */
				--p;

				/* Now isolate the .. segment.          */
				do
					--p;
				while(!psep(*p) && (p != base));
			}

			s = p;
		}
	}

	/* Eliminate trailing psep if present.                          */
	s = &base[fstrlen(base) - 1];
	if((s != base) && psep(*s))
		*s = NULL;

}


#ifndef IPL
COUNT 
dos_cd (struct dpb *dpbp, BYTE FAR *s)
{
	BYTE FAR *p;
	static BYTE path[65];
	struct f_node FAR *fnp;

	/* Get the current directory so that we initialize all access   */
	/* relative to root.                                            */
	scopy(dpbp -> dpb_path, path);

	/* If it's not an absolute path (i.e. - starts from root), then */
	/* convert it to an absolute path.                              */
	if(!(*s == '/' || *s == '\\'))
	{
		/* Append to end of path and let trim_path take care of */
		/* it later.                                            */
		for(p = (BYTE FAR *)path; *p != '\0'; p++)
			;
		*p++ = '\\';
		fsncopy(s, p, 64);
	}

	/* If it is absolute, copy it into path instead                 */
	else
		fscopy(s, (BYTE FAR *)path);

	/* Run throught the newly created path and convert all path     */
	/* seperators to \                                              */
	for(p = (BYTE FAR *)path; *p != '\0'; p++)
		if(*p == '/')
			*p = '\\';

	/* clean up the path and convert to upper case                  */
	trim_path((BYTE FAR *)path);
	upMem((BYTE FAR *)path, strlen(path));

	/* now test for its existance. If it doesn't, return an error.  */
	/* If it does, copy the path to the current directory           */
	/* structure.                                                   */
	if((fnp = dir_open((BYTE FAR *)path)) == NULL)
		return DE_PATHNOTFND;
	else
	{
		dir_close(fnp);
		scopy(path, dpbp -> dpb_path);
		return SUCCESS;
	}
}
#endif


struct f_node FAR *
get_f_node (void)
{
	REG i;
	
	for(i = 0; i < NFILES; i++)
	{
		if(f_nodes[i].f_count == 0)
		{
			++f_nodes[i].f_count;
			return &f_nodes[i];
		}
	}
	return (struct f_node FAR *)0;
}


VOID 
release_f_node (struct f_node FAR *fnp)
{
	if(fnp -> f_count > 0)
		--fnp -> f_count;
	else
		fnp -> f_count = 0;
}


#ifndef IPL
VOID 
dos_setdta (BYTE FAR *newdta)
{
	dta = newdta;
}


COUNT 
dos_getfattr (BYTE FAR *name, UWORD FAR *attrp)
{
	struct f_node FAR *fnp;
	COUNT fd;

	/* Translate the fd into an fnode pointer, since all internal   */
	/* operations are achieved through fnodes.                      */
	if((fd = dos_open(name, O_RDONLY)) < SUCCESS)
		return DE_FILENOTFND;
	/* note: an invalid fd is indicated by a 0 return               */
	if((fnp = xlt_fd(fd)) == (struct f_node FAR *)0)
		return DE_TOOMANY;

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit        */
	if(fnp -> f_count <= 0)
		return DE_FILENOTFND;

	/* Get the attribute from the fnode and return          */
	*attrp = fnp -> f_dir.dir_attrib;
	dos_close(fd);
	return SUCCESS;
}


COUNT 
dos_setfattr (BYTE FAR *name, UWORD FAR *attrp)
{
	struct f_node FAR *fnp;
	COUNT fd;

	/* Translate the fd into an fnode pointer, since all internal   */
	/* operations are achieved through fnodes.                      */
	if((fd = dos_open(name, O_RDONLY)) < SUCCESS)
		return DE_FILENOTFND;
	/* note: an invalid fd is indicated by a 0 return               */
	if((fnp = xlt_fd(fd)) == (struct f_node FAR *)0)
		return DE_TOOMANY;

	/* If the fd was invalid because it was out of range or the     */
	/* requested file was not open, tell the caller and exit        */
	if(fnp -> f_count <= 0)
		return DE_FILENOTFND;

	/* Set the attribute from the fnode and return          */
	fnp -> f_dir.dir_attrib = *attrp;
	fnp -> f_flags.f_dmod = TRUE;
	dos_close(fd);
	return SUCCESS;
}
#endif


COUNT 
media_check (REG struct dpb *dpbp)
{
	request rq;
	bpb FAR *bpbp;
	REG COUNT i;
	ULONG size;


	/* First test if anyone has changed the removable media         */
	FOREVER
	{
		rq.r_length = sizeof(request);
		rq.r_unit = dpbp -> dpb_subunit;
		rq.r_command = C_MEDIACHK;
		rq.r_mcmdesc = dpbp -> dpb_mdb;
		rq.r_status = 0;
		execrh((request FAR *)&rq, dpbp -> dpb_device);
		if(!(rq.r_status & S_ERROR) && (rq.r_status & S_DONE))
			break;
		else
		{
		loop1:
			switch(block_error(&rq, dpbp -> dpb_unit))
			{
			case ABORT:
			case FAIL:
				return DE_INVLDDRV;

			case RETRY:
				continue;

			case CONTINUE:
				break;

			default:
				goto loop1;
			}
		}
	}

	switch(rq.r_mcretcode | dpbp -> dpb_flags)
	{
	case M_NOT_CHANGED:
		/* It was definitely not changed, so ignore it          */
		return SUCCESS;

		/* If it is forced or the media may have changed,       */
		/* rebuild the bpb                                      */
	case M_DONT_KNOW:
		flush_buffers(dpbp -> dpb_unit);

		/* If it definitely changed, don't know (falls through) */
		/* or has been changed, rebuild the bpb.                */
	case M_CHANGED:
	default:
		setinvld(dpbp -> dpb_unit);
		FOREVER
		{
			rq.r_length = sizeof(request);
			rq.r_unit = dpbp -> dpb_subunit;
			rq.r_command = C_BLDBPB;
			rq.r_mcmdesc = dpbp -> dpb_mdb;
			rq.r_status = 0;
			execrh((request FAR *)&rq, dpbp -> dpb_device);
			if(!(rq.r_status & S_ERROR) && (rq.r_status & S_DONE))
				break;
			else
			{
			loop2:
				switch(block_error(&rq, dpbp -> dpb_unit))
				{
				case ABORT:
				case FAIL:
					return DE_INVLDDRV;

				case RETRY:
					continue;

				case CONTINUE:
					break;

				default:
					goto loop2;
				}
			}
		}
		bpbp = rq.r_bpptr;
		dpbp -> dpb_mdb = bpbp -> bpb_mdesc;
		dpbp -> dpb_secsize = bpbp -> bpb_nbyte;
		dpbp -> dpb_clssize = bpbp -> bpb_nsector;
		dpbp -> dpb_clsmask = bpbp -> bpb_nsector - 1;
		dpbp -> dpb_fatstrt = bpbp -> bpb_nreserved;
		dpbp -> dpb_fats = bpbp -> bpb_nfat;
		dpbp -> dpb_dirents = bpbp -> bpb_ndirent;
		size =  bpbp -> bpb_nsize == 0 ?
		 bpbp -> bpb_huge :
		 (ULONG)bpbp -> bpb_nsize;
		dpbp -> dpb_size = size / ((ULONG)bpbp -> bpb_nsector);
		dpbp -> dpb_fatsize = bpbp -> bpb_nfsect;
		dpbp -> dpb_dirstrt = dpbp -> dpb_fatstrt
			+ dpbp -> dpb_fats * dpbp -> dpb_fatsize + 1;
		dpbp -> dpb_data = dpbp -> dpb_dirstrt
			+ ((DIRENT_SIZE * dpbp -> dpb_dirents
			+ (dpbp -> dpb_secsize - 1))
			/ dpbp -> dpb_secsize);
		dpbp -> dpb_flags = 0;
		dpbp -> dpb_next = (struct dpb FAR *)-1;
		dpbp -> dpb_cluster = UNKNCLUSTER;
		dpbp -> dpb_nfreeclst = UNKNCLUSTER;
		for(i = 1, dpbp -> dpb_shftcnt = 0;
		 i < (sizeof(dpbp -> dpb_shftcnt) * 8); /* 8 bit bytes in C */
		 dpbp -> dpb_shftcnt++, i <<= 1)
		{
			if(i >= bpbp -> bpb_nsector)
				break;
		}
		return SUCCESS;
	}
}


/* translate the fd into an f_node pointer                              */

struct f_node FAR *
xlt_fd (COUNT fd)
{
	return fd > NFILES ? (struct f_node FAR *)0 : &f_nodes[fd];
}


COUNT 
xlt_fnp (struct f_node FAR *fnp)
{
	return fnp - f_nodes;
}


struct dhdr FAR *
select_unit (COUNT drive)
{
	/* Just get the header from the dhdr array                      */
	return blk_devices[drive].dpb_device;
}

