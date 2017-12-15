/***************************************************************************
                          main.h  -  description
                             -------------------
    begin                : Sun Feb 22 2004
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
#ifndef _MAIN_H
#define _MAIN_H

//#define FREE_BSD 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#ifdef FREE_BSD
    #include <sys/wait.h>
#else
    #include <wait.h>
#endif

#ifdef FREE_BSD
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/queue.h>
#include <netinet/tcp_var.h>
#define TCP_KEEPIDLE TCPCTL_KEEPIDLE
#define TCP_KEEPINTVL TCPCTL_KEEPINTVL
#endif

#include <config.h>
#include "client.h"
#include "type.h"
#include "netmes.h"
#include "monmessage.h"
#include "unipar.h"
#include "parser.h"

#define TIMER_SEND_REGISTER_MONITOR_VALUE 100

#define MAX_MOD 64

#define ROTATION_DAY 1
#define ROTATION_DECADE 2
#define ROTATION_MONTH 3
#define ROTATION_REALTIME 4

#define MAX_CLIENT 3

extern int Scomm_fd;
void OnMessage(CSMPMessage *mes);
void Loger(const char* str);

#endif
