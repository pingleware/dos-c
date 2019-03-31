
/****************************************************************/
/*                                                              */
/*                          fcbfns.c                            */
/*                                                              */
/*           Old CP/M Style Function Handlers for Kernel        */
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

/* $Logfile:   D:/dos-c/src/fs/fcbfns.c_v  $ */
#ifndef IPL
static BYTE *RcsId = "$Header:   D:/dos-c/src/fs/fcbfns.c_v   1.3   29 May 1996 21:15:14   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/fs/fcbfns.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:15:14   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   01 Sep 1995 17:48:44   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:06:06   patv
 * Initial revision.
 */

#define FCB_SUCCESS     0
#define FCB_ERR_NODATA  1
#define FCB_ERR_EOF     3
#define FCB_ERR_WRITE   1
#define D_ALL   D_NORMAL | D_RDONLY | D_HIDDEN | D_SYSTEM | D_DIR | D_ARCHIVE

#ifdef PROTO
fcb FAR *ExtFcbToFcb(xfcb FAR *lpExtFcb);
fcb FAR *CommonFcbInit(xfcb FAR *lpExtFcb, BYTE *pszBuffer, COUNT *pCurDrive);
void FcbNameInit(fcb FAR *lpFcb, BYTE *pszBuffer, COUNT *pCurDrive);
sft FAR *FcbGetSft(COUNT SftIndex);
VOID FcbNextRecord(fcb FAR *lpFcb);
sft FAR *FcbGetFreeSft(WORD FAR *sft_idx);
BOOL FcbFnameMatch(BYTE FAR *s, BYTE FAR *d, COUNT n, COUNT mode);
BOOL FcbCharMatch(COUNT s, COUNT d, COUNT mode);
BOOL FcbCalcRec(xfcb FAR *lpXfcb);
VOID MoveDirInfo(dmatch FAR *lpDmatch, struct dirent FAR *lpDir);
#else
fcb FAR *ExtFcbToFcb();
fcb FAR *CommonFcbInit();
void FcbNameInit();
sft FAR *FcbGetSft();
VOID FcbNextRecord();
sft FAR *FcbGetFreeSft();
BOOL FcbFnameMatch();
BOOL FcbCharMatch();
BOOL FcbCalcRec();
VOID MoveDirInfo();
#endif

VOID DosOutputString(BYTE FAR *s)
{
	while(*s != '$')
		DosCharOutput(*s++);
}

static BYTE *con_name = "CON";

int DosCharInputEcho(VOID)
{
	BYTE cb;
	request rq;

	rq.r_length = sizeof(request);
	rq.r_command = C_INPUT;
	rq.r_count = 1;
	rq.r_trans = (VOID FAR *)&cb;
	rq.r_status = 0;
	execrh((request FAR *)&rq, syscon);
	if(rq.r_status & S_ERROR)
		return char_error(&rq, con_name);
	DosCharOutput(cb);
	return cb;
}


int DosCharInput(VOID)
{
	BYTE cb;
	request rq;

	rq.r_length = sizeof(request);
	rq.r_command = C_INPUT;
	rq.r_count = 1;
	rq.r_trans = (VOID FAR *)&cb;
	rq.r_status = 0;
	execrh((request FAR *)&rq, syscon);
	if(rq.r_status & S_ERROR)
		return char_error(&rq, con_name);
	return cb;
}


VOID DosDirectConsoleIO(iregs FAR *r)
{
	request rq;
	BYTE buf;

	if(r -> DL == 0xff)
	{
		r -> FLAGS &= ~FLG_ZERO;

		rq.r_length = sizeof(request);
		rq.r_command = C_ISTAT;
		rq.r_status = 0;
		execrh((request FAR *)&rq, syscon);
		if(rq.r_status & S_ERROR)
		{
			char_error(&rq, con_name);
			return;
		}

		if(rq.r_status & S_BUSY)
		{
			rq.r_length = sizeof(request);
			rq.r_command = C_INPUT;
			rq.r_count = 1;
			rq.r_trans = (VOID FAR *)&buf;
			rq.r_status = 0;
			execrh((request FAR *)&rq, syscon);
			if(rq.r_status & S_ERROR)
			{
				char_error(&rq, con_name);
				return;
			}

			r -> AL = buf;
			r -> FLAGS |= FLG_ZERO;
		}

	}
	else
	{
		rq.r_length = sizeof(request);
		rq.r_command = C_OUTPUT;
		rq.r_count = 1;
		rq.r_trans = (VOID FAR *)(&buf);
		rq.r_status = 0;
		execrh((request FAR *)&rq, syscon);
		if(rq.r_status & S_ERROR)
			char_error(&rq, con_name);
	}
}

/* Console output with printer echo                                     */
VOID DosCharOutput(COUNT c)
{
	request rq;
	BYTE buf = c;

	/* Test for break first                                         */
	if(con_break())
		return;

	/* Now do an output directly to the console                     */
	rq.r_length = sizeof(request);
	rq.r_command = C_OUTPUT;
	rq.r_count = 1;
	rq.r_trans = (VOID FAR *)(&buf);
	rq.r_status = 0;
	execrh((request FAR *)&rq, syscon);
	if(rq.r_status & S_ERROR)
		char_error(&rq, con_name);
	++scr_pos;

	/* printer echo stuff                                   */
}


