
/****************************************************************/
/*								*/
/*			    chario.c				*/
/*			     DOS-C				*/
/*								*/
/*    Character device functions and device driver interface	*/
/*								*/
/*			Copyright (c) 1994			*/
/*			Pasquale J. Villani			*/
/*			All Rights Reserved			*/
/*								*/
/* This file is part of DOS-C.					*/
/*								*/
/* DOS-C is free software; you can redistribute it and/or	*/
/* modify it under the terms of the GNU General Public License	*/
/* as published by the Free Software Foundation; either version	*/
/* 2, or (at your option) any later version.			*/
/*								*/
/* DOS-C is distributed in the hope that it will be useful, but	*/
/* WITHOUT ANY WARRANTY; without even the implied warranty of	*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See	*/
/* the GNU General Public License for more details.		*/
/*								*/
/* You should have received a copy of the GNU General Public	*/
/* License along with DOS-C; see the file COPYING.  If not,	*/
/* write to the Free Software Foundation, 675 Mass Ave,		*/
/* Cambridge, MA 02139, USA.					*/
/*								*/
/****************************************************************/

#include "../../hdr/portab.h"

/* $Logfile:   D:/dos-c/src/fs/chario.c_v  $ */
#ifndef IPL
static BYTE *charioRcsId = "$Header:   D:/dos-c/src/fs/chario.c_v   1.3   29 May 1996 21:15:12   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/fs/chario.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:15:12   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   01 Sep 1995 17:48:42   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:05:44   patv
 * Initial revision.
 * 
 */

#include "globals.h"

static BYTE *con_name = "CON";

#if !defined(KERNEL) && !defined(IPL)
VOID INRPT FAR 
handle_break (void)
{
}
#endif

#ifdef PROTO
BOOL _sto(COUNT);
VOID kbfill(keyboard FAR *, UCOUNT, BOOL);
#else
BOOL _sto();
VOID kbfill();
#endif

static BOOL 
_sto (COUNT c)
{
	request rq;
	BYTE buf = c;

	if(con_break())
	{
		handle_break();
		return FALSE;
	}
	rq.r_length = sizeof(request);
	rq.r_command = C_OUTPUT;
	rq.r_count = 1;
	rq.r_trans = (BYTE FAR *)(&buf);
	rq.r_status = 0;
	execrh((request FAR *)&rq, (struct dhdr FAR *)&con_dev);
	if(rq.r_status & S_ERROR)
		return char_error(&rq, con_name);
	return TRUE;
}


VOID 
sto (COUNT c)
{
	/* Test for hold char					*/
	con_hold();
	
	/* Display a printable character			*/
	if(c != HT)
		_sto(c);
	if(c == CR)
		scr_pos = 0;
	else if(c == BS)
	{
		if(scr_pos > 0)
			--scr_pos;
	}
	else if(c == HT)
	{
		do
			_sto(' ');
		while(++scr_pos & 7);
	}
	else if(c != LF && c != BS)
		++scr_pos;
}


VOID 
mod_sto (REG UCOUNT c)
{
	if(c < ' ' && c != HT)
	{
		sto('^');
		sto(c + '@');
	}
	else
		sto(c);
}


VOID 
destr_bs (void)
{
	sto(BS);
	sto(' ');
	sto(BS);
}


UCOUNT 
_sti (void)
{
	UBYTE cb;
	request rq;

	rq.r_length = sizeof(request);
	rq.r_command = C_INPUT;
	rq.r_count = 1;
	rq.r_trans = (BYTE FAR *)&cb;
	rq.r_status = 0;
	execrh((request FAR *)&rq, (struct dhdr FAR *)&con_dev);
	if(rq.r_status & S_ERROR)
		return char_error(&rq, con_name);
	if(cb == CTL_C)
	{
		handle_break();
		return CTL_C;
	}
	else
		return cb;
}

VOID 
con_hold (void)
{
	request rq;

	rq.r_unit = 0;
	rq.r_status = 0;
	rq.r_command = C_NDREAD;
	rq.r_length = sizeof(request);
	execrh((request FAR *)&rq, (struct dhdr FAR *)&con_dev);
	if(rq.r_status & S_BUSY)
		return;
	if(rq.r_ndbyte == CTL_S)
	{
		_sti();
		while(_sti() != CTL_Q)
			/* just wait */;
	}
}


