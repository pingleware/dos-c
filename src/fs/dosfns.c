
/****************************************************************/
/*                                                              */
/*                           dosfns.c                           */
/*                                                              */
/*                         DOS functions                        */
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

#ifdef VERSION_STRINGS
static BYTE *dosfnsRcsId = "$Header:   D:/dos-c/src/fs/dosfns.c_v   1.5   29 May 1996 21:15:20   patv  $";
#endif

/* $Logfile:   D:/dos-c/src/fs/dosfns.c_v  $ */

/*
 * $Log:   D:/dos-c/src/fs/dosfns.c_v  $
 * 
 *    Rev 1.5   29 May 1996 21:15:20   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.4   19 Feb 1996  3:20:08   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.2   01 Sep 1995 17:48:48   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:50:24   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:04:20   patv
 * Initial revision.
 */

#include "globals.h"

#ifdef PROTO
BOOL check_break(VOID);
sft FAR *get_sft(COUNT);
WORD get_free_hndl(VOID);
sft FAR *get_free_sft(WORD FAR *);
BYTE FAR *get_root(BYTE FAR *);
BOOL cmatch(COUNT, COUNT, COUNT);
BOOL fnmatch(BYTE FAR *, BYTE FAR *, COUNT, COUNT);
#else
BOOL check_break();
sft FAR *get_sft();
WORD get_free_hndl();
sft FAR *get_free_sft();
BYTE FAR *get_root();
BOOL cmatch();
BOOL fnmatch();
#endif


BOOL 
check_break (void)
{
	if(break_ena && con_break())
	{
		handle_break();
		/* shouldn't return, but in case...     */
		return TRUE;
	}
	else
		return FALSE;
}


static sft FAR *get_sft(hndl)
COUNT hndl;
{
	psp FAR *p = MK_FP(cu_psp,0);
	WORD sys_idx;
	sfttbl FAR *sp;

	/* Get the SFT block that contains the SFT      */
	sys_idx = p -> ps_filetab[hndl];
	for(sp = sfthead; sp != (sfttbl FAR *)-1; sp = sp -> sftt_next)
	{
		if(sys_idx < sp -> sftt_count)
			break;
		else
			sys_idx -= sp -> sftt_count;
	}

	/* If not found, return an error                */
	if(sp == (sfttbl FAR *)-1)
		return (sft FAR *)-1;

	/* finally, point to the right entry            */
	return (sft FAR *)&(sp -> sftt_table[sys_idx]);
}


UCOUNT 
DosRead (COUNT hndl, UCOUNT n, BYTE FAR *bp, COUNT FAR *err)
{
	sft FAR *s;
	WORD sys_idx;
	sfttbl FAR *sp;
	UCOUNT ReadCount;

	/* Test that the handle is valid                */
	if(hndl < 0)
	{
		*err = DE_INVLDHNDL;
		return 0;
	}

	/* Get the SFT block that contains the SFT      */
	if((s = get_sft(hndl)) == (sft FAR *)-1)
	{
		*err = DE_INVLDHNDL;
		return 0;
	}
	
	/* If not open or write permission - exit       */
	if(s -> sft_count == 0 || (s -> sft_mode & SFT_MWRITE))
	{
		*err = DE_INVLDACC;
		return 0;
	}

	/* Do a device read if device                   */
	if(s -> sft_flags & SFT_FDEVICE)
	{
		request rq;
		
		/* First test for eof and exit          */
		/* immediately if it is                 */
		if(!(s -> sft_flags & SFT_FEOF) || (s -> sft_flags & SFT_FNUL))
		{
			s -> sft_flags &= ~SFT_FEOF;
			*err = SUCCESS;
			return 0;
		}

		/* Now handle raw and cooked modes      */
		if(s -> sft_flags & SFT_FBINARY)
		{
			rq.r_length = sizeof(request);
			rq.r_command = C_INPUT;
			rq.r_count = n;
			rq.r_trans = (BYTE FAR *)bp;
			rq.r_status = 0;
			execrh((request FAR *)&rq, s -> sft_dev);
			if(rq.r_status & S_ERROR)
			{
				REG i;
				BYTE buff[FNAME_SIZE+1];

				fbcopy(s -> sft_name, (BYTE FAR *)buff, FNAME_SIZE);
				buff[FNAME_SIZE+1] = 0;
				for(i = FNAME_SIZE; i > 0; i--)
					if(buff[i] == ' ')
						buff[i] = 0;
					else
						break;
				char_error(&rq, buff);
			}
			else
			{
				*err = SUCCESS;
				return rq.r_count;
			}
		}
		else if(s -> sft_flags & SFT_FSTDIN)
		{
			if(!check_break())
			{
				kb_buf.kb_size = LINESIZE - 1;
				kb_buf.kb_count = 0;
				sti((keyboard FAR *)&kb_buf);
				fbcopy((BYTE FAR *)kb_buf.kb_buf, bp, kb_buf.kb_count);
				*err = SUCCESS;
				return kb_buf.kb_count;
			}
			else
			{
				*err = SUCCESS;
				return 0;
			}
		}
		else
		{
			if(!check_break())
			{
				*bp = _sti();
				*err = SUCCESS;
				return 1;
			}
			else
			{
				*err = SUCCESS;
				return 0;
			}
		}
	}
	else /* a block read                            */
	{
		if(!check_break())
		{
			COUNT rc;

			ReadCount = rdwrblock(s -> sft_status, bp, n, XFR_READ, &rc);
			if(rc != SUCCESS)
			{
				*err = rc;
				return 0;
			}
			else
			{
				*err = SUCCESS;
				return ReadCount;
			}
		}
		else
		{
			*err = SUCCESS;
			return 0;
		}
	}
	*err = SUCCESS;
	return 0;
}