VOID DosDisplayOutput(COUNT c)
{
	/* Non-portable construct                                       */
	if(c < ' ' || c == 0x7f)
	{
		switch(c)
		{
		case '\r':
			scr_pos = 0;
			break;

		case 0x7f:
			++scr_pos;
			break;

		case '\b':
			if(scr_pos > 0)
				--scr_pos;
			break;

		case '\t':
			do
				DosCharOutput(' ');
			while(scr_pos & 7);
			return;

		default:
			break;
		}
		DosCharOutput(c);
	}
	else
	{
		DosCharOutput(c);
	}
}


VOID 
FatGetDrvData (COUNT drive, COUNT FAR *spc, COUNT FAR *bps, COUNT FAR *nc, BYTE FAR **mdp)
{
	struct dpb *dpbp;

	/* first check for valid drive                                  */
	if(drive < 0 || drive > NDEVS)
	{
		*spc = -1;
		return;
	}

	/* next - "log" in the drive                                    */
	drive = (drive == 0 ? default_drive : drive - 1);
	dpbp = &blk_devices[drive];
	++(dpbp -> dpb_count);
	dpbp -> dpb_flags = -1;
	if((media_check(dpbp) < 0) || (dpbp -> dpb_count <= 0))
	{
		*spc = -1;
		return;
	}

	/* get the data vailable from dpb                       */
	*nc = (dpbp -> dpb_size - dpbp -> dpb_data + 1) / dpbp -> dpb_clssize;
	*spc = dpbp -> dpb_clssize;
	*bps = dpbp -> dpb_secsize;

	/* Point to the media desctriptor fotr this drive               */
	*mdp = &(dpbp -> dpb_mdb);
	--(dpbp -> dpb_count);
}

#ifndef IPL
WORD FcbParseFname(wTestMode, lpFileName, lpFcb)
REG WORD wTestMode;
BYTE FAR **lpFileName;
fcb FAR *lpFcb;
{
	COUNT   nIndex;
	WORD    wRetCode = PARSE_RET_NOWILD;

	/* pjv -- ExtFcbToFcb?                                          */
	/* Start out with some simple stuff first.  Check if we are     */
	/* going to use a default drive specificaton.                   */
	if(!(wTestMode & PARSE_DFLT_DRIVE))
		lpFcb -> fcb_drive = FDFLT_DRIVE;
	if(!(wTestMode & PARSE_BLNK_FNAME))
	{
		for(nIndex = 0; nIndex < FNAME_SIZE; ++nIndex)
			lpFcb -> fcb_fname[nIndex] = ' ';
	}
	if(!(wTestMode & PARSE_BLNK_FEXT))
	{
		for(nIndex = 0; nIndex < FEXT_SIZE; ++nIndex)
			lpFcb -> fcb_fext[nIndex] = ' ';
	}

	/* Undocumented behavior, set record number & record size to 0  */
	lpFcb -> fcb_curec = lpFcb -> fcb_recsiz = 0;

	if(!(wTestMode & PARSE_SEP_STOP))
	{
		*lpFileName = ParseSkipWh(*lpFileName);
		if(TestCmnSeps(*lpFileName))
			++*lpFileName;
	}

	/* Undocumented "feature," we skip white space anyway           */
	*lpFileName = ParseSkipWh(*lpFileName);

	/* Now check for drive specification                            */
	if(*(*lpFileName + 1) == ':')
	{
		REG BYTE Drive = **lpFileName;

		/* non-portable construct to be changed                 */
		if(Drive < 'A' || Drive > 'Z')
			return PARSE_RET_BADDRIVE;
		Drive -= ('A' - 1);
		if(Drive > nblkdev)
			return PARSE_RET_BADDRIVE;
		else
			lpFcb -> fcb_drive = Drive;
		*lpFileName += 2;
	}

	/* Now to format the file name into the string                  */
	*lpFileName = GetNameField(*lpFileName, (BYTE FAR *)lpFcb -> fcb_fname, FNAME_SIZE, (BOOL *)&wRetCode);

	/* Do we have an extension? If do, format it else return        */
	if(**lpFileName == '.')
		*lpFileName = GetNameField(++*lpFileName, (BYTE FAR *)lpFcb -> fcb_fext, FEXT_SIZE, (BOOL *)&wRetCode);

	return wRetCode ? PARSE_RET_WILD : PARSE_RET_NOWILD;
}


BYTE FAR *
ParseSkipWh (BYTE FAR *lpFileName)
{
	while(*lpFileName == ' ' || *lpFileName == '\t')
		++lpFileName;
	return lpFileName;
}


BOOL 
TestCmnSeps (BYTE FAR *lpFileName)
{
	BYTE *pszTest, *pszCmnSeps = ":<|>+=,";

	for(pszTest = pszCmnSeps; *pszTest != '\0'; ++pszTest)
		if(*lpFileName == *pszTest)
			return TRUE;
	return FALSE;
}

BOOL 
TestFieldSeps (BYTE FAR *lpFileName)
{
	BYTE *pszTest, *pszCmnSeps = "/\"[]<>|.";

	/* Another non-portable construct                               */
	if(*lpFileName < ' ')
		return FALSE;
	for(pszTest = pszCmnSeps; *pszTest != '\0'; ++pszTest)
		if(*lpFileName == *pszTest)
			return TRUE;
	return FALSE;
}

