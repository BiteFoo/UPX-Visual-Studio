/* help.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2013 Laszlo Molnar
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#include "conf.h"
#include "compress.h"
#include "packmast.h"
#include "packer.h"


/*************************************************************************
//
**************************************************************************/

static bool head_done = 0;

void show_head(void)
{
    FILE *f = con_term;
    int fg;

    if (head_done)
        return;
    head_done = 1;

#define V(x)    (strcmp(UPX_VERSION_STRING, UPX_VERSION_STRING4) ? UPX_VERSION_STRING : UPX_VERSION_STRING x)
    fg = con_fg(f,FG_GREEN);
    con_fprintf(f,
                "                       Ultimate Packer for Win32 eXecutables\n"
                "            Compile with Microsoft Visual Studio 2013 by James34602\n\n");
    fg = con_fg(f,fg);
#undef V

    UNUSED(fg);
}


/*************************************************************************
//
**************************************************************************/

void show_usage(void)
{
    FILE *f = con_term;

    con_fprintf(f,"Usage: %s [-123456789dlthVL] [-qvfk] [-o file] %sfile..\n", progname,
#if defined(__DJGPP__) || defined(__EMX__)
                "[@]");
#else
                "");
#endif
}


/*************************************************************************
//
**************************************************************************/

struct PackerNames
{
    struct Entry {
        const char* fname;
        const char* sname;
    };
    Entry names[64];
    size_t names_count;
    const options_t *o;
    PackerNames() : names_count(0), o(NULL) { }
    void add(const Packer *p)
    {
        p->assertPacker();
        assert(names_count < 64);
        names[names_count].fname = p->getFullName(o);
        names[names_count].sname = p->getName();
        names_count++;
    }
    static Packer* visit(Packer *p, void *user)
    {
        PackerNames *self = (PackerNames *) user;
        self->add(p);
        delete p; p = NULL;
        return NULL;
    }
    static int __acc_cdecl_qsort cmp_fname(const void *a, const void *b) {
        return strcmp(((const Entry *) a)->fname, ((const Entry *) b)->fname);
    }
    static int __acc_cdecl_qsort cmp_sname(const void *a, const void *b) {
        return strcmp(((const Entry *) a)->sname, ((const Entry *) b)->sname);
    }
};

static void show_all_packers(FILE *f, int verbose)
{
    options_t o; o.reset();
    PackerNames pn; pn.o = &o;
    PackMaster::visitAllPackers(pn.visit, NULL, &o, &pn);
    qsort(pn.names, pn.names_count, sizeof(PackerNames::Entry), pn.cmp_fname);
    size_t pos = 0;
    for (size_t i = 0; i < pn.names_count; ++i)
    {
        const char *fn = pn.names[i].fname;
        const char *sn = pn.names[i].sname;
        if (verbose)
        {
            con_fprintf(f, "    %-32s %s\n", fn, sn);
        }
        else
        {
            size_t fl = strlen(fn);
            if (pos == 0) {
                con_fprintf(f, "  %s", fn);
                pos = 2 + fl;
            } else if (pos + 1 + fl > 80) {
                con_fprintf(f, "\n  %s", fn);
                pos = 2 + fl;
            } else {
                con_fprintf(f, " %s", fn);
                pos += 1 + fl;
            }
        }
    }
    if (!verbose && pn.names_count)
        con_fprintf(f, "\n");
}


/*************************************************************************
//
**************************************************************************/