BOOL 
con_break (void)
{
	request rq;

	rq.r_unit = 0;
	rq.r_status = 0;
	rq.r_command = C_NDREAD;
	rq.r_length = sizeof(request);
	execrh((request FAR *)&rq, (struct dhdr FAR *)&con_dev);
	if(rq.r_status & S_BUSY)
		return FALSE;
	if(rq.r_ndbyte == CTL_C)
	{
		_sti();
		return TRUE;
	}
	else
		return FALSE;
}


BOOL 
KbdBusy (void)
{
	request rq;

	rq.r_unit = 0;
	rq.r_status = 0;
	rq.r_command = C_ISTAT;
	rq.r_length = sizeof(request);
	execrh((request FAR *)&rq, (struct dhdr FAR *)&con_dev);
	if(rq.r_status & S_BUSY)
		return TRUE;
	else
		return FALSE;
}


VOID 
KbdFlush (void)
{
	request rq;

	rq.r_unit = 0;
	rq.r_status = 0;
	rq.r_command = C_IFLUSH;
	rq.r_length = sizeof(request);
	execrh((request FAR *)&rq, (struct dhdr FAR *)&con_dev);
}


static VOID kbfill(kp, c, ctlf)
keyboard FAR *kp;
UCOUNT c;
BOOL ctlf;
{
	if(kp -> kb_count > kp -> kb_size)
	{
		sto(BELL);
		return;
	}
	kp -> kb_buf[kp -> kb_count++] = c;
	if(!ctlf)
		mod_sto(c);
	else
		sto(c);
}


VOID sti(kp)
keyboard FAR *kp;
{
	REG UWORD c, cu_pos = scr_pos;
	WORD init_count = kp -> kb_count;
#ifndef NOSPCL
	static BYTE local_buffer[LINESIZE];
#endif

	if(kp -> kb_size == 0)
		return;
	if(kp -> kb_size <= kp -> kb_count || kp -> kb_buf[kp -> kb_count] != CR)
		kp -> kb_count = 0;
	FOREVER
	{
		switch(c = _sti())
		{
		case CTL_F:
			continue;

#ifndef NOSPCL
		case SPCL:
			switch(c = _sti())
			{
			case LEFT:
				goto backspace;

			case F3:
			{
				REG COUNT i;

				for(i = kp -> kb_count; local_buffer[i] != '\0'; i++)
				{
					c = local_buffer[kp -> kb_count];
					if(c == '\r' || c == '\n')
						break;
					kbfill(kp, c, FALSE);
				}
				break;
			}

			case RIGHT:
				c = local_buffer[kp -> kb_count];
				if(c == '\r' || c == '\n')
					break;
				kbfill(kp, c, FALSE);
				break;
			}
			break;
#endif

		case CTL_BS:
		case BS:
		backspace:
			if(kp -> kb_count > 0)
			{
				if(kp -> kb_buf[kp -> kb_count - 1] >= ' ')
				{
					destr_bs();
				}
				else if((kp -> kb_buf[kp -> kb_count - 1] < ' ')
				 && (kp -> kb_buf[kp -> kb_count - 1] != HT))
				{
					destr_bs();
					destr_bs();
				}
				else if(kp -> kb_buf[kp -> kb_count - 1] == HT)
				{
					do
					{
						destr_bs();
					}
					while((scr_pos > cu_pos) && (scr_pos & 7));
				}
				--kp -> kb_count;
			}
			break;

		case CR:
			kbfill(kp, CR, TRUE);
			kbfill(kp, LF, TRUE);
#ifndef NOSPCL
			fbcopy((BYTE FAR *)kp -> kb_buf,
			 (BYTE FAR *)local_buffer, (COUNT)kp -> kb_count);
			local_buffer[kp -> kb_count] = '\0';
#endif
			return;

		case LF:
			sto(CR);
			sto(LF);
			break;

		case ESC:
			sto('\\');
			sto(CR);
			sto(LF);
			for(c = 0; c < cu_pos; c++)
				sto(' ');
			kp -> kb_count = init_count;
			break;

		default:
			kbfill(kp, c, FALSE);
			break;
		}
	}
}