BYTE FAR *
GetNameField (BYTE FAR *lpFileName, BYTE FAR *lpDestField, COUNT nFieldSize, BOOL *pbWildCard)
{
	COUNT nIndex = 0;
	BYTE cFill = ' ';

	*pbWildCard = FALSE;
	while(*lpFileName != '\0' && !TestFieldSeps(lpFileName) && nIndex< nFieldSize)
	{
		if(*lpFileName == ' ')
			break;
		if(*lpFileName == '*')
		{
			*pbWildCard = TRUE;
			cFill = '?';
			++lpFileName;
			break;
		}
		if(*lpFileName == '?')
			*pbWildCard = TRUE;
		*lpDestField++ = *lpFileName++;
		++nIndex;
	}

	/* Blank out remainder of field on exit                         */
	for( ; nIndex < nFieldSize; ++nIndex)
		*lpDestField++ = cFill;
	return lpFileName;
}

static sft FAR *FcbGetSft(SftIndex)
COUNT SftIndex;
{
	sfttbl FAR *lpSftEntry;

	/* Get the SFT block that contains the SFT      */
	for(lpSftEntry = sfthead; lpSftEntry != (sfttbl FAR *)-1;
	 lpSftEntry = lpSftEntry -> sftt_next)
	{
		if(SftIndex < lpSftEntry -> sftt_count)
			break;
		else
			SftIndex -= lpSftEntry -> sftt_count;
	}

	/* If not found, return an error                */
	if(lpSftEntry == (sfttbl FAR *)-1)
		return (sft FAR *)-1;

	/* finally, point to the right entry            */
	return (sft FAR *)&(lpSftEntry -> sftt_table[SftIndex]);
}


static VOID FcbNextRecord(lpFcb)
fcb FAR *lpFcb;
{
	if(++lpFcb -> fcb_curec > 128)
	{
		lpFcb -> fcb_curec = 0;
		++lpFcb -> fcb_cublock;
	}
}


BOOL FcbRead(lpXfcb, nErrorCode)
xfcb FAR *lpXfcb;
COUNT *nErrorCode;
{
	sft FAR *s;
	fcb FAR *lpFcb;
	LONG lPosit;
	COUNT nRead;
	psp FAR *p = MK_FP(cu_psp,0);

	/* Convert to fcb if necessary                                  */
	lpFcb = ExtFcbToFcb(lpXfcb);

	/* Get the SFT block that contains the SFT      */
	if((s = FcbGetSft(lpFcb -> fcb_sftno)) == (sft FAR *)-1)
		return FALSE;

	/* If this is not opened another error          */
	if(s -> sft_count == 0)
		return FALSE;

	/* Now update the fcb and compute where we need to position     */
	/* to.                                                          */
	lPosit = ((lpFcb -> fcb_cublock * 128) + lpFcb -> fcb_curec)
	 * lpFcb -> fcb_recsiz;
	if(dos_lseek(s -> sft_status, lPosit, 0) < 0)
	{
		*nErrorCode = FCB_ERR_EOF;
		return FALSE;
	}

	/* Do the read                                                  */
	nRead = dos_read(s -> sft_status,
	 p -> ps_dta, lpFcb -> fcb_recsiz);

	/* Now find out how we will return and do it.                   */
	if(nRead == lpFcb -> fcb_recsiz)
	{
		*nErrorCode = FCB_SUCCESS;
		FcbNextRecord(lpFcb);
		return TRUE;
	}
	else if(nRead < 0)
	{
		*nErrorCode = FCB_ERR_EOF;
		return TRUE;
	}
	else if(nRead == 0)
	{
		*nErrorCode = FCB_ERR_NODATA;
		return FALSE;
	}
	else
	{
		COUNT nIdx, nCount;
		BYTE FAR *lpDta;

		nCount = lpFcb -> fcb_recsiz - nRead;
		lpDta = (BYTE FAR *)&(p -> ps_dta[nRead]);
		for(nIdx = 0; nIdx < nCount; nIdx++)
			*lpDta++ = 0;
		*nErrorCode = FCB_ERR_EOF;
		FcbNextRecord(lpFcb);
		return FALSE;
	}
}

BOOL FcbWrite(lpXfcb, nErrorCode)
xfcb FAR *lpXfcb;
COUNT *nErrorCode;
{
	sft FAR *s;
	fcb FAR *lpFcb;
	LONG lPosit;
	COUNT nWritten;
	psp FAR *p = MK_FP(cu_psp,0);

	/* Convert to fcb if necessary                                  */
	lpFcb = ExtFcbToFcb(lpXfcb);

	/* Get the SFT block that contains the SFT      */
	if((s = FcbGetSft(lpFcb -> fcb_sftno)) == (sft FAR *)-1)
		return FALSE;

	/* If this is not opened another error          */
	if(s -> sft_count == 0)
		return FALSE;

	/* Now update the fcb and compute where we need to position     */
	/* to.                                                          */
	lPosit = ((lpFcb -> fcb_cublock * 128) + lpFcb -> fcb_curec)
	 * lpFcb -> fcb_recsiz;
	if(dos_lseek(s -> sft_status, lPosit, 0) < 0)
	{
		*nErrorCode = FCB_ERR_EOF;
		return FALSE;
	}

	/* Do the read                                                  */
	nWritten = dos_write(s -> sft_status,
	 p -> ps_dta, lpFcb -> fcb_recsiz);

	/* Now find out how we will return and do it.                   */
	if(nWritten == lpFcb -> fcb_recsiz)
	{
		lpFcb -> fcb_fsize = dos_getcufsize(s -> sft_status);
		FcbNextRecord(lpFcb);
		*nErrorCode = FCB_SUCCESS;
		return TRUE;
	}
	else if(nWritten <= 0)
	{
		*nErrorCode = FCB_ERR_WRITE;
		return TRUE;
	}
	*nErrorCode = FCB_ERR_WRITE;
	return FALSE;
}