UCOUNT 
DosWrite (COUNT hndl, UCOUNT n, BYTE FAR *bp, COUNT FAR *err)
{
	sft FAR *s;
	WORD sys_idx;
	sfttbl FAR *sp;
	UCOUNT ReadCount;

	/* Test that the handle is valid                */
	if(hndl < 0)
	{
		*err = DE_INVLDHNDL;
		return 0;
	}

	/* Get the SFT block that contains the SFT      */
	if((s = get_sft(hndl)) == (sft FAR *)-1)
	{
		*err = DE_INVLDHNDL;
		return 0;
	}

	/* If this is not opened and it's not a write   */
	/* another error                                */
	if(s -> sft_count == 0 ||
	 (!(s -> sft_mode & SFT_MWRITE) && !(s -> sft_mode & SFT_MRDWR)))
	{
		*err = DE_ACCESS;
		return 0;
	}

	/* Do a device write if device                  */
	if(s -> sft_flags & SFT_FDEVICE)
	{
		request rq;

		/* set to no EOF                        */
		s -> sft_flags &= ~SFT_FEOF;

		/* if null just report full transfer    */
		if(s -> sft_flags & SFT_FNUL)
		{
			*err = SUCCESS;
			return n;
		}

		/* Now handle raw and cooked modes      */
		if(s -> sft_flags & SFT_FBINARY)
		{
			rq.r_length = sizeof(request);
			rq.r_command = C_OUTPUT;
			rq.r_count = n;
			rq.r_trans = (BYTE FAR *)bp;
			rq.r_status = 0;
			execrh((request FAR *)&rq, s -> sft_dev);
			if(rq.r_status & S_ERROR)
			{
				REG i;
				BYTE buff[FNAME_SIZE+1];

				fbcopy(s -> sft_name, (BYTE FAR *)buff, FNAME_SIZE);
				buff[FNAME_SIZE+1] = 0;
				for(i = FNAME_SIZE; i > 0; i--)
					if(buff[i] == ' ')
						buff[i] = 0;
					else
						break;
				char_error(&rq, buff);
			}
			else
			{
				*err = SUCCESS;
				return rq.r_count;
			}
		}
		else if(s -> sft_flags & SFT_FSTDOUT)
		{
			REG WORD c, cnt = n, xfer = 0;

			while((cnt-- > 0) && ((c = *bp++) != CTL_Z) && !check_break())
			{
				switch(c)
				{
				case LF:
				case CR:
					sto(c);
					break;

				default:
					mod_sto(c);
					break;
				}
				++xfer;
			}
			*err = SUCCESS;
			return xfer;
		}
		else
		{
			REG c, cnt = n, xfer = 0;

			while(cnt-- && (*bp != CTL_Z) && !check_break())
			{
				rq.r_length = sizeof(request);
				rq.r_command = C_OUTPUT;
				rq.r_count = 1;
				rq.r_trans = (BYTE FAR *)bp++;
				rq.r_status = 0;
				execrh((request FAR *)&rq, s -> sft_dev);
				if(rq.r_status & S_ERROR)
				{
					REG i;
					BYTE buff[FNAME_SIZE+1];

					fbcopy(s -> sft_name, (BYTE FAR *)buff, FNAME_SIZE);
					buff[FNAME_SIZE+1] = 0;
					for(i = FNAME_SIZE; i > 0; i--)
						if(buff[i] == ' ')
							buff[i] = 0;
						else
							break;
					char_error(&rq, buff);
				}
				++xfer;
			}
			*err = SUCCESS;
			return xfer;
		}
	}
	else /* a block write                           */
	{
		if(!check_break())
		{
			COUNT rc;

			ReadCount = rdwrblock(s -> sft_status, bp, n, XFR_WRITE, &rc);
			if(rc < SUCCESS)
			{
				*err = rc;
				return 0;
			}
			else
			{
				*err = SUCCESS;
				return ReadCount;
			}
		}
		else
		{
			*err = SUCCESS;
			return 0;
		}
	}
	*err = SUCCESS;
	return 0;
}

