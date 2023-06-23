/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QString>

#include "unzip.h"
#define WRITEBUFFERSIZE (8192)
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)

class QGCZip
{
public:
    unzFile uf;
    int opt_extract_without_path;
    int opt_overwrite;


    static int do_extract(unzFile uf,int opt_extract_without_path,int opt_overwrite, const char* filepath);
    static int do_extract_currentfile(unzFile uf,const int* popt_extract_without_path,int* popt_overwrite, const char* filepath);


};