BOOL FcbGetFileSize(lpXfcb)
xfcb FAR *lpXfcb;
{
	BYTE buff[FNAME_SIZE+FEXT_SIZE + 3];
	fcb FAR *lpFcb;
	COUNT FcbDrive, FileNum;

	/* Build a traditional DOS file name                            */
	lpFcb = CommonFcbInit(lpXfcb, buff, &FcbDrive);

	/* check for a device                                           */
	/* if we have an extension, can't be a device                   */
	if(IsDevice(buff) || (lpFcb -> fcb_recsiz == 0))
	{
		return FALSE;
	}
	FileNum = dos_open(buff, O_RDONLY);
	if(FileNum >= 0)
	{
		LONG fsize;

		/* Get the size                                         */
		fsize = dos_getfsize(FileNum);

		/* compute the size and update the fcb                  */
		lpFcb -> fcb_rndm = fsize / lpFcb -> fcb_recsiz;
		if((fsize % lpFcb -> fcb_recsiz) != 0)
			++lpFcb -> fcb_rndm;

		/* close the file and leave                             */
		return dos_close(FileNum) == SUCCESS;
	}
	else
		return FALSE;
}


BOOL FcbSetRandom(lpXfcb)
xfcb FAR *lpXfcb;
{
	fcb FAR *lpFcb;
	LONG lPosit;

	/* Convert to fcb if necessary                                  */
	lpFcb = ExtFcbToFcb(lpXfcb);

	/* Now update the fcb and compute where we need to position     */
	/* to.                                                          */
	lpFcb -> fcb_rndm = (lpFcb -> fcb_cublock * 128)
	 + lpFcb -> fcb_curec;

	return TRUE;
}


BOOL FcbCalcRec(lpXfcb)
xfcb FAR *lpXfcb;
{
	fcb FAR *lpFcb;
	LONG lPosit;

	/* Convert to fcb if necessary                                  */
	lpFcb = ExtFcbToFcb(lpXfcb);

	/* Now update the fcb and compute where we need to position     */
	/* to.                                                          */
	lpFcb -> fcb_cublock =  lpFcb -> fcb_rndm / 128;
	lpFcb -> fcb_curec =  lpFcb -> fcb_rndm % 128;

	return TRUE;
}


BOOL FcbRandomBlockRead(lpXfcb, nRecords, nErrorCode)
xfcb FAR *lpXfcb;
COUNT nRecords;
COUNT *nErrorCode;
{
	fcb FAR *lpFcb;

	FcbCalcRec(lpXfcb);

	/* Convert to fcb if necessary                                  */
	lpFcb = ExtFcbToFcb(lpXfcb);

	do
		FcbRead(lpXfcb, nErrorCode);
	while ((--nRecords > 0) && (*nErrorCode == 0));

	/* Now update the fcb                                           */
	lpFcb -> fcb_rndm = lpFcb -> fcb_cublock * 128 + lpFcb -> fcb_curec;

	return TRUE;
}


BOOL FcbRandomBlockWrite(lpXfcb, nRecords, nErrorCode)
xfcb FAR *lpXfcb;
COUNT nRecords;
COUNT *nErrorCode;
{
	fcb FAR *lpFcb;

	FcbCalcRec(lpXfcb);

	/* Convert to fcb if necessary                                  */
	lpFcb = ExtFcbToFcb(lpXfcb);

	do
		FcbWrite(lpXfcb, nErrorCode);
	while ((--nRecords > 0) && (*nErrorCode == 0));

	/* Now update the fcb                                           */
	lpFcb -> fcb_rndm = lpFcb -> fcb_cublock * 128 + lpFcb -> fcb_curec;

	return TRUE;
}


BOOL FcbRandomRead(lpXfcb, nErrorCode)
xfcb FAR *lpXfcb;
COUNT *nErrorCode;
{
	UWORD uwCurrentBlock;
	UBYTE ucCurrentRecord;
	fcb FAR *lpFcb;

	FcbCalcRec(lpXfcb);

	/* Convert to fcb if necessary                                  */
	lpFcb = ExtFcbToFcb(lpXfcb);

	uwCurrentBlock = lpFcb -> fcb_cublock;
	ucCurrentRecord = lpFcb -> fcb_curec;

	FcbRead(lpXfcb, nErrorCode);

	lpFcb -> fcb_cublock = uwCurrentBlock;
	lpFcb -> fcb_curec = ucCurrentRecord;
	return TRUE;
}