COUNT 
DosSeek (COUNT hndl, LONG new_pos, COUNT mode, ULONG *set_pos)
{
	sft FAR *s;

	/* Test for invalid mode                        */
	if(mode < 0 || mode > 2)
		return DE_INVLDFUNC;

	/* Test that the handle is valid                */
	if(hndl < 0)
		return DE_INVLDHNDL;

	/* Get the SFT block that contains the SFT      */
	if((s = get_sft(hndl)) == (sft FAR *)-1)
		return DE_INVLDHNDL;

	/* Do special return for character devices      */
	if(s -> sft_flags & SFT_FDEVICE)
	{
		*set_pos = 0l;
		return SUCCESS;
	}
	else
	{
		*set_pos = dos_lseek(s -> sft_status, new_pos, mode);
		if((LONG)*set_pos < 0)
			return (int)*set_pos;
		else
			return SUCCESS;
	}
}


static WORD 
get_free_hndl (void)
{
	psp FAR *p = MK_FP(cu_psp,0);
	WORD hndl;

	for(hndl = 0; hndl < p -> ps_maxfiles; hndl++)
	{
		if(p -> ps_filetab[hndl] == 0xff)
			return hndl;
	}
	return 0xff;
}

static sft FAR *get_free_sft(sft_idx)
WORD FAR *sft_idx;
{
	WORD sys_idx = 0;
	sfttbl FAR *sp;

	/* Get the SFT block that contains the SFT      */
	for(sp = sfthead; sp != (sfttbl FAR *)-1; sp = sp -> sftt_next)
	{
		REG WORD i;

		for(i = 0; i < sp -> sftt_count; i++)
		{
			if(sp -> sftt_table[i].sft_count == 0)
			{
				*sft_idx = sys_idx + i;
				return (sft FAR *)&sp -> sftt_table[sys_idx + i];
			}
		}
		sys_idx += i;
	}

	/* If not found, return an error                */
	return (sft FAR *)-1;
}


static BYTE FAR *
get_root (BYTE FAR *fname)
{
	BYTE FAR *froot;
	REG WORD length;

	/* find the end                                 */
	for(length = 0, froot = fname; !froot != '\0'; ++froot)
		++length;
	/* now back up to first path seperator or start */
	for(--froot; length > 0 && !(*froot == '/' || *froot == '\\'); --froot)
		;
	return ++froot;
}


/* Ascii only file name match routines                  */
static BOOL 
cmatch (COUNT s, COUNT d, COUNT mode)
{
	if(s >= 'a' && s <= 'z')
		s -= 'a' - 'A';
	if(d >= 'a' && d <= 'z')
		d -= 'a' - 'A';
	if(mode && s == '?' && (d >= 'A' && s <= 'Z'))
		return TRUE;
	return s == d;
}

static BOOL 
fnmatch (BYTE FAR *s, BYTE FAR *d, COUNT n, COUNT mode)
{
	while(n--)
	{
		if(!cmatch(*s++, *d++, mode))
			return FALSE;
	}
	return TRUE;
}


