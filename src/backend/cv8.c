// Compiler implementation of the D programming language
// Copyright (c) 2012-2012 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// http://www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

// This module generates the .debug$S and .debug$T sections for Win64,
// which are the MS-Coff symbolic debug info and type debug info sections.

#if !SPP

#include        <stdio.h>
#include        <string.h>
#include        <stdlib.h>
#include        <time.h>

#include        "cc.h"
#include        "el.h"
#include        "code.h"
#include        "oper.h"
#include        "global.h"
#include        "type.h"
#include        "dt.h"
#include        "exh.h"
#include        "cgcv.h"
#include        "obj.h"
#include        "outbuf.h"

static char __file__[] = __FILE__;      /* for tassert.h                */
#include        "tassert.h"

#if _MSC_VER || __sun
#include        <alloca.h>
#endif

#if MARS
#if TARGET_WINDOS

// The "F1" section, which is the symbols
static Outbuffer *F1_buf;

// The "F2" section, which is the line numbers
static Outbuffer *F2_buf;

// The "F3" section, which is global and a string table of source file names.
static Outbuffer *F3_buf;

// The "F4" section, which is global and a lists info about source files.
static Outbuffer *F4_buf;

static const char *srcfilename;
static unsigned srcfileoff;
static Symbol *sfunc;

/* Fixups that go into F1 section
 */
struct F1_Fixups
{
    Symbol *s;
    unsigned offset;
};

static Outbuffer *F1fixup;      // array of F1_Fixups

/* Struct in which to collect per-function data, for later emission
 * into .debug$S.
 */
struct FuncData
{
    Symbol *sfunc;
    unsigned section_length;
    const char *srcfilename;
    unsigned srcfileoff;
    unsigned linepairstart;     // starting index of offset/line pairs in linebuf[]
    unsigned linepairnum;       // number of offset/line pairs
};

static Outbuffer *funcdata;     // array of FuncData's

static Outbuffer *linepair;     // array of offset/line pairs
static unsigned linepairstart;
static unsigned linepairnum;

unsigned cv8_addfile(const char *filename);
void cv8_writesection(int seg, unsigned type, Outbuffer *buf);
void cv8_flushsection(int seg, unsigned type, Outbuffer *buf);
static void cv8_writeCompiland(const char *filename);

/************************************************
 * Called at the start of an object file generation.
 * One source file can generate multiple object files; this starts an object file.
 * Input:
 *      filename        source file name
 */
void cv8_initfile(const char *filename, const char *objfile)
{
    //printf("cv8_initfile()\n");

    // Recycle buffers; much faster than delete/renew

    if (!F1_buf)
        F1_buf = new Outbuffer(1024);
    F1_buf->setsize(0);

    if (!F1fixup)
        F1fixup = new Outbuffer(1024);
    F1fixup->setsize(0);

    if (!F2_buf)
        F2_buf = new Outbuffer(1024);
    F2_buf->setsize(0);

    if (!F3_buf)
        F3_buf = new Outbuffer(1024);
    F3_buf->setsize(0);
    F3_buf->writeByte(0);       // first "filename"

    if (!F4_buf)
        F4_buf = new Outbuffer(1024);
    F4_buf->setsize(0);

    if (!funcdata)
        funcdata = new Outbuffer(1024);
    funcdata->setsize(0);

    if (!linepair)
        linepair = new Outbuffer(1024);
    linepair->setsize(0);
    linepairstart = 0;
    linepairnum = 0;

    cv_init();

    cv8_writeCompiland(objfile);
}