BOOL FcbRandomWrite(lpXfcb, nErrorCode)
xfcb FAR *lpXfcb;
COUNT *nErrorCode;
{
	UWORD uwCurrentBlock;
	UBYTE ucCurrentRecord;
	fcb FAR *lpFcb;

	FcbCalcRec(lpXfcb);

	/* Convert to fcb if necessary                                  */
	lpFcb = ExtFcbToFcb(lpXfcb);

	uwCurrentBlock = lpFcb -> fcb_cublock;
	ucCurrentRecord = lpFcb -> fcb_curec;

	FcbWrite(lpXfcb, nErrorCode);

	lpFcb -> fcb_cublock = uwCurrentBlock;
	lpFcb -> fcb_curec = ucCurrentRecord;
	return TRUE;
}


static sft FAR *FcbGetFreeSft(sft_idx)
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


BOOL FcbCreate(lpXfcb)
xfcb FAR *lpXfcb;
{
	WORD sft_idx;
	sft FAR *sftp;
	struct dhdr FAR *dhp;
	BYTE buff[FNAME_SIZE+FEXT_SIZE + 3];
	fcb FAR *lpFcb;
	COUNT FcbDrive;

	/* get a free system file table entry                           */
	if((sftp = FcbGetFreeSft((WORD FAR *)&sft_idx)) == (sft FAR *)-1)
		return DE_TOOMANY;

	/* Build a traditional DOS file name                            */
	lpFcb = CommonFcbInit(lpXfcb, buff, &FcbDrive);

	/* check for a device                                           */
	/* if we have an extension, can't be a device                   */
	if(IsDevice(buff))
	{
		for(dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1; dhp = dhp -> dh_next)
		{
			if(FcbFnameMatch((BYTE FAR *)buff, (BYTE FAR *)dhp -> dh_name, FNAME_SIZE, FALSE))
			{
				sftp -> sft_count += 1;
				sftp -> sft_mode = O_RDWR;
				sftp -> sft_attrib = 0;
				sftp -> sft_flags =
				  (dhp -> dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF;
				sftp -> sft_psp = cu_psp;
				fbcopy(lpFcb -> fcb_fname, sftp -> sft_name, FNAME_SIZE+FEXT_SIZE);
				sftp -> sft_dev = dhp;
				lpFcb -> fcb_sftno = sft_idx;
				lpFcb -> fcb_curec = 0;
				lpFcb -> fcb_recsiz = 0;
				lpFcb -> fcb_fsize = 0;
				lpFcb -> fcb_date = dos_getdate();
				lpFcb -> fcb_time = dos_gettime();
				lpFcb -> fcb_rndm = 0;
				return TRUE;
			}
		}
	}
	sftp -> sft_status = dos_creat(buff, 0);
	if(sftp -> sft_status >= 0)
	{
		lpFcb -> fcb_drive = FcbDrive;
		lpFcb -> fcb_sftno = sft_idx;
		lpFcb -> fcb_curec = 0;
		lpFcb -> fcb_recsiz = 128;
		lpFcb -> fcb_fsize = 0;
		lpFcb -> fcb_date = dos_getdate();
		lpFcb -> fcb_time = dos_gettime();
		lpFcb -> fcb_rndm = 0;
		sftp -> sft_count += 1;
		sftp -> sft_mode = O_RDWR;
		sftp -> sft_attrib = 0;
		sftp -> sft_flags = 0;
		sftp -> sft_psp = cu_psp;
		fbcopy((BYTE FAR *)&lpFcb -> fcb_fname, (BYTE FAR *)&sftp -> sft_name, FNAME_SIZE+FEXT_SIZE);
		return TRUE;
	}
	else
		return FALSE;
}


static fcb FAR *ExtFcbToFcb(xfcb FAR *lpExtFcb)
{
	if(*((UBYTE FAR *)lpExtFcb) == 0xff)
		return &lpExtFcb -> xfcb_fcb;
	else
		return (fcb FAR *)lpExtFcb;
}


static fcb FAR *CommonFcbInit(lpExtFcb, pszBuffer, pCurDrive)
xfcb FAR *lpExtFcb;
BYTE *pszBuffer;
COUNT *pCurDrive;
{
	BYTE FAR *lpszFcbFname, *lpszFcbFext;
	BYTE buff[FNAME_SIZE+FEXT_SIZE + 3];
	COUNT nDrvIdx, nFnameIdx, nFextIdx;
	fcb FAR *lpFcb;

	/* convert to fcb if needed first                               */
	lpFcb = ExtFcbToFcb(lpExtFcb);

	/* Build a traditional DOS file name                            */
	FcbNameInit(lpFcb, pszBuffer, pCurDrive);

	/* and return the fcb pointer                                   */
	return lpFcb;
}

void FcbNameInit(lpFcb, pszBuffer, pCurDrive)
fcb FAR *lpFcb;
BYTE *pszBuffer;
COUNT *pCurDrive;
{
	BYTE FAR *lpszFcbFname, FAR *lpszFcbFext;
	BYTE buff[FNAME_SIZE+FEXT_SIZE+4];  /* "a:" + '.' + 0 */
	COUNT nDrvIdx, nFnameIdx, nFextIdx;

	/* Build a traditional DOS file name                            */
	lpszFcbFname = (BYTE FAR *)lpFcb -> fcb_fname;
	if(lpFcb -> fcb_drive != 0)
	{
		*pCurDrive = lpFcb -> fcb_drive;
		pszBuffer[0] = 'A' + lpFcb -> fcb_drive - 1;
		pszBuffer[1] = ':';
		nDrvIdx = 2;
	}
	else
	{
		*pCurDrive = default_drive + 1;
		nDrvIdx = 0;
	}

	for(nFnameIdx = 0; nFnameIdx < FNAME_SIZE; nFnameIdx++)
	{
		if(*lpszFcbFname != ' ')
			pszBuffer[nDrvIdx + nFnameIdx] = *lpszFcbFname++;
		else
			break;
	}

	lpszFcbFext = (BYTE FAR *)lpFcb -> fcb_fext;
	if(*lpszFcbFext != ' ')
	{
		pszBuffer[nDrvIdx + nFnameIdx++] = '.';
		for(nFextIdx = 0; nFextIdx < FEXT_SIZE; nFextIdx++)
		{
			if(*lpszFcbFext != ' ')
				pszBuffer[nDrvIdx + nFnameIdx + nFextIdx] =
				 *lpszFcbFext++;
			else
				break;
		}
	}
	else
		nFextIdx = 0;
	pszBuffer[nDrvIdx + nFnameIdx + nFextIdx] = '\0';
}

/* Ascii only file name match routines                  */
static BOOL 
FcbCharMatch (COUNT s, COUNT d, COUNT mode)
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
FcbFnameMatch (BYTE FAR *s, BYTE FAR *d, COUNT n, COUNT mode)
{
	while(n--)
	{
		if(!FcbCharMatch(*s++, *d++, mode))
			return FALSE;
	}
	return TRUE;
}


BOOL FcbOpen(lpXfcb)
xfcb FAR *lpXfcb;
{
	WORD sft_idx;
	sft FAR *sftp;
	struct dhdr FAR *dhp;
	BYTE buff[FNAME_SIZE+FEXT_SIZE + 3];
	fcb FAR *lpFcb;
	COUNT FcbDrive;

	/* get a free system file table entry                           */
	if((sftp = FcbGetFreeSft((WORD FAR *)&sft_idx)) == (sft FAR *)-1)
		return DE_TOOMANY;

	/* Build a traditional DOS file name                            */
	lpFcb = CommonFcbInit(lpXfcb, buff, &FcbDrive);

	/* check for a device                                           */
	/* if we have an extension, can't be a device                   */
	if(IsDevice(buff))
	{
		for(dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1; dhp = dhp -> dh_next)
		{
			if(FcbFnameMatch((BYTE FAR *)buff, (BYTE FAR *)dhp -> dh_name, FNAME_SIZE, FALSE))
			{
				sftp -> sft_count += 1;
				sftp -> sft_mode = O_RDWR;
				sftp -> sft_attrib = 0;
				sftp -> sft_flags =
				  (dhp -> dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF;
				sftp -> sft_psp = cu_psp;
				fbcopy(lpFcb -> fcb_fname, sftp -> sft_name, FNAME_SIZE+FEXT_SIZE);
				sftp -> sft_dev = dhp;
				lpFcb -> fcb_sftno = sft_idx;
				lpFcb -> fcb_curec = 0;
				lpFcb -> fcb_recsiz = 0;
				lpFcb -> fcb_fsize = 0;
				lpFcb -> fcb_date = dos_getdate();
				lpFcb -> fcb_time = dos_gettime();
				lpFcb -> fcb_rndm = 0;
				return TRUE;
			}
		}
	}
	sftp -> sft_status = dos_open(buff, O_RDWR);
	if(sftp -> sft_status >= 0)
	{
		lpFcb -> fcb_drive = FcbDrive;
		lpFcb -> fcb_sftno = sft_idx;
		lpFcb -> fcb_curec = 0;
		lpFcb -> fcb_recsiz = 128;
		lpFcb -> fcb_fsize = dos_getfsize(sftp -> sft_status);
		dos_getftime(sftp -> sft_status,
		 (date FAR *)&lpFcb -> fcb_date,
		 (time FAR *)&lpFcb -> fcb_time);
		lpFcb -> fcb_rndm = 0;
		sftp -> sft_count += 1;
		sftp -> sft_mode = O_RDWR;
		sftp -> sft_attrib = 0;
		sftp -> sft_flags = 0;
		sftp -> sft_psp = cu_psp;
		fbcopy((BYTE FAR *)&lpFcb -> fcb_fname, (BYTE FAR *)&sftp -> sft_name, FNAME_SIZE+FEXT_SIZE);
		return TRUE;
	}
	else
		return FALSE;
}


BOOL FcbDelete(lpXfcb)
xfcb FAR *lpXfcb;
{
	BYTE buff[FNAME_SIZE+FEXT_SIZE + 3];
	COUNT FcbDrive;

	/* Build a traditional DOS file name                            */
	CommonFcbInit(lpXfcb, buff, &FcbDrive);

	/* check for a device                                           */
	/* if we have an extension, can't be a device                   */
	if(IsDevice(buff))
	{
		return FALSE;
	}
	else
	{
		BYTE FAR *lpOldDta = dta;
		dmatch Dmatch;

		dta = (BYTE FAR *)&Dmatch;
		if(dos_findfirst(D_ALL, buff[1] == ':' ? &buff[2] : buff) != SUCCESS)
		{
			dta = lpOldDta;
			return FALSE;
		}
		do
		{
			if(dos_delete(Dmatch.dm_name) != SUCCESS)
			{
				dta = lpOldDta;
				return FALSE;
			}
		}
		while(dos_findnext() == SUCCESS);
		dta = lpOldDta;
		return TRUE;
	}
}


BOOL FcbRename(lpXfcb)
xfcb FAR *lpXfcb;
{
	BYTE buff[FNAME_SIZE+FEXT_SIZE+4];
	rfcb FAR *lpRenameFcb;
	COUNT FcbDrive;

	/* Build a traditional DOS file name                            */
	lpRenameFcb = (rfcb FAR *)CommonFcbInit(lpXfcb, buff, &FcbDrive);

	/* check for a device                                           */
	/* if we have an extension, can't be a device                   */
	if(IsDevice(buff))
	{
		return FALSE;
	}
	else
	{
		BYTE FAR *lpOldDta = dta;
		dmatch Dmatch;

		dta = (BYTE FAR *)&Dmatch;
		if(dos_findfirst(D_ALL, buff[1] == ':' ? &buff[2] : buff) != SUCCESS)
		{
			dta = lpOldDta;
			return FALSE;
		}


		do
		{
			BYTE ToName[FNAME_SIZE + FEXT_SIZE + 2]; /* '.' & 0 */
			fcb LocalFcb;
			BYTE *pToName, *pszFrom;
			BYTE FAR *pFromPattern;
			COUNT nIndex;

			/* First, expand the find match into fcb style  */
			/* file name entry                              */
			/* Fill with blanks first                       */
			for(pToName = LocalFcb.fcb_fname, nIndex = 0;
			 nIndex < FNAME_SIZE; nIndex++)
			{
				*pToName++ = ' ';
			}
			for(pToName = LocalFcb.fcb_fext, nIndex = 0;
			 nIndex < FEXT_SIZE; nIndex++)
			{
				*pToName++ = ' ';
			}

			/* next move in the file name while overwriting */
			/* the filler blanks                            */
			pszFrom = Dmatch.dm_name;
			pToName = LocalFcb.fcb_fname;
			for(nIndex = 0; nIndex < FNAME_SIZE; nIndex++)
			{
				if(*pszFrom != 0 && *pszFrom != '.')
					*pToName++ = *pszFrom++;
				else if(*pszFrom == '.')
				{
					++pszFrom;
					break;
				}
				else
					break;
			}

			if(*pszFrom != '\0')
			{
				pToName = LocalFcb.fcb_fext;
				for(nIndex = 0; nIndex < FEXT_SIZE; nIndex++)
				{
					if(*pszFrom != '\0')
						*pToName++ = *pszFrom++;
					else
						break;
				}
			}

			/* Overlay the pattern, skipping '?'            */
			/* I'm cheating because this assumes that the   */
			/* struct alignments are on byte boundaries     */
			pToName = LocalFcb.fcb_fname;
			for(pFromPattern = lpRenameFcb -> renNewName,
			 nIndex = 0; nIndex < FNAME_SIZE + FEXT_SIZE; nIndex++)
			{
				if(*pFromPattern != '?')
					*pToName++ = *pFromPattern++;
				else
					++pFromPattern;
			}

			/* now to build a dos name again                */
			LocalFcb.fcb_drive = 0;
			FcbNameInit((fcb FAR *)&LocalFcb, buff, &FcbDrive);

			if(dos_rename(Dmatch.dm_name,
			 buff[1] == ':' ? &buff[2] : buff) != SUCCESS)
			{
				dta = lpOldDta;
				return FALSE;
			}
		}
		while(dos_findnext() == SUCCESS);
		dta = lpOldDta;
		return TRUE;
	}
}


void MoveDirInfo(lpDmatch, lpDir)
dmatch FAR *lpDmatch;
struct dirent FAR *lpDir;
{
	BYTE FAR *lpToName, FAR *lpszFrom;
	COUNT nIndex;

	/* First, expand the find match into dir style  */
	/* file name entry                              */
	/* Fill with blanks first                       */
	for(lpToName = lpDir -> dir_name, nIndex = 0;
	 nIndex < FNAME_SIZE; nIndex++)
	{
		*lpToName++ = ' ';
	}
	for(lpToName = lpDir -> dir_ext, nIndex = 0;
	 nIndex < FEXT_SIZE; nIndex++)
	{
		*lpToName++ = ' ';
	}

	/* next move in the file name while overwriting */
	/* the filler blanks                            */
	lpszFrom = lpDmatch -> dm_name;
	lpToName = lpDir -> dir_name;
	for(nIndex = 0; nIndex < FNAME_SIZE; nIndex++)
	{
		if(*lpszFrom != 0 && *lpszFrom != '.')
			*lpToName++ = *lpszFrom++;
		else
			break;
	}

	if(*lpszFrom != '\0')
	{
		if(*lpszFrom == '.')
			++lpszFrom;
		lpToName = lpDir -> dir_ext;
		for(nIndex = 0; nIndex < FEXT_SIZE; nIndex++)
		{
			if(*lpszFrom != '\0')
				*lpToName++ = *lpszFrom++;
			else
				break;
		}
	}
	for(nIndex = 0; nIndex < 10; nIndex++)
		lpDir -> dir_reserved[nIndex] = 0;
	lpDir -> dir_attrib = lpDmatch ->dm_attr_fnd;
	lpDir -> dir_time = lpDmatch ->dm_time;
	lpDir -> dir_date = lpDmatch ->dm_date;
	lpDir -> dir_start = lpDmatch ->dm_cluster;
	lpDir -> dir_size = lpDmatch ->dm_size;
}


BOOL FcbClose(lpXfcb)
xfcb FAR *lpXfcb;
{
	sft FAR *s;
	fcb FAR *lpFcb;

	/* Convert to fcb if necessary                                  */
	lpFcb = ExtFcbToFcb(lpXfcb);

	/* Get the SFT block that contains the SFT      */
	if((s = FcbGetSft(lpFcb -> fcb_sftno)) == (sft FAR *)-1)
		return FALSE;

	/* If this is not opened another error          */
	if(s -> sft_count == 0)
		return FALSE;

	/* now just drop the count if a device, else    */
	/* call file system handler                     */
	if(s -> sft_flags & SFT_FDEVICE)
	{
		s -> sft_count -= 1;
		return TRUE;
	}
	else
	{
		s -> sft_count -= 1;
		if(s -> sft_count > 0)
			return SUCCESS;
		else
		{
			/* change time and set file size                */
			dos_setftime(s -> sft_status,
			 (date FAR *)&lpFcb -> fcb_date,
			 (time FAR *)&lpFcb -> fcb_time);
			dos_setfsize(s -> sft_status,
			 lpFcb -> fcb_fsize);
			return dos_close(s -> sft_status) == SUCCESS;
		}
	}
}


BOOL FcbFindFirst(lpXfcb)
xfcb FAR *lpXfcb;
{
	BYTE FAR *lpOldDta;
	dmatch Dmatch;
	struct dirent FAR *lpDir;
	COUNT nIdx, FcbDrive;
	fcb FAR *lpFcb;
	BYTE Buffer[FNAME_SIZE+FEXT_SIZE+4];
	WORD wAttr;
	psp FAR *lpPsp = MK_FP(cu_psp,0);

	/* First, move the dta to a local and change it around to match */
	/* our functions.                                               */
	lpDir = (struct dirent FAR *)dta;
	dta = (BYTE FAR *)&Dmatch;

	/* Next initialze local variables by moving them from the fcb   */
	lpFcb = CommonFcbInit(lpXfcb, Buffer, &FcbDrive);
	if(lpXfcb -> xfcb_flag == 0xff)
		wAttr = lpXfcb -> xfcb_attrib;
	else
		wAttr = D_ALL;

	if(dos_findfirst(wAttr, Buffer[1] == ':' ? &Buffer[2] : Buffer) != SUCCESS)
	{
		dta = lpPsp -> ps_dta;
		return FALSE;
	}

	MoveDirInfo((dmatch FAR *)&Dmatch, lpDir);
	lpFcb -> fcb_dirclst = Dmatch.dm_cluster;
	lpFcb -> fcb_diroff = Dmatch.dm_entry;
	dta = lpPsp -> ps_dta;
	return TRUE;
}


BOOL FcbFindNext(lpXfcb)
xfcb FAR *lpXfcb;
{
	BYTE FAR *lpOldDta;
	dmatch Dmatch;
	struct dirent FAR *lpDir;
	COUNT nIdx, FcbDrive;
	fcb FAR *lpFcb;
	BYTE Buffer[FNAME_SIZE+FEXT_SIZE+4];
	WORD wAttr;
	psp FAR *lpPsp = MK_FP(cu_psp,0);

	/* First, move the dta to a local and change it around to match */
	/* our functions.                                               */
	lpDir = (struct dirent FAR *)dta;
	dta = (BYTE FAR *)&Dmatch;

	/* Next initialze local variables by moving them from the fcb   */
	lpFcb = CommonFcbInit(lpXfcb, Buffer, &FcbDrive);
	if((xfcb FAR *)lpFcb != lpXfcb)
		wAttr = lpXfcb -> xfcb_attrib;
	else
		wAttr = D_ALL;

	/* Reconstrct the dirmatch structure from the fcb               */
	Dmatch.dm_drive = FcbDrive;
	fbcopy(lpFcb -> fcb_fname, (BYTE FAR *)Dmatch.dm_name_pat, FNAME_SIZE+FEXT_SIZE);
	upMem((BYTE FAR *)Dmatch.dm_name_pat, FNAME_SIZE+FEXT_SIZE);
	Dmatch.dm_attr_srch = wAttr;
	Dmatch.dm_entry = lpFcb -> fcb_diroff;
	Dmatch.dm_cluster = lpFcb -> fcb_dirclst;

	if(dos_findnext() != SUCCESS)
	{
		dta = lpPsp -> ps_dta;
		return FALSE;
	}

	MoveDirInfo((dmatch FAR *)&Dmatch, lpDir);
	lpFcb -> fcb_dirclst = Dmatch.dm_cluster;
	lpFcb -> fcb_diroff = Dmatch.dm_entry;
	dta = lpPsp -> ps_dta;
	return TRUE;
}
#endif