#ifndef IPL
COUNT 
DosCreat (BYTE FAR *fname, COUNT attrib)
{
	psp FAR *p = MK_FP(cu_psp,0);
	WORD hndl, sft_idx;
	sft FAR *sftp;
	struct dhdr FAR *dhp;
	BYTE FAR *froot;
	BYTE buff[FNAME_SIZE+FEXT_SIZE];
	WORD i;

	/* get a free handle                            */
	if((hndl = get_free_hndl()) == 0xff)
		return DE_TOOMANY;

	/* now get a free system file table entry       */
	if((sftp = get_free_sft((WORD FAR *)&sft_idx)) == (sft FAR *)-1)
		return DE_TOOMANY;
	
	/* check for a device                           */
	froot = get_root(fname);
	for(i = 0; i < FNAME_SIZE; i++)
	{
		if(*froot != '\0' && *froot != '.')
			buff[i] = *froot++;
		else
			break;
	}

	for( ; i < FNAME_SIZE; i++)
		buff[i] = ' ';

	/* if we have an extension, can't be a device   */
	if(*froot != '.');
	{
		for(dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1; dhp = dhp -> dh_next)
		{
			if(fnmatch((BYTE FAR *)buff, (BYTE FAR *)dhp -> dh_name, FNAME_SIZE, FALSE))
			{
				sftp -> sft_count += 1;
				sftp -> sft_mode = SFT_MRDWR;
				sftp -> sft_attrib = attrib;
				sftp -> sft_flags = 
				  (dhp -> dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF;
				sftp -> sft_psp = cu_psp;
				fbcopy((BYTE FAR *)buff, sftp -> sft_name, FNAME_SIZE+FEXT_SIZE);
				sftp -> sft_dev = dhp;
				p -> ps_filetab[hndl] = sft_idx;
				return hndl;
			}
		}
	}
	sftp -> sft_status = dos_creat(fname, attrib);
	if(sftp -> sft_status >= 0)
	{
		p -> ps_filetab[hndl] = sft_idx;
		sftp -> sft_count += 1;
		sftp -> sft_mode = SFT_MRDWR;
		sftp -> sft_attrib = attrib;
		sftp -> sft_flags = 0;
		sftp -> sft_psp = cu_psp;
		fbcopy((BYTE FAR *)buff, sftp -> sft_name, FNAME_SIZE+FEXT_SIZE);
		return hndl;
	}
	else
		return sftp -> sft_status;
}
#endif


COUNT 
CloneHandle (COUNT hndl)
{
	sft FAR *sftp;

	/* now get the system file table entry                          */
	if((sftp = get_sft(hndl)) == (sft FAR *)-1)
		return DE_INVLDHNDL;
	
	/* now that we have the system file table entry, get the fnode  */
	/* index, and increment the count, so that we've effectively    */
	/* cloned the file.                                             */
	sftp -> sft_count += 1;
	return hndl;
}


COUNT 
DosDup (COUNT Handle)
{
	psp FAR *p = MK_FP(cu_psp,0);
	COUNT NewHandle;
	sft FAR *Sftp;

	/* Get the SFT block that contains the SFT                      */
	if((Sftp = get_sft(Handle)) == (sft FAR *)-1)
		return DE_INVLDACC;

	/* If not open - exit                                           */
	if(Sftp -> sft_count <= 0)
		return DE_INVLDACC;

	/* now get a free handle                                        */
	if((NewHandle = get_free_hndl()) == 0xff)
		return DE_TOOMANY;

	/* If everything looks ok, bump it up.                          */
	if((Sftp -> sft_flags & SFT_FDEVICE) || (Sftp -> sft_status >= 0))
	{
		p -> ps_filetab[NewHandle] = p -> ps_filetab[Handle];
		Sftp -> sft_count += 1;
		return NewHandle;
	}
	else
		return DE_INVLDACC;
}




COUNT 
DosForceDup (COUNT OldHandle, COUNT NewHandle)
{
	psp FAR *p = MK_FP(cu_psp,0);
	sft FAR *Sftp;

	/* Get the SFT block that contains the SFT                      */
	if((Sftp = get_sft(OldHandle)) == (sft FAR *)-1)
		return DE_INVLDACC;

	/* If not open - exit                                           */
	if(Sftp -> sft_count <= 0)
		return DE_INVLDACC;

	/* now close the new handle if it's open                        */
	if(p -> ps_filetab[NewHandle] != 0xff)
	{
		COUNT ret;

		if((ret = DosClose(NewHandle)) != SUCCESS)
			return ret;
	}

	/* If everything looks ok, bump it up.                          */
	if((Sftp -> sft_flags & SFT_FDEVICE) || (Sftp -> sft_status >= 0))
	{
		p -> ps_filetab[NewHandle] = p -> ps_filetab[OldHandle];
		Sftp -> sft_count += 1;
		return SUCCESS;
	}
	else
		return DE_INVLDACC;
}


COUNT 
DosOpen (BYTE FAR *fname, COUNT mode)
{
	psp FAR *p = MK_FP(cu_psp,0);
	WORD hndl, sft_idx;
	sft FAR *sftp;
	struct dhdr FAR *dhp;
	BYTE FAR *froot;
	BYTE buff[FNAME_SIZE+FEXT_SIZE];
	WORD i;

	/* test if mode is in range                     */
	if((mode & ~SFT_OMASK) != 0)
		return DE_INVLDACC;

	mode &= 3;         

	/* get a free handle                            */
	if((hndl = get_free_hndl()) == 0xff)
		return DE_TOOMANY;

	/* now het a free system file table entry       */
	if((sftp = get_free_sft((WORD FAR *)&sft_idx)) == (sft FAR *)-1)
		return DE_TOOMANY;

	/* check for a device                           */
	froot = get_root(fname);
	for(i = 0; i < FNAME_SIZE; i++)
	{
		if(*froot != '\0' && *froot != '.')
			buff[i] = *froot++;
		else
			break;
	}

	for( ; i < FNAME_SIZE; i++)
		buff[i] = ' ';

	/* if we have an extension, can't be a device   */
	if(*froot != '.');
	{
		for(dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1; dhp = dhp -> dh_next)
		{
			if(fnmatch((BYTE FAR *)buff, (BYTE FAR *)dhp -> dh_name, FNAME_SIZE, FALSE))
			{
				sftp -> sft_count += 1;
				sftp -> sft_mode = mode;
				sftp -> sft_attrib = 0;
				sftp -> sft_flags = 
				  (dhp -> dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF;
				sftp -> sft_psp = cu_psp;
				fbcopy((BYTE FAR *)buff, sftp -> sft_name, FNAME_SIZE+FEXT_SIZE);
				sftp -> sft_dev = dhp;
				p -> ps_filetab[hndl] = sft_idx;
				return hndl;
			}
		}
	}
	sftp -> sft_status = dos_open(fname, mode);
	if(sftp -> sft_status >= 0)
	{
		p -> ps_filetab[hndl] = sft_idx;
		sftp -> sft_count += 1;
		sftp -> sft_mode = mode;
		sftp -> sft_attrib = 0;
		sftp -> sft_flags = 0;
		sftp -> sft_psp = cu_psp;
		fbcopy((BYTE FAR *)buff, sftp -> sft_name, FNAME_SIZE+FEXT_SIZE);
		return hndl;
	}
	else
		return sftp -> sft_status;
}


COUNT 
DosClose (COUNT hndl)
{
	psp FAR *p = MK_FP(cu_psp,0);
	sft FAR *s;

	/* Test that the handle is valid                */
	if(hndl < 0)
		return DE_INVLDHNDL;

	/* Get the SFT block that contains the SFT      */
	if((s = get_sft(hndl)) == (sft FAR *)-1)
		return DE_INVLDHNDL;

	/* If this is not opened another error          */
	if(s -> sft_count == 0)
		return DE_ACCESS;

	/* now just drop the count if a device, else    */
	/* call file system handler                     */
	if(s -> sft_flags & SFT_FDEVICE)
	{
		p -> ps_filetab[hndl] = 0xff;
		s -> sft_count -= 1;
		return SUCCESS;
	}
	else
	{
		p -> ps_filetab[hndl] = 0xff;
		s -> sft_count -= 1;
		if(s -> sft_count > 0)
			return SUCCESS;
		else
			return dos_close(s -> sft_status);
	}
}

VOID 
DosGetFree (COUNT drive, COUNT FAR *spc, COUNT FAR *navc, COUNT FAR *bps, COUNT FAR *nc)
{
	struct dpb *dpbp;

	/* first check for valid drive          */
	if(drive < 0 || drive > NDEVS)
	{
		*spc = -1;
		return;
	}

	/* next - "log" in the drive            */
	drive = (drive == 0 ? default_drive : drive - 1);
	dpbp = &blk_devices[drive];
	++(dpbp -> dpb_count);
	dpbp -> dpb_flags = -1;
	if((media_check(dpbp) < 0) || (dpbp -> dpb_count <= 0))
	{
		*spc = -1;
		return;
	}

	/* get the data vailable from dpb       */
	*nc = (dpbp -> dpb_size - dpbp -> dpb_data + 1) / dpbp -> dpb_clssize;
	*spc = dpbp -> dpb_clssize;
	*bps = dpbp -> dpb_secsize;

	/* now tell fs to give us free cluster  */
	/* count                                */
	*navc = dos_free(dpbp);
	--(dpbp -> dpb_count);
}


COUNT 
DosGetCuDir (COUNT drive, BYTE FAR *s)
{
	REG struct dpb *dpbp;

	/* first check for valid drive          */
	if(drive < 0 || drive > NDEVS)
		return DE_INVLDDRV;

	/* next - "log" in the drive            */
	drive = (drive == 0 ? default_drive : drive - 1);
	dpbp = &blk_devices[drive];
	dpbp -> dpb_flags = -1;
	++(dpbp -> dpb_count);
	if((media_check(dpbp) < 0) || (dpbp -> dpb_count <= 0))
		return DE_INVLDDRV;

	/* now get fs to give current           */
	/* directory                            */
	dos_pwd(dpbp, s);
	--(dpbp -> dpb_count);
	return SUCCESS;
}


COUNT 
DosChangeDir (BYTE FAR *s)
{
	REG struct dpb *dpbp;
	REG COUNT drive;
	struct f_node FAR *fp;
	COUNT ret;

	/* test for path existance from fs      */
	if((fp = dir_open((BYTE FAR *)s)) == (struct f_node FAR *)0)
		return DE_PATHNOTFND;
	else
		dir_close(fp);

	/* Parse and extract drive              */
	if(*(s + 1) == ':')
	{
		drive = *s++ - '@';
		if (drive > 26) drive -= 'a' - 'A';
		++s;
	}
	else
		drive = 0;

	/* first check for valid drive          */
	if(drive < 0 || drive > NDEVS)
		return DE_INVLDDRV;

	/* next - "log" in the drive            */
	drive = (drive == 0 ? default_drive : drive - 1);
	dpbp = &blk_devices[drive];
	++(dpbp -> dpb_count);
	dpbp -> dpb_flags = -1;
	if((media_check(dpbp) < 0) || (dpbp -> dpb_count <= 0))
		return DE_INVLDDRV;

	/* now get fs to change to new          */
	/* directory                            */
	ret = dos_cd(dpbp, s);
	--(dpbp -> dpb_count);
	return ret;
}




COUNT 
DosFindFirst (UCOUNT attr, BYTE FAR *name)
{
	return dos_findfirst(attr, name);
}


COUNT 
DosFindNext (void)
{
	return dos_findnext();
}


#ifndef IPL
COUNT DosGetFtime(hndl, dp, tp)
COUNT hndl;
date FAR *dp;
time FAR *tp;
{
	sft FAR *s;
	sfttbl FAR *sp;

	/* Test that the handle is valid                */
	if(hndl < 0)
		return DE_INVLDHNDL;

	/* Get the SFT block that contains the SFT      */
	if((s = get_sft(hndl)) == (sft FAR *)-1)
		return DE_INVLDHNDL;

	/* If this is not opened another error          */
	if(s -> sft_count == 0)
		return DE_ACCESS;

	/* call file system handler                     */
	return dos_getftime(s -> sft_status, dp, tp);
}

COUNT DosSetFtime(hndl, dp, tp)
COUNT hndl;
date FAR *dp;
time FAR *tp;
{
	sft FAR *s;
	sfttbl FAR *sp;

	/* Test that the handle is valid                */
	if(hndl < 0)
		return DE_INVLDHNDL;

	/* Get the SFT block that contains the SFT      */
	if((s = get_sft(hndl)) == (sft FAR *)-1)
		return DE_INVLDHNDL;

	/* If this is not opened another error          */
	if(s -> sft_count == 0)
		return DE_ACCESS;

	/* call file system handler                     */
	return dos_setftime(s -> sft_status, dp, tp);
}
#endif

COUNT 
DosGetFattr (BYTE FAR *name, UWORD FAR *attrp)
{
	return dos_getfattr(name, attrp);
}


COUNT 
DosSetFattr (BYTE FAR *name, UWORD FAR *attrp)
{
	return dos_setfattr(name, attrp);
}


