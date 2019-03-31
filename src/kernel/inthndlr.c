
/****************************************************************/
/*                                                              */
/*                          inthndlr.c                          */
/*                                                              */
/*    Interrupt Handler and Function dispatcher for Kernel      */
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
/*                                                              */
/****************************************************************/

#include "../../hdr/portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
BYTE *RcsId = "$Header:   D:/dos-c/src/kernel/inthndlr.c_v   1.3   29 May 1996 21:03:40   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/inthndlr.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:03:40   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   19 Feb 1996  3:21:48   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.1   01 Sep 1995 17:54:20   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995  8:33:34   patv
 * Initial revision.
 */

int DosMktmp(BYTE FAR *pathname, UWORD attr);

/* Terminate the current process                                        */
#pragma argsused
VOID INRPT far 
int20_handler (int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags)
{
	tsr = FALSE;
	InDOS++;
	DosMemCheck();
	return_user();
}

/* Convert an interrupt into our register struct definition             */
VOID INRPT far 
int21_handler (iregs UserRegs)
{
	static COUNT FAR *stack;
	
	++InDOS;
	user_r = (iregs far *)&UserRegs;
	stack = user_r -> AH < 0x0c ? &char_api_tos : &disk_api_tos;
     
	/* Initialize stacks                                            */
	api_sp = FP_OFF(stack);
	api_ss = FP_SEG(stack);
		
	set_stack();
	int21_service(user_r);
	restore_stack();
	--InDOS;
}

/* Special entry for far call into the kernel                           */
#pragma argsused
VOID FAR 
int21_entry (iregs UserRegs)
{
	int21_handler(UserRegs);
}


