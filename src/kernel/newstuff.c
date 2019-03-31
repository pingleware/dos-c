
/****************************************************************/
/*                                                              */
/*                           newstuff.c                         */
/*                            DOS-C                             */
/*                                                              */
/*                       Copyright (c) 1996			*/
/*                          Svante Frey				*/
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

/* $Logfile$ */
#ifdef VERSION_STRINGS
static BYTE *mainRcsId = "$Header$";
#endif

/*
 * $Log$
 */


#include        "../../hdr/portab.h"
#include        "globals.h"
#include        "proto.h"

BYTE dbcsTable[] =          /* empty DBCS table */
{
        0, 0
};

int SetJFTSize(UWORD nHandles)
{
    UWORD block, maxBlock;
    psp FAR *ppsp = MK_FP(cu_psp, 0);
    UBYTE FAR *newtab;
    COUNT i;
    
    if (nHandles <= ppsp -> ps_maxfiles)
    {
        ppsp -> ps_maxfiles = nHandles;
        return SUCCESS;
    }

    if (nHandles > SFTMAX)
        return DE_TOOMANY;
    
    if((DosMemAlloc((nHandles + 0xf) >> 4, mem_access_mode, &block, &maxBlock)) < 0)
        return DE_NOMEM;
    
    ++block;
    newtab = MK_FP(block, 0);
    
    for (i = 0; i < ppsp -> ps_maxfiles; i++)
    {
         newtab[i] = ppsp -> ps_filetab[i];
    }
    
    for ( ; i < nHandles; i++)
    {
         newtab[i] = 0xff;
    }
    
    ppsp -> ps_maxfiles = nHandles;
    ppsp -> ps_filetab = newtab;
    
    return SUCCESS;
}

int DosMkTmp(BYTE FAR *pathname, UWORD attr)
{
    /* create filename from current date and time */
    char tempname[128];
    char tokens[16] = "0123456789ABCDEF";
    char *ptmp = tempname;
    BYTE wd, month, day;
    BYTE h, m, s, hund;
    WORD sh;
    WORD year;
    int rc;

    while (*pathname)
    {
          *ptmp++ = *pathname++;
    } 
    
    if (ptmp[-1] != '\\' && ptmp[-1] != '/')
        *ptmp++ = '\\';

    DosGetDate(&wd, &month, &day, &year);
    DosGetTime(&h, &m, &s, &hund);

    sh = s * 100 + hund;
    
    ptmp[0] = tokens[ year & 0xf ];
    ptmp[1] = tokens[ month ];
    ptmp[2] = tokens[ day & 0xf ];
    ptmp[3] = tokens[ h & 0xf ];
    ptmp[4] = tokens[ m & 0xf ];
    ptmp[5] = tokens[ (sh >> 8) & 0xf ];
    ptmp[6] = tokens[ (sh >> 4) & 0xf ];
    ptmp[7] = tokens[ sh & 0xf ];
    ptmp[8] = '.';                      
    ptmp[9] = 'A';
    ptmp[10] = 'A';
    ptmp[11] = 'A';
    ptmp[12] = '0';

    while ((rc = DosOpen(tempname, 0)) >= 0)
    {
        DosClose(rc);
       
        if (++ptmp[11] > 'Z')
        {
                if (++ptmp[10] > 'Z')
                {    
                        if (++ptmp[9] > 'Z')
                            return DE_TOOMANY;
                        
                        ptmp[10] = 'A';
                }
                ptmp[11] = 'A';
       }
    }

    if (rc == DE_FILENOTFND) 
    {
        rc = DosCreat(tempname, attr);
    }
    return rc;
}   

int truename(char FAR *src, char FAR *dest)
{
    char buf[128] = "A:\\";
    char *bufp = buf + 3;
    BYTE far *test;

    /* Do we have a drive? */
    if (src[1] == ':')
    {
        buf[0] = (src[0] | 0x20) + 'A' - 'a';
        
        if (buf[0] >= nblkdev + 'A')
            return DE_PATHNOTFND;
        
        src += 2; 
    }
    else {
         buf[0] = default_drive + 'A';
    }

    if (*src != '\\' && *src != '/')   /* append current dir */
    {
         DosGetCuDir(buf[0] - '@', bufp);
         
         /* the first backslash already added */
         if (*bufp)
         {
             while (*bufp) bufp++;
             /* add the last one */
             *bufp++ = '\\';
         }
    } 

    /* convert all forward slashes to backslashes, and uppercase all characters */
    while(*src)
    {
       char c;

       switch((c = *src++))
       {
          case '/':         /* convert to backslash */
            *bufp++ = '\\';   
            break;
          
          /* look for '.' and '..' dir entries */
          case '.':         
            if (bufp[-1] == '\\')
            {
              if (*src == '.' && (src[1] == '/' || src[1] == '\\' || !src[1]))
              {    
                /* '..' dir entry: rewind bufp to last backslash */
                
                for (bufp -= 2 ; *bufp != '\\'; bufp--)
                {
                    if (bufp < buf + 2)    /* '..' illegal in root dir */
                        return DE_PATHNOTFND;     
                }
                bufp++;
              } else if (*src == '/' || *src == '\\' || *src == 0)     
              {
                 if (*src != 0) src++;     /* '.' directory: just skip it */          
              }
            }
            else *bufp++ = c;
            break;

          default:
            *bufp++ = c;
            break;
       }
    }
    *bufp++ = 0;

    /* finally, uppercase everything */
    upString(buf);

    /* copy to user's buffer */
    _fmemcpy(dest, buf, bufp - buf);

    return SUCCESS;
}