void cv8_termfile()
{
    //printf("cv8_termfile()\n");

    /* Write out the debug info sections.
     */

    int seg = MsCoffObj::seg_debugS();

    // Write out "F1" section
    cv8_flushsection(seg, 0xF1, F1_buf);

    // Write out "F2" sections
    unsigned length = funcdata->size();
    unsigned char *p = funcdata->buf;
    for (unsigned u = 0; u < length; u += sizeof(FuncData))
    {   FuncData *fd = (FuncData *)(p + u);

        int f2seg = seg;
        if (symbol_iscomdat(fd->sfunc))
        {
            f2seg = MsCoffObj::seg_debugS_comdat(fd->sfunc);
        }
        cv8_flushsection(f2seg, 0xF2, F2_buf); // ensures 4 hdr written to f2seg and F2_buf empty
#if 1
        cv8_flushsection(f2seg, 0xF1, F1_buf);

        int namelen = strlen(fd->sfunc->Sident);
        unsigned short id = 0x1110; // S_GPROC32_V3
        unsigned short len = 2 + 8 * 4 + 2 + 1 + namelen + 1;
        F1_buf->write(&len, 2);
        F1_buf->write(&id, 2);
        F1_buf->write32(0); // pparent;
        F1_buf->write32(0); // pend;
        F1_buf->write32(0); // next;
        F1_buf->write32(fd->section_length); // proc_len;
        F1_buf->write32(0); // debug_start;
        F1_buf->write32(fd->section_length-1); // debug_end;
        F1_buf->write32(0); // proctype;
        F1_buf->write32(0); // offset;
        unsigned reloffset = SegData[f2seg]->SDoffset + 8 + F1_buf->size();
        F1_buf->writezeros(2); // segment;
        F1_buf->writeByte(0); // flags;
        F1_buf->write(fd->sfunc->Sident, namelen + 1);

        id = 6; // S_END
        len = 2;
        F1_buf->write(&len, 2);
        F1_buf->write(&id, 2);

        cv8_flushsection(f2seg, 0xF1, F1_buf);

        objmod->reftoident(f2seg, reloffset, fd->sfunc, 0, CFseg | CFoff);
#endif

        F2_buf->write32(fd->sfunc->Soffset);
        F2_buf->write32(0);
        F2_buf->write32(fd->section_length);
        F2_buf->write32(fd->srcfileoff);
        F2_buf->write32(fd->linepairnum);
        F2_buf->write32(fd->linepairnum * 8 + 12);
        F2_buf->write(linepair->buf + fd->linepairstart * 8, fd->linepairnum * 8);

        unsigned offset = SegData[f2seg]->SDoffset + 8;
        cv8_flushsection(f2seg, 0xF2, F2_buf);
        objmod->reftoident(f2seg, offset, fd->sfunc, 0, CFseg | CFoff);
    }

    // Write out "F3" section
    cv8_writesection(seg, 0xF3, F3_buf);

    // Write out "F4" section
    cv8_writesection(seg, 0xF4, F4_buf);

    // Write out "F1" section
    unsigned f1offset = SegData[seg]->SDoffset;
    cv8_writesection(seg, 0xF1, F1_buf);

    // Fixups for "F1" section
    length = F1fixup->size();
    p = F1fixup->buf;
    for (unsigned u = 0; u < length; u += sizeof(F1_Fixups))
    {   F1_Fixups *f = (F1_Fixups *)(p + u);

        objmod->reftoident(seg, f1offset + 8 + f->offset, f->s, 0, CFseg | CFoff);
    }

    // Write out .debug$T section
    cv_term();
}

/************************************************
 * Called at the start of a module.
 * Note that there can be multiple modules in one object file.
 * cv8_initfile() must be called first.
 */
void cv8_initmodule(const char *filename, const char *modulename)
{
    //printf("cv8_initmodule(filename = %s, modulename = %s)\n", filename, modulename);

    /* Experiments show that filename doesn't have to be qualified if
     * it is relative to the directory the .exe file is in.
     */
    srcfileoff = cv8_addfile(filename);
}

void cv8_termmodule()
{
    //printf("cv8_termmodule()\n");
    assert(config.exe == EX_WIN64);
}

/******************************************
 * Called at the start of a function.
 */
void cv8_func_start(Symbol *sfunc)
{
    //printf("cv8_func_start(%s)\n", sfunc->Sident);
    linepairstart += linepairnum;
    linepairnum = 0;
    srcfilename = NULL;
}

void cv8_func_term(Symbol *sfunc)
{
    //printf("cv8_func_term(%s)\n", sfunc->Sident);

    FuncData fd;
    memset(&fd, 0, sizeof(fd));

    fd.sfunc = sfunc;
    fd.section_length = retoffset + retsize;
    fd.srcfilename = srcfilename;
    fd.srcfileoff = srcfileoff;
    fd.linepairstart = linepairstart;
    fd.linepairnum = linepairnum;

    funcdata->write(&fd, sizeof(fd));
}

/**********************************************
 */

void cv8_linnum(Srcpos srcpos, targ_size_t offset)
{
    //printf("cv8_linnum(file = %s, line = %d, offset = x%x)\n", srcpos.Sfilename, (int)srcpos.Slinnum, (unsigned)offset);
    if (srcfilename)
    {
        /* Ignore line numbers from different files in the same function.
         * This can happen with inlined functions.
         * To make this work would require a separate F2 section for each different file.
         */
        if (srcfilename != srcpos.Sfilename &&
            strcmp(srcfilename, srcpos.Sfilename))
            return;
    }
    else
    {
        srcfilename = srcpos.Sfilename;
        srcfileoff  = cv8_addfile(srcpos.Sfilename);
    }
    linepair->write32((unsigned)offset);
    linepair->write32((unsigned)srcpos.Slinnum | 0x80000000);
    ++linepairnum;
}

/**********************************************
 * Add source file, if it isn't already there.
 * Return offset into F4.
 */