VOID int21_service(iregs far *r)
{
	COUNT rc, rc1;
	ULONG lrc;
	BYTE FAR *xp;

	/* The dispatch handler                                         */
#ifdef DEBUG
	if(bDumpRegs)
	{
		fbcopy((VOID FAR *)r, (VOID FAR *)&error_regs, sizeof(iregs));
		printf("System call (21h): %02x\n", r -> AX);
		dump_regs = TRUE;
		dump();
	}
#endif
	switch(r -> AH)
	{
	/* print unused and fall into terminate (debug only)            */
#ifdef DEBUG
		fbcopy((VOID FAR *)r, (VOID FAR *)&error_regs, sizeof(iregs));
		printf("Invalid system call (21h)\n");
		dump_regs = TRUE;
		dump();
# ifdef KDB
		break;
# endif
#else
		r -> AX = -rc;
		r -> FLAGS |= FLG_CARRY;
		break;
#endif

	/* Terminate Program                                            */
	case 0x00:
		tsr = FALSE;
		return_mode = break_flg ? 1 : 0;
		return_code = r -> AL;
		DosMemCheck();
		return_user();
		break;

	/* Read Keyboard with Echo                                              */
	case 0x01:
		r -> AL = DosCharInputEcho();
		break;

	/* Display Character                                            */
	case 0x02:
		DosDisplayOutput(r -> DL);
		break;

	/* Auxiliary Input                                                      */
	case 0x03:
		r -> AL = _sti();
		break;

	/* Auxiliary Output                                                     */
	case 0x04:
		sto(r -> DL);
		break;

	/* Print Character                                                      */
	case 0x05:
		sto(r -> DL);
		break;

	/* Direct Cosole I/O                                            */
	case 0x06:
		DosDirectConsoleIO(r);
		break;

	/* Direct Console Input                                         */
	case 0x07:
		r -> AL = DosCharInput();
		break;

	/* Read Keyboard Without Echo                                   */
	case 0x08:
		r -> AL = _sti();
		break;

	/* Display String                                               */
	case 0x09:
		DosOutputString(MK_FP(r -> DS, r -> DX));
		break;

	/* Buffered Keyboard Input                                      */
	case 0x0a:
		sti((keyboard FAR *)MK_FP(r -> DS, r -> DX));
		((keyboard FAR *)MK_FP(r -> DS, r -> DX)) -> kb_count -= 2;
		break;

	/* Check Keyboard Status                                        */
	case 0x0b:
		if(KbdBusy())
			r -> AL = 0x00;
		else
			r -> AL = 0xff;
		
		break;

	/* Flush Buffer, Read Keayboard                                 */
	case 0x0c:
		KbdFlush();
		switch(r -> AL)
		{
			/* Read Keyboard with Echo                      */
			case 0x01:
				r -> AL = DosCharInputEcho();
				break;

			/* Direct Cosole I/O                            */
			case 0x06:
				DosDirectConsoleIO(r);
				break;

			/* Direct Console Input                         */
			case 0x07:
				r -> AL = DosCharInput();
				break;

			/* Read Keyboard Without Echo                   */
			case 0x08:
				r -> AL = _sti();
				break;

			/* Buffered Keyboard Input                      */
			case 0x0a:
				sti((keyboard FAR *)MK_FP(r -> DS, r -> DX));
				break;

			default:
				r -> AL = 0x00;
				break;
		}
		break;

	/* Reset Drive                                                  */
	case 0x0d:
		flush();
		break;

	/* Set Default Drive                                            */
	case 0x0e:
		if(((COUNT)r -> DL) >= 0 && r -> DL < nblkdev)
			default_drive = r -> DL;
		
		r -> AL = nblkdev;            
		break;

	case 0x0f:
		if(FcbOpen(MK_FP(r -> DS, r -> DX)))
			r -> AL = 0;
		else
			r -> AL = 0xff;
		break;

	case 0x10:
		if(FcbClose(MK_FP(r -> DS, r -> DX)))
			r -> AL = 0;
		else
			r -> AL = 0xff;
		break;

	case 0x11:
		if(FcbFindFirst(MK_FP(r -> DS, r -> DX)))
			r -> AL = 0;
		else
			r -> AL = 0xff;
		break;

	case 0x12:
		if(FcbFindNext(MK_FP(r -> DS, r -> DX)))
			r -> AL = 0;
		else
			r -> AL = 0xff;
		break;

	case 0x13:
		if(FcbDelete(MK_FP(r -> DS, r -> DX)))
			r -> AL = 0;
		else
			r -> AL = 0xff;
		break;

	case 0x14:
		{
			COUNT nErrorCode;

			if(FcbRead(MK_FP(r -> DS, r -> DX), &nErrorCode))
				r -> AL = 0;
			else
				r -> AL = nErrorCode;
			break;
		}

	case 0x15:
		{
			COUNT nErrorCode;

			if(FcbWrite(MK_FP(r -> DS, r -> DX), &nErrorCode))
				r -> AL = 0;
			else
				r -> AL = nErrorCode;
			break;
		}

	case 0x16:
		if(FcbCreate(MK_FP(r -> DS, r -> DX)))
			r -> AL = 0;
		else
			r -> AL = 0xff;
		break;

	case 0x17:
		if(FcbRename(MK_FP(r -> DS, r -> DX)))
			r -> AL = 0;
		else
			r -> AL = 0xff;
		break;

	/* CP/M compatibility functions                                 */
	case 0x18:
	case 0x1d:
	case 0x1e:
	case 0x20:
		r -> AL = 0;
		break;

	/* Get Default Drive                                            */
	case 0x19:
		r -> AL = default_drive;
		break;

	/* Set DTA                                                      */
	case 0x1a:
		{
			psp FAR *p = MK_FP(cu_psp,0);

			p -> ps_dta = MK_FP(r -> DS, r -> DX);
			dos_setdta(p -> ps_dta);
		}
		break;

	/* Get Default Drive Data                                       */
	case 0x1b:
		{
			BYTE FAR *p;

			FatGetDrvData(0,
			 (COUNT FAR *)&r -> AX,
			 (COUNT FAR *)&r -> CX,
			 (COUNT FAR *)&r -> DX,
			 (BYTE FAR **)&p);
			r -> DS = FP_SEG(p);
			r -> BX = FP_OFF(p);
		}
		break;

	/* Get Drive Data                                               */
	case 0x1c:
		{
			BYTE FAR *p;

			FatGetDrvData(r -> DL,
			 (COUNT FAR *)&r -> AX,
			 (COUNT FAR *)&r -> CX,
			 (COUNT FAR *)&r -> DX,
			 (BYTE FAR **)&p);
			r -> DS = FP_SEG(p);
			r -> BX = FP_OFF(p);
		}
		break;

	/* Get default DPB                                              */
	case 0x1f:
		r -> AH = 0xff;
		break;

	case 0x21:
		{
			COUNT nErrorCode;

			if(FcbRandomRead(MK_FP(r -> DS, r -> DX), &nErrorCode))
				r -> AL = 0;
			else
				r -> AL = nErrorCode;
			break;
		}

	case 0x22:
		{
			COUNT nErrorCode;

			if(FcbRandomWrite(MK_FP(r -> DS, r -> DX), &nErrorCode))
				r -> AL = 0;
			else
				r -> AL = nErrorCode;
			break;
		}

	/* Get file size in records                                     */
	case 0x23:
		if(FcbGetFileSize(MK_FP(r -> DS, r -> DX)))
			r -> AL = 0;
		else
			r -> AL = 0xff;
		break;

	case 0x24:
		FcbSetRandom(MK_FP(r -> DS, r -> DX));
		break;

	/* Set Interrupt Vector                                         */
	case 0x25:
		{
			VOID (INRPT FAR *p)() = MK_FP(r -> DS, r -> DX);

			
			setvec(r -> AL, p);
		}
		break;

	/* Dos Create New Psp                                           */
	case 0x26:
		{
			psp FAR *p = MK_FP(cu_psp, 0);

			new_psp((psp FAR *)MK_FP(r -> DX, 0), p -> ps_size);
		}
		break;

	case 0x27:
		{
			COUNT nErrorCode;

			if(FcbRandomBlockRead(MK_FP(r -> DS, r -> DX), r -> CX, &nErrorCode))
				r -> AL = 0;
			else
				r -> AL = nErrorCode;
			break;
		}

	case 0x28:
		{
			COUNT nErrorCode;

			if(FcbRandomBlockWrite(MK_FP(r -> DS, r -> DX), r -> CX, &nErrorCode))
				r -> AL = 0;
			else
				r -> AL = nErrorCode;
			break;
		}

	/* Parse File Name                                              */
	case 0x29:
		{
			BYTE FAR *lpFileName;

			lpFileName = MK_FP(r -> DS, r -> SI);
			r -> AL = FcbParseFname(r -> AL,
			 &lpFileName,
			 MK_FP(r -> ES, r -> DI));
			r -> DS = FP_SEG(lpFileName);
			r -> SI = FP_OFF(lpFileName);
		}
		break;

	/* Get Date                                                     */
	case 0x2a:
		DosGetDate(
		 (BYTE FAR *)&(r -> AL),        /* WeekDay              */
		 (BYTE FAR *)&(r -> DH),        /* Month                */
		 (BYTE FAR *)&(r -> DL),        /* MonthDay             */
		 (COUNT FAR *)&(r -> CX));      /* Year                 */
		break;

	/* Set Date                                                     */
	case 0x2b:
		rc = DosSetDate(
		 (BYTE FAR *)&(r -> DH),        /* Month                */
		 (BYTE FAR *)&(r -> DL),        /* MonthDay             */
		 (COUNT FAR *)&(r -> CX));      /* Year                 */
		if(rc != SUCCESS)
			r -> AL = 0xff;
		else
			r -> AL = 0;
		break;

	/* Get Time                                                     */
	case 0x2c:
		DosGetTime(
		 (BYTE FAR *)&(r -> CH),        /* Hour                 */
		 (BYTE FAR *)&(r -> CL),        /* Minutes              */
		 (BYTE FAR *)&(r -> DH),        /* Seconds              */
		 (BYTE FAR *)&(r -> DL));       /* Hundredths           */
		break;

	/* Set Date                                                     */
	case 0x2d:
		rc = DosSetTime(
		 (BYTE FAR *)&(r -> CH),        /* Hour                 */
		 (BYTE FAR *)&(r -> CL),        /* Minutes              */
		 (BYTE FAR *)&(r -> DH),        /* Seconds              */
		 (BYTE FAR *)&(r -> DL));       /* Hundredths           */
		if(rc != SUCCESS)
			r -> AL = 0xff;
		else
			r -> AL = 0;
		break;

	/* Set verify flag                                              */
	case 0x2e:
		verify_ena = (r -> AL ? TRUE : FALSE);
		break;

	/* Get DTA                                                      */
	case 0x2f:
		r -> ES = FP_SEG(dta);
		r -> BX = FP_OFF(dta);
		break;

	/* Get DOS Version                                              */
	case 0x30:
		r -> AL = os_major;
		r -> AH = os_minor;
		
		switch(r -> AL)
		{
		default:
		case 0:
			r -> BH= OEM_ID;
			break;

		case 1:
			r -> BH = 0;    /* RAM only for now             */
			break;
		}
		r -> BL = 0xff;         /* for now                      */
		r -> CX = 0xffff;
		break;

	/* Keep Program                                                 */
	case 0x31:
		DosMemChange(cu_psp, r -> DX < 6 ? 6 : r -> DX, 0);
		flush();
		tsr = TRUE;
		return_user();
		break;

	/* Get DPB                                                      */
	case 0x32:
		r -> AH = 0xff;
		break;

	/* DosVars - get/set dos variables                              */
	case 0x33:
		switch((r -> AL) & 0xff)
		{
		/* Get Ctrl-C flag                                      */
		case 0x00:
			r -> DL = (break_ena ? TRUE : FALSE);
			break;

		/* Set Ctrl-C flag                                      */
		case 0x01:
			break_ena = (r -> DL ? TRUE : FALSE);
			break;

		/* Get Boot Drive                                       */
		case 0x05:
			r -> DL = BootDrive;
			break;

		/* Get DOS-C version                                    */
		case 0x06:
			r -> BL = os_major;
			r -> BH = os_minor;
			r -> DL = rev_number;
			r -> DH = version_flags;
			break;
		default:
			r -> AX = -DE_INVLDFUNC;
			r -> FLAGS |= FLG_CARRY;
			break;

#ifdef DEBUG
		/* Toggle DOS-C rdwrblock trace dump                    */
		case 0xfd:
			bDumpRdWrParms = !bDumpRdWrParms;
			break;

		/* Toggle DOS-C syscall trace dump                      */
		case 0xfe:
			bDumpRegs = !bDumpRegs;
			break;
#endif

		/* Get DOS-C release string pointer                     */
		case 0xff:
			r -> DX = FP_SEG(os_release);
			r -> AX = FP_OFF(os_release);
			break;
		}
		break;

	/* Get InDOS flag                                               */
	case 0x34:
		{
			BYTE FAR *p;

			p = (BYTE FAR *)((BYTE *)&InDOS);
			r -> ES = FP_SEG(p);
			r -> BX = FP_OFF(p);
		}
		break;
		
	/* Get Interrupt Vector                                         */
	case 0x35:
		{
			BYTE FAR *p;

			p = getvec((COUNT)r -> AL);
			r -> ES = FP_SEG(p);
			r -> BX = FP_OFF(p);
		}
		break;

	/* Dos Get Disk Free Space                                      */
	case 0x36:
		DosGetFree(
			(COUNT)r -> DL,
			(COUNT FAR *)&r -> AX,
			(COUNT FAR *)&r -> BX,
			(COUNT FAR *)&r -> CX,
			(COUNT FAR *)&r -> DX);
		break;

	/* Undocumented Get/Set Switchar                                */
	case 0x37:
		switch(r -> AL)
		{
		case 0:
			r -> DL = switchar;
			break;

		case 1:
			switchar = r -> DL;
			break;

		case 2:
		case 3:
			r -> DL = 0xff;
			break;
		
		default:
			r -> AX = -DE_INVLDFUNC;
			r -> FLAGS |= FLG_CARRY;
			break;
		}
		break;

	/* Get/Set Country Info                                         */
	case 0x38:
		{
			BYTE FAR *lpTable
				= (BYTE FAR *)MK_FP(r -> DS, r -> DX);
			BYTE nRetCode;

			if(0xffff == r -> DX)
			{
				r -> BX = SetCtryInfo(
					(UBYTE FAR *)&(r -> AL),
					(UWORD FAR *)&(r -> BX),
					(BYTE FAR *)&lpTable,
					(UBYTE *)&nRetCode);
				if(nRetCode != 0)
				{
					r -> AX = 0xff;
					r -> FLAGS |= FLG_CARRY;
				}
				else
				{
					r -> AX = nRetCode;
					r -> FLAGS &= ~FLG_CARRY;
				}
			}
			else
			{
				r -> BX = GetCtryInfo(&(r -> AL), &(r -> BX), lpTable);
				r -> DS = FP_SEG(lpTable);
				r -> DX = FP_OFF(lpTable);
				r -> FLAGS &= ~FLG_CARRY;
			}
		}
		break;

	/* Dos Create Directory                                         */
	case 0x39:
		rc = dos_mkdir((BYTE FAR *)MK_FP(r -> DS, r -> DX));
		if(rc != SUCCESS)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;

	/* Dos Remove Directory                                         */
	case 0x3a:
		rc = dos_rmdir((BYTE FAR *)MK_FP(r -> DS, r -> DX));
		if(rc != SUCCESS)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;

	/* Dos Change Directory                                         */
	case 0x3b:
		xp = MK_FP( r -> DS, r -> DX);

		if((rc = DosChangeDir((BYTE FAR *)MK_FP(r -> DS, r -> DX))) < 0)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;

	/* Dos Create File                                              */
	case 0x3c:
		if((rc = DosCreat(MK_FP(r -> DS, r -> DX), r -> CX)) < 0)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> AX = rc;
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;

	/* Dos Open                                                     */
	case 0x3d:
		if((rc = DosOpen(MK_FP(r -> DS, r -> DX), r -> AL)) < 0)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> AX = rc;
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;

	/* Dos Close                                                    */
	case 0x3e:
		if((rc = DosClose(r -> BX)) < 0)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
			r -> FLAGS &= ~FLG_CARRY;
		break;

	/* Dos Read                                                     */
	case 0x3f:
		rc = DosRead(r -> BX, r -> CX, MK_FP(r -> DS, r -> DX), (COUNT FAR *)&rc1);

		if(rc1 != SUCCESS)
		{
			r -> FLAGS |= FLG_CARRY;
			r -> AX = -rc1;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
			r -> AX = rc;
		}
		break;

	/* Dos Write                                                    */
	case 0x40:
		rc = DosWrite(r -> BX, r -> CX, MK_FP(r -> DS, r -> DX), (COUNT FAR *)&rc1);
		if(rc1 != SUCCESS)
		{
			r -> FLAGS |= FLG_CARRY;
			r -> AX = -rc1;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
			r -> AX = rc;
		}
		break;

	/* Dos Delete File                                              */
	case 0x41:
		rc = dos_delete((BYTE FAR *)MK_FP(r -> DS, r -> DX));
		if(rc < 0)
		{
			r -> FLAGS |= FLG_CARRY;
			r -> AX = -rc1;
		}
		else
			r -> FLAGS &= ~FLG_CARRY;
		break;

	/* Dos Seek                                                     */
	case 0x42:
		if((rc = DosSeek(r -> BX, (LONG)((((LONG)(r -> CX)) << 16) + r -> DX), r -> AL, &lrc)) < 0)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> DX = (lrc >> 16);
			r -> AX = lrc & 0xffff;
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;

	/* Get/Set File Attributes                                      */
	case 0x43:
		switch(r -> AL)
		{
		case 0x00:
			rc = DosGetFattr((BYTE FAR *)MK_FP(r -> DS, r -> DX), (UWORD FAR *)&r -> CX);
			if(rc < SUCCESS)
			{
				r -> FLAGS |= FLG_CARRY;
				r -> AX = -rc;
			}
			else
			{
				r -> FLAGS &= ~FLG_CARRY;
			}
			break;

		case 0x01:
			rc = DosSetFattr((BYTE FAR *)MK_FP(r -> DS, r -> DX), (UWORD FAR *)&r -> CX);
			if(rc != SUCCESS)
			{
				r -> FLAGS |= FLG_CARRY;
				r -> AX = -rc;
			}
			else
				r -> FLAGS &= ~FLG_CARRY;
			break;
		
		default:
			r -> AX = -DE_INVLDFUNC;
			r -> FLAGS |= FLG_CARRY;
			break;
		}
		break;

	/* Device I/O Control                                           */
	case 0x44:
		{
			BOOL bNoChangeAx;
			
			bNoChangeAx = ((r -> AL == 0) || (r -> AL == 1));
			rc = DosDevIOctl(r, (COUNT FAR *)&rc1);
			if(rc1 != SUCCESS)
			{
				r -> FLAGS |= FLG_CARRY;
				r -> AX = -rc1;
			}
			else
			{
				r -> FLAGS &= ~FLG_CARRY;
				if(!bNoChangeAx)
					r -> AX = rc;
			}
		}
		break;

	/* Duplicate File Handle                                        */
	case 0x45:
		rc = DosDup(r -> BX);
		if(rc < SUCCESS)
		{
			r -> FLAGS |= FLG_CARRY;
			r -> AX = -rc;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
			r -> AX = rc;
		}
		break;

	/* Force Duplicate File Handle                                  */
	case 0x46:
		rc = DosForceDup(r -> BX, r -> CX);
		if(rc < SUCCESS)
		{
			r -> FLAGS |= FLG_CARRY;
			r -> AX = -rc;
		}
		else
			r -> FLAGS &= ~FLG_CARRY;
		break;

	/* Get Current Directory                                        */
	case 0x47:
		if((rc = DosGetCuDir(r -> DL, MK_FP(r -> DS, r -> SI))) < 0)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;
			
	/* Memory management                                            */
	case 0x48:
		if((rc = DosMemAlloc(r -> BX, mem_access_mode, &(r -> AX), &(r -> BX))) < 0)
		{
			DosMemLargest(&(r -> BX));
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			++(r -> AX);
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;
			

	case 0x49:
		if((rc = DosMemFree(--(r -> ES))) < 0)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
			r -> FLAGS &= ~FLG_CARRY;
		break;

	case 0x4a:
	{
		UWORD maxSize;

		if((rc = DosMemChange(r -> ES, r -> BX, &maxSize)) < 0)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
			
			if (rc == DE_NOMEM)
			    r -> BX = maxSize;
#if 0
			if(cu_psp == r -> ES)
			{
				
				psp FAR *p;

				p = MK_FP(cu_psp, 0);
				p -> ps_size = r -> BX + cu_psp;
			}
#endif
		}
		else
			r -> FLAGS &= ~FLG_CARRY;
		break;
	}

	/* Load and Execute Program                                     */
	case 0x4b:
		break_flg = FALSE;
		if((rc = DosExec(r -> AL, MK_FP(r -> ES, r -> BX), MK_FP(r -> DS, r -> DX))) 
			!= SUCCESS)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
			r -> FLAGS &= ~FLG_CARRY;
		break;

	/* End Program                                                  */
	case 0x4c:
		tsr = FALSE;
		return_mode = break_flg ? 1 : 0;
		return_code = r -> AL;
		DosMemCheck();
		return_user();
		break;

	/* Get Child-program Return Value                               */
	case 0x4d:
		r -> AL = return_code;
		r -> AH = return_mode;
		break;

	/* Dos Find First                                               */
	case 0x4e:
		{
			psp FAR *p = MK_FP(cu_psp,0);
			BYTE FAR *xp = MK_FP(r -> DS, r -> DX);
			/* dta for this call is set on entry.  This     */
			/* needs to be changed for new versions.        */

			if((rc = DosFindFirst((UCOUNT)r -> CX, (BYTE FAR *)MK_FP(r -> DS, r -> DX))) < 0)
			{
				r -> AX = -rc;
				r -> FLAGS |= FLG_CARRY;
			}
			else
			{
				r -> AX = 0;
				r -> FLAGS &= ~FLG_CARRY;
			}
		}
		break;

	/* Dos Find Next                                                */
	case 0x4f:
		{
			psp FAR *p = MK_FP(cu_psp,0);

			/* dta for this call is set on entry.  This     */
			/* needs to be changed for new versions.        */

			if((rc = DosFindNext()) < 0)
			{
				r -> AX = -rc;
				
				if (r -> AX == 2)
				    r -> AX = 18;

				r -> FLAGS |= FLG_CARRY;
			}
			else
			{
				r -> FLAGS &= ~FLG_CARRY;
			}
		}
		break;

	/* Set PSP                                                      */
	case 0x50:
		cu_psp = r -> BX;
		break;

	/* Get PSP                                                      */
	case 0x51:
		r -> BX = cu_psp;
		break;

	/* ************UNDOCUMENTED**************************************/
	/* Get List of Lists                                            */
	case 0x52:
		{
			BYTE FAR *p;

			p = (BYTE FAR *)&DPBp;
			r -> ES = FP_SEG(p);
			r -> BX = FP_OFF(p);
		}
		break;

	/* Get verify state                                             */
	case 0x54:
		r -> AL = (verify_ena ? TRUE : FALSE);
		break;

	/* ************UNDOCUMENTED**************************************/
	/* Dos Create New Psp & set p_size                              */
	case 0x55:
		new_psp((psp FAR *)MK_FP(r -> DX, 0), r -> SI);
		break;

	/* Dos Rename                                                   */
	case 0x56:
		rc = dos_rename(
		 (BYTE FAR *)MK_FP(r -> DS, r -> DX),   /* OldName      */
		 (BYTE FAR *)MK_FP(r -> ES, r -> DI));  /* NewName      */
		if(rc != SUCCESS)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;

	/* Get/Set File Date and Time                                   */
	case 0x57:
		switch(r -> AL)
		{
		case 0x00:
			if(!DosGetFtime(
			 (COUNT)r -> BX,        /* Handle               */
			 (date FAR *)&r -> DX,  /* FileDate             */
			 (time FAR *)&r -> CX)) /* FileTime             */
			{
				r -> AX = -DE_INVLDHNDL;
				r -> FLAGS |= FLG_CARRY;
			}
			else
				r -> FLAGS &= ~FLG_CARRY;
			break;

		case 0x01:
			if(!DosSetFtime(
			 (COUNT)r -> BX,        /* Handle               */
			 (date FAR *)&r -> DX,  /* FileDate             */
			 (time FAR *)&r -> CX)) /* FileTime             */
			{
				r -> AX = -DE_INVLDHNDL;
				r -> FLAGS |= FLG_CARRY;
			}
			else
				r -> FLAGS &= ~FLG_CARRY;
			break;
		
		default:
			r -> AX = -DE_INVLDFUNC;
			r -> FLAGS |= FLG_CARRY;
			break;
		}
		break;


	/* Get/Set Allocation Strategy                                  */
	case 0x58:
		switch(r -> AL)
		{
		case 0x00:
			r -> AX = mem_access_mode;
			break;

		case 0x01:
			if(((COUNT)r -> BX) < 0 || r -> BX > 2)
			{
				r -> AX = -DE_INVLDFUNC;
				r -> FLAGS |= FLG_CARRY;
			}
			else
			{
				mem_access_mode = r -> BX;
				r -> FLAGS &= ~FLG_CARRY;
			}
			break;
		
		default:
			r -> AX = -DE_INVLDFUNC;
			r -> FLAGS |= FLG_CARRY;
			break;
#ifdef DEBUG
		case 0xff:
			show_chain();
			break;
#endif
		}
		break;

	case 0x5a:
		if((rc = DosMkTmp(MK_FP(r -> DS, r -> DX), r -> CX)) < 0)
		{
			 r -> AX = -rc;
			 r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			 r -> AX = rc;
			 r -> FLAGS &= ~FLG_CARRY;
		}
		break;

	case 0x5b:
		
		if((rc = DosOpen(MK_FP(r -> DS, r -> DX), 0)) >= 0)
		{
			DosClose(rc);
			r -> AX = 80;
			r -> FLAGS |= FLG_CARRY;
		} 
		else 
		{
		       if((rc = DosCreat(MK_FP(r -> DS, r -> DX), r -> CX)) < 0)
		       {
				r -> AX = -rc;
				r -> FLAGS |= FLG_CARRY;
		       }
		       else
		       {
				r -> AX = rc;
				r -> FLAGS &= ~FLG_CARRY;
		       }
		}
		break;
	
	/* UNDOCUMENTED: share.exe and sda function                     */
	/* not really supported, but will pass.                         */
	case 0x5d:
		r -> AX = -DE_INVLDFUNC;
		r -> FLAGS |= FLG_CARRY;
		break;

	case 0x60:      /* TRUENAME */
		if ((rc = truename(MK_FP(r -> DS, r-> SI), MK_FP( r -> ES, r -> DI))) != SUCCESS)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;
		
	/* UNDOCUMENTED: no-op                                          */
	case 0x61:
		break;

	/* UNDOCUMENTED: return current psp                             */
	case 0x62:
		r -> BX = cu_psp;
		break;

	/* UNDOCUMENTED: Double byte and korean tables                  */
	/* not really supported, but will pass.                         */
	case 0x63:
	{
#ifdef DBLBYTE
		void FAR *dp = dbcsTable;

		r -> DS = FP_SEG(dp);
		r -> SI = FP_OFF(dp);
		r -> AL = 0;
#else
		r -> AL = 0xff;
#endif
		break;
	}
	case 0x64:
		r -> FLAGS &= ~FLG_CARRY;
		break;

	/* Extended country info                                        */
	case 0x65:
		if(r -> AL <= 0x7)
		{
			if(ExtCtryInfo(
				r -> AL,
				r -> BX,
				r -> CX,
				MK_FP(r -> ES, r -> DI)))
				r -> FLAGS &= ~FLG_CARRY;
			else
			{
				r -> AX = -DE_INVLDFUNC;
				r -> FLAGS |= FLG_CARRY;
			}
		}
		else if((r -> AL >= 0x20) && (r -> AL <= 0x22))
		{
			switch(r -> AL)
			{
			case 0x20:
				r -> DL = upChar(r -> DL);
				goto okay;

			case 0x21:
				upMem(
					MK_FP(r -> DS, r -> DX),
					r -> CX);
				goto okay;

			case 0x22:
				upString(MK_FP(r -> DS, r -> DX));
			okay:
				r -> FLAGS &= ~FLG_CARRY;
				break;

			case 0x23:
				r -> AX = yesNo(r -> DL);
				goto okay;

			default:
				r -> AX = -DE_INVLDFUNC;
				r -> FLAGS |= FLG_CARRY;
				break;
			}
		}
		else
			r -> FLAGS |= FLG_CARRY;
		break;

	case 0x66:
		switch(r -> AL)
		{
		case 1:
			GetGlblCodePage(
				(UWORD FAR *)&(r -> BX),
				(UWORD FAR *)&(r -> DX));
			goto okay_66;

		case 2:
			SetGlblCodePage(
				(UWORD FAR *)&(r -> BX),
				(UWORD FAR *)&(r -> DX));
		okay_66:
			r -> FLAGS &= ~FLG_CARRY;
			break;

		default:
			r -> AX = -DE_INVLDFUNC;
			r -> FLAGS |= FLG_CARRY;
			break;
		}
		break;
	case 0x67: 
		if ((rc = SetJFTSize( r -> BX)) != SUCCESS)
		{
			r -> AX = -rc;
			r -> FLAGS |= FLG_CARRY;
		}
		else
		{
			r -> FLAGS &= ~FLG_CARRY;
		}
		break;
	}

#ifdef DEBUG
	if(bDumpRegs)
	{
		printf("Exiting system call (21h)\n");
		fbcopy((VOID FAR *)r, (VOID FAR *)&error_regs, sizeof(iregs));
		dump_regs = TRUE;
		dump();
		printf("---\n");
	}
#endif
}



VOID INRPT FAR 
int22_handler (void)
{
}

#pragma argsused
VOID INRPT FAR 
int23_handler (int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags)
{
	tsr = FALSE;
	return_mode = 1;
	return_code = -1;
	mod_sto(CTL_C);
	DosMemCheck();
	return_user();
}


#pragma argsused
VOID INRPT FAR 
int24_handler (void)
{
}


#pragma argsused
VOID INRPT FAR 
int25_handler (void)
{
   /* ...just turn this into a regular device driver request */
}


#pragma argsused
VOID INRPT FAR 
int26_handler (void)
{
}


#pragma argsused
VOID INRPT FAR 
int27_handler (int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags)
{
	tsr = 1;
	InDOS++;
	DosMemChange(cs , dx, 0);
	flush();
	return_user();
}


VOID INRPT FAR 
int28_handler (void)
{
}

VOID INRPT FAR
empty_handler (void)
{
}