void show_help(int verbose)
{
    FILE *f = con_term;
    int fg;

    show_head();
    show_usage();

    fg = con_fg(f,FG_YELLOW);
    con_fprintf(f,"\nCommands:\n");
    fg = con_fg(f,fg);
    con_fprintf(f,
                "  -1     compress faster                   -9    compress better\n"
                "%s"
                "  -d     decompress                        -l    list compressed file\n"
                "  -t     test compressed file              -V    display version number\n"
                "  -h     give %s help                    -L    display software license\n%s",
                verbose == 0 ? "" : "  --best compress best (can be slow for big files)\n",
                verbose == 0 ? "more" : "this", verbose == 0 ? "" : "\n");

    fg = con_fg(f,FG_YELLOW);
    con_fprintf(f,"Options:\n");
    fg = con_fg(f,fg);

    con_fprintf(f,
                "  -q     be quiet                          -v    be verbose\n"
                "  -oFILE write output to 'FILE'\n"
                //"  -f     force overwrite of output files and compression of suspicious files\n"
                "  -f     force compression of suspicious files\n"
                "%s%s"
                , (verbose == 0) ? "  -k     keep backup files\n" : ""
#if 1
                , (verbose > 0) ? "  --no-color, --mono, --color, --no-progress   change look\n" : ""
#else
                , ""
#endif
                );

    if (verbose > 0)
    {
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"\nCompression tuning options:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --brute             try all available compression methods & filters [slow]\n"
                    "  --ultra-brute       try even more compression variants [very slow]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Backup options:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  -k, --backup        keep backup files\n"
                    "  --no-backup         no backup files [default]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Overlay options:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --overlay=copy      copy any extra data attached to the file [default]\n"
                    "  --overlay=strip     strip any extra data attached to the file [DANGEROUS]\n"
                    "  --overlay=skip      don't compress a file with an overlay\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Options for djgpp2/coff:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --coff              produce COFF output [default: EXE]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Options for dos/com:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --8086              make compressed com work on any 8086\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Options for dos/exe:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --8086              make compressed exe work on any 8086\n"
                    "  --no-reloc          put no relocations in to the exe header\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Options for dos/sys:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --8086              make compressed sys work on any 8086\n"
                    "\n");
#if 0
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Options for linux/386\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --script             use /usr/local/lib/upx/upx[bd] as decompressor\n"
                    "  --script=/path/upxX  use path/upxX as decompressor\n"
                    "\n");
#endif
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Options for ps1/exe:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --8-bit             uses 8 bit size compression [default: 32 bit]\n"
                    "  --8mib-ram          8 megabyte memory limit [default: 2 MiB]\n"
                    "  --boot-only         disables client/host transfer compatibility\n"
                    "  --no-align          don't align to 2048 bytes [enables: --console-run]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Options for watcom/le:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --le                produce LE output [default: EXE]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"Options for win32/pe, rtm32/pe & arm/pe:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --compress-exports=0    do not compress the export section\n"
                    "  --compress-exports=1    compress the export section [default]\n"
                    "  --compress-icons=0      do not compress any icons\n"
                    "  --compress-icons=1      compress all but the first icon\n"
                    "  --compress-icons=2      compress all but the first icon directory [default]\n"
                    "  --compress-icons=3      compress all icons\n"
                    "  --compress-resources=0  do not compress any resources at all\n"
                    "  --keep-resource=list    do not compress resources specified by list\n"
                    "  --strip-relocs=0        do not strip relocations\n"
                    "  --strip-relocs=1        strip relocations [default]\n"
                    "\n");
        con_fprintf(f,"Options for linux/elf:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --preserve-build-id     copy .gnu.note.build-id to compressed output\n"
                    "\n");
    }

    con_fprintf(f, "file..   executables to (de)compress\n");

    if (verbose > 0)
    {
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"\nThis version supports:\n");
        fg = con_fg(f,fg);
        show_all_packers(f, verbose);
    }
    else
    {
        con_fprintf(f,"\nType '%s --help' for more detailed help.\n", progname);
    }

    con_fprintf(f,"\nNo one will respond for any loss\n");
    UNUSED(fg);
}


/*************************************************************************
//
**************************************************************************/

void show_license(void)
{
    FILE *f = con_term;

    show_head();

    con_fprintf(f,
        "   There have no licence.\n"
        "\n"
    );
    int fg = con_fg(f,FG_YELLOW);
    con_fprintf(f,"        James34602\n");
    (void)con_fg(f,FG_ORANGE);
    con_fprintf(f,
        "\n"        "   James34602               james34602@gmail.com\n"
    );
    fg = con_fg(f,fg);

    UNUSED(fg);
}


/*************************************************************************
//
**************************************************************************/

void show_version(int x)
{
    FILE *fp = stdout;
    const char *v;
    UNUSED(x);
    UNUSED(v);

    fprintf(fp, "upx %s\n", UPX_VERSION_STRING);
#if (WITH_UCL)
    v = upx_ucl_version_string();
    if (v != NULL && v[0])
        fprintf(fp, "UCL data compression library %s\n", v);
#endif
#if (WITH_ZLIB)
    v = upx_zlib_version_string();
    if (v != NULL && v[0])
        fprintf(fp, "zlib data compression library %s\n", v);
#endif
#if (WITH_LZMA)
    v = upx_lzma_version_string();
    if (v != NULL && v[0])
        fprintf(fp, "LZMA SDK version %s\n", v);
#endif
    fprintf(fp, "No one will respond for any loss; for details type '%s -L'.\n", progname);
}