unsigned cv8_addfile(const char *filename)
{
    //printf("cv8_addfile('%s')\n", filename);

    /* The algorithms here use a linear search. This is acceptable only
     * because we expect only 1 or 2 files to appear.
     * Unlike C, there won't be lots of .h source files to be accounted for.
     */

    unsigned length = F3_buf->size();
    unsigned char *p = F3_buf->buf;
    size_t len = strlen(filename);

    unsigned off = 1;
    while (off + len < length)
    {
        if (memcmp(p + off, filename, len + 1) == 0)
        {   // Already there
            //printf("\talready there at %x\n", off);
            goto L1;
        }
        off += strlen((const char *)(p + off)) + 1;
    }
    off = length;
    // Add it
    F3_buf->write(filename, len + 1);

L1:
    // off is the offset of the filename in F3.
    // Find it in F4.

    length = F4_buf->size();
    p = F4_buf->buf;

    unsigned u = 0;
    while (u + 8 <= length)
    {
        //printf("\t%x\n", *(unsigned *)(p + u));
        if (off == *(unsigned *)(p + u))
        {
            //printf("\tfound %x\n", u);
            return u;
        }
        u += 4;
        unsigned short type = *(unsigned short *)(p + u);
        u += 2;
        if (type == 0x0110)
            u += 16;            // MD5 checksum
        u += 2;
    }

    // Not there. Add it.
    F4_buf->write32(off);

    /* Write 10 01 [MD5 checksum]
     *   or
     * 00 00
     */
    F4_buf->writeShort(0);

    // 2 bytes of pad
    F4_buf->writeShort(0);

    //printf("\tadded %x\n", length);
    return length - 8; // subtract header size
}

void cv8_writesection(int seg, unsigned type, Outbuffer *buf)
{
    /* Write out as:
     *  bytes   desc
     *  -------+----
     *  4       type
     *  4       length
     *  length  data
     *  pad     pad to 4 byte boundary
     */
    unsigned off = SegData[seg]->SDoffset;
    objmod->bytes(seg,off,4,&type);
    unsigned length = buf->size();
    objmod->bytes(seg,off+4,4,&length);
    objmod->bytes(seg,off+8,length,buf->buf);
    // Align to 4
    unsigned pad = ((length + 3) & ~3) - length;
    objmod->lidata(seg,off+8+length,pad);
}

void cv8_flushsection(int seg, unsigned type, Outbuffer *buf)
{
    if(buf->size() == 0)
        return;

    unsigned off = SegData[seg]->SDoffset;
    if(off == 0)
    {
        unsigned v = 4;
        objmod->bytes(seg,0,4,&v);
    }

    cv8_writesection(seg, type, buf);
    buf->setsize(0);
}

static void cv8_writeCompiland(const char *filename)
{
    int flen = strlen(filename);
    unsigned short len = 2 + 4 + flen + 1;
    F1_buf->write(&len, 2);
    unsigned short id = 0x1101; // S_COMPILAND_V3
    F1_buf->write(&id, 2);
    F1_buf->write32(0);
    F1_buf->write(filename, flen + 1);
}

#define S_COMPILAND_V3  0x1101
#define S_THUNK_V3      0x1102
#define S_BLOCK_V3      0x1103
#define S_LABEL_V3      0x1105
#define S_REGISTER_V3   0x1106
#define S_CONSTANT_V3   0x1107
#define S_UDT_V3        0x1108
#define S_BPREL_V3      0x110B
#define S_LDATA_V3      0x110C
#define S_GDATA_V3      0x110D
#define S_PUB_V3        0x110E
#define S_LPROC_V3      0x110F
#define S_GPROC_V3      0x1110
#define S_BPREL_XXXX_V3 0x1111
#define S_MSTOOL_V3     0x1116
#define S_PUB_FUNC1_V3  0x1125
#define S_PUB_FUNC2_V3  0x1127
#define S_SECTINFO_V3   0x1136
#define S_SUBSECTINFO_V3 0x1137
#define S_ENTRYPOINT_V3 0x1138
#define S_SECUCOOKIE_V3 0x113A
#define S_MSTOOLINFO_V3 0x113C
#define S_MSTOOLENV_V3  0x113D

void cv8_outsym(Symbol *s)
{
    //printf("cv8_outsym(s = '%s')\n", s->Sident);
    //symbol_print(s);
    if (s->Sflags & SFLnodebug)
        return;
return;

    idx_t typidx = cv_typidx(s->Stype);
    const char *id = s->prettyIdent ? s->prettyIdent : prettyident(s);
    size_t len = strlen(id);

    F1_Fixups f1f;

    switch (s->Sclass)
    {
        case SCglobal:
            /*
             *  2       length (not including these 2 bytes)
             *  2       S_GDATA_V3
             *  4       typidx
             *  6       ref to symbol
             *  n       0 terminated name string
             */
            F1_buf->reserve(2 + 2 + 4 + 6 + len + 1);
            F1_buf->writeWordn(2 + 4 + 6 + len + 1);
            F1_buf->writeWordn(S_GDATA_V3);
            F1_buf->write32(typidx);

            f1f.s = s;
            f1f.offset = F1_buf->size();
            F1fixup->write(&f1f, sizeof(f1f));
            F1_buf->write32(0);
            F1_buf->writeWordn(0);

            F1_buf->writen(id, len + 1);
            break;
    }
}

#endif
#endif
#endif
