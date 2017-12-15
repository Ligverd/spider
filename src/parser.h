/***************************************************************************
                          parse.h  -  description
                             -------------------
    begin                : Sun Feb 15 2004
    copyright            : (C) 2004 by maksim
    email                : maksim@m-200.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef PARSE_H
#define PARSE_H
#include <netinet/in.h>

class CParser
{
    int CheckDec(const char *str);
    int CheckIp(const char * ipstr);
    void PrintHelp(void);
public:
    char* sScommIp;
    char* sOutDir;
    char* sFileNameBase;
    char* sPassword;
    char* sLogFile;
    unsigned int nLogFileSize;
    in_addr_t ScommPort;
    in_addr_t ServerPort;
    int rotation;
    bool fDaemon;
    unsigned int nTimeUpdateMin;

    int ParseCStringParams (int argc, char *argv[]);
    int FillMainParams (void);
    CParser();
    ~CParser();
};
#endif
