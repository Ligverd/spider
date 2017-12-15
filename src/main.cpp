/***************************************************************************
 *   Copyright (C) 2007 by PAX   *
 *   pax@m-200.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "main.h"
#include <netinet/tcp.h>
#include <pthread.h>
#include <wait.h>

CSMPClient smp(0);

BYTE mas_mod[MAX_MOD];

int timerA;
int timerB;
int timerC;

bool runTimerA;
bool runTimerB;
bool runTimerC;

struct itimerval real_timer;
struct sigaction sact;

int Scomm_fd = 0;
int Server_fd = 0;
int Client_fd[MAX_CLIENT];

pthread_t Server_tid;

char tfs_file_name[128]; //имя файла tfs
char txt_file_name[128]; //имя файла txt
int fd_tfs_file; // handle на открытый tfs файл
int fd_txt_file; // handle на открытый txt файл

// Configuration variable
CParser Parser;



void get_time_str(char* tm_str, tm T)
{
    sprintf(tm_str,"[%02d-%02d-%04d %02d:%02d:%02d]",T.tm_mday,T.tm_mon,(T.tm_year+1900),T.tm_hour,T.tm_min,T.tm_sec);
}

bool StrToLog(const char* str)
{
    int fd;
    bool fret = true;
    if ( (fd = open(Parser.sLogFile, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
    {
        printf("Can't write to log file:%s\n",Parser.sLogFile);
        return false;
    }
    if(write(fd, str, strlen(str))<0)
        fret = false;
    close(fd);
    return fret;
}

void Loger(const char* str)
{
    char time_str[22];
    char logstr[128];
    time_t itime;
    tm T;
    time (&itime);
    localtime_r(&itime,&T);
    printf("%s",str);
    get_time_str(time_str,T);
    sprintf(logstr,"%s %s",time_str, str);
    StrToLog(logstr);
}



bool make_nonblock(int sock)
{
    int sock_opt;
    if((sock_opt = fcntl(sock, F_GETFL, O_NONBLOCK)) <0) 
    {
        return false;
    }
    if((sock_opt = fcntl(sock, F_SETFL, sock_opt | O_NONBLOCK)) <0)
    {
        return false;
    }
    return true;
}

int Login_ethernet(const char* ip, in_addr_t port)
{
    struct sockaddr_in sock_addr;
    struct hostent *host;

    int fd,sock_opt;
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
    {
        Loger("Can't create scomm socket!\n");
        return -1;
    }

    if(!make_nonblock(fd)) 
    {
        close(fd); 
        Loger("Can't make scomm socket nonblock!\n");
        return -1;
    }

    sock_opt = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)) <0)
    {
        Loger("Can't set scomm socket option SO_REUSEADDR!\n");
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);

    if( inet_aton(ip,&sock_addr.sin_addr) <0) 
    {
        close(fd);
        Loger("Scomm ip error!\n"); 
        return -1;
    }

    host = gethostbyname(ip);
    if (!host) {
      Loger("Error resolve server!\n");
      return -1;
    }
    memcpy(&sock_addr.sin_addr, host->h_addr_list[0], sizeof(sock_addr.sin_addr));

    int state;
    struct timeval tout;
    tout.tv_sec = 20;
    tout.tv_usec = 0;
    fd_set wset,rset;
    FD_ZERO(&wset);
    FD_SET(fd,&wset);
    rset = wset;
    int n;
    if (connect(fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr))<0)
    {
        if (errno != EINPROGRESS)
        { 
            close(fd);
            printf("Connection to scomm Error!\n");
            return -1;
        } 
        state = 1;
        printf("Connection in process...\n");
    }
    if (state) 
    {
        n = select(fd+1, &rset, &wset, NULL, &tout);
        if( n<0 || (FD_ISSET(fd, &wset) && FD_ISSET(fd, &rset)) )
        {
            close(fd);
            printf("Connection to scomm error!\n");
            return -1;
        }
        if(!n)
        {
            close(fd);
            printf("Time out of connection to scomm\n");
            return -1;
        }
    }
    char log[50];
    sprintf(log,"Successfuly connected to:%s\n\n",ip);
    Loger(log);
    return fd;
}

int Create_server_point(in_addr_t port)
{
    struct sockaddr_in server_addr;
    int fd,sock_opt;

    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
    {
        Loger("Can't create spider server socket!\n");
        return -1;
    }

    if(!make_nonblock(fd)) 
    {
        close(fd); 
        Loger("Can't make spider server socket nonblock!\n");
        return -1;
    }

    sock_opt = 1;
    if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&sock_opt,sizeof(sock_opt)) <0)
    {
        printf("Can't set spider server socket option SO_REUSEADDR!\n");
    }

    sock_opt = 1;
    if(setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&sock_opt,sizeof(sock_opt)) <0)
    {
        Loger("Can't set spider server socket option SO_KEEPALIVE!\n");
    }

#ifdef TCP_OPT
    sock_opt = 60;
    if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPIDLE,&sock_opt,sizeof(sock_opt)) <0)
    {
        Loger("Can't set spider server socket option TCP_KEEPIDLE!\n");
    }

    sock_opt = 60;
    if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPINTVL,&sock_opt,sizeof(sock_opt)) <0)
    {
        Loger("Can't set spider server socket option TCP_KEEPINTVL!\n");
    }
#endif
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    char log[64];
    if ( bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <0)
    {
        close(fd);
        sprintf(log,"Error binding server with port:%d\n",port);
        Loger(log);
        return -1;
    }

    if (listen(fd,1) < 0)
    {
        close(fd);
        Loger("Listen spider server socket error!\n");
        return -1;
    }
    sprintf(log,"spider server listening port:%d\n",ntohs(server_addr.sin_port));
    Loger(log);
    return fd;
}



void get_file_names()
{
    tm current_time;
    time_t itime;

    time(&itime);
    localtime_r(&itime,&current_time);

    switch (Parser.rotation)
    {
      case ROTATION_DAY:
          sprintf(tfs_file_name,"%s/%s_%02d_%02d_%d.tfs", Parser.sOutDir, Parser.sFileNameBase, current_time.tm_mday, current_time.tm_mon + 1, current_time.tm_year + 1900);
          sprintf(txt_file_name,"%s/%s_%02d_%02d_%d.txt", Parser.sOutDir, Parser.sFileNameBase, current_time.tm_mday, current_time.tm_mon + 1, current_time.tm_year + 1900);
          break;
      case ROTATION_DECADE:
          sprintf(tfs_file_name,"%s/%s_dec%02d_%d.tfs", Parser.sOutDir, Parser.sFileNameBase, (current_time.tm_yday/10)+1, current_time.tm_year + 1900);
          sprintf(txt_file_name,"%s/%s_dec%02d_%d.txt", Parser.sOutDir, Parser.sFileNameBase, (current_time.tm_yday/10)+1, current_time.tm_year + 1900);
          break;
      case ROTATION_MONTH:
          sprintf(tfs_file_name,"%s/%s_mon%02d_%d.tfs", Parser.sOutDir, Parser.sFileNameBase, current_time.tm_mon + 1, current_time.tm_year + 1900);
          sprintf(txt_file_name,"%s/%s_mon%02d_%d.txt", Parser.sOutDir, Parser.sFileNameBase, current_time.tm_mon + 1, current_time.tm_year + 1900);
          break;
    }
}

void WriteToTfsFile(const BYTE *buf, DWORD len)
{
        int fd;
        char log[200];

        if ( (fd = open(tfs_file_name, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
        {
            sprintf(log, "Can't open file:%s\n",tfs_file_name);
            Loger(log);
            return;
        }

        if(write(fd, buf, len)<0)
        {
            sprintf(log, "Can't write to file:%s\n",tfs_file_name);
            Loger(log);
        }

        close(fd);
}

void WriteToTxtFile(const char *str)
{
        int fd;
        char log[200];

        if ( (fd = open(txt_file_name, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
        {
            sprintf(log,"Can't open file:%s\n",txt_file_name);
            Loger(log);
            return;
        }
        if(write(fd, str, strlen(str))<0)
        {
            sprintf(log, "Can't write to file:%s\n",txt_file_name);
            Loger(log);
        }
        close(fd);
}

void WriteMesToFiles(const CMonMessageEx& MonMes, const BYTE btMod)
{
    BYTE buff[sizeof(DWORD) + MonMessage_MaxMessageSize + sizeof(BYTE)];
    unsigned char *p = MonMes.encode(buff + sizeof(DWORD));
    DWORD size = p - buff - sizeof(DWORD);
    memcpy(buff, &size, sizeof(DWORD));
    memcpy(buff+size+sizeof(DWORD), &btMod, sizeof(BYTE));

    for (int i = 0; i < MAX_CLIENT; i++)
    {
        char log[64];
        if(Client_fd[i])
        {
            struct timeval tv;
            fd_set wset;
            int retval;
            do 
            {
                tv.tv_sec = 1;
                tv.tv_usec = 0;
                FD_ZERO(&wset);
                FD_SET(Client_fd[i], &wset);
                if ((retval = select(Client_fd[i] + 1, NULL, &wset, NULL, &tv)) > 0) 
                {
                    int ERROR=-1;
                    socklen_t opt_size = sizeof(ERROR);
                    getsockopt(Client_fd[i],SOL_SOCKET,SO_ERROR,&ERROR,&opt_size);
                    if(ERROR == 0)
                    {
                        DWORD iLen = size + 1;
                        write(Client_fd[i], &iLen, sizeof(DWORD));
                        write(Client_fd[i], buff + sizeof(DWORD) , iLen);
                        //write(Client_fd, &btMod , sizeof(BYTE));
                    }
                    else
                    {
                        close(Client_fd[i]);
                        Client_fd[i] = 0;
                        sprintf(log, "Client%d socket error!\n",i);
                        Loger(log);
                        break;
                    }
                }
                else if(retval < 0 && errno != EINTR)
                {
                    close(Client_fd[i]);
                    Client_fd[i] = 0;
                    sprintf(log, "Client%d socket internal error!\n",i);
                    Loger(log);
                    break;
                }
            } while(retval<=0);
        }
    }

    char tmp_str[1000];
    MonMes.monMessageToText(tmp_str, 1000);
    strcat(tmp_str,"\r\n");
    if(Parser.rotation != ROTATION_REALTIME)
    {
        get_file_names();
        WriteToTfsFile(buff, sizeof(DWORD) + size);
        WriteToTxtFile(tmp_str);
    }
    printf("%s",tmp_str);
}



void set_timer_a(int timer)
{
    timerA = timer;
}

void set_timer_b(int timer)
{
    timerB = timer;
}

void set_timer_c(int timer)
{
    timerC = timer;
}



void EventModuleDown(BYTE btNum)
{
    CMonMessageEx MonMes(MON_SPIDER_MODULE_DOWN);
    time_t itime;
    tm current_time;

    MonMes.addParameterByte(btNum);

    time(&itime);
    localtime_r(&itime, &current_time);

    TClock clock;
    clock.Date = current_time.tm_mday;
    clock.Month =  current_time.tm_mon;
    clock.Year =  current_time.tm_year - 100;
    clock.Hours = current_time.tm_hour;
    clock.Minutes = current_time.tm_min;
    clock.Seconds = current_time.tm_sec;

    MonMes.addParameterTime(&clock);

    WriteMesToFiles(MonMes, 0xFF);
}

void EventModuleUp(BYTE btNum)
{
    CMonMessageEx MonMes(MON_SPIDER_MODULE_UP);
    time_t itime;
    tm current_time;

    MonMes.addParameterByte(btNum);

    time(&itime);
    localtime_r(&itime, &current_time);

    TClock clock;
    clock.Date = current_time.tm_mday;
    clock.Month =  current_time.tm_mon;
    clock.Year =  current_time.tm_year - 100;
    clock.Hours = current_time.tm_hour;
    clock.Minutes = current_time.tm_min;
    clock.Seconds = current_time.tm_sec;

    MonMes.addParameterTime(&clock);

    WriteMesToFiles(MonMes, 0xFF);
}

void EventTCPUp(void)
{
    CMonMessageEx MonMes(MON_SPIDER_TCP_UP);
    time_t itime;
    tm current_time;

    time(&itime);
    localtime_r(&itime, &current_time);

    TClock clock;
    clock.Date = current_time.tm_mday;
    clock.Month =  current_time.tm_mon;
    clock.Year =  current_time.tm_year - 100;
    clock.Hours = current_time.tm_hour;
    clock.Minutes = current_time.tm_min;
    clock.Seconds = current_time.tm_sec;

    MonMes.addParameterTime(&clock);

    WriteMesToFiles(MonMes, 0xFF);
}

void EventTCPDown(void)
{
    CMonMessageEx MonMes(MON_SPIDER_TCP_DOWN);
    time_t itime;
    tm current_time;

    time(&itime);
    localtime_r(&itime, &current_time);

    TClock clock;
    clock.Date = current_time.tm_mday;
    clock.Month =  current_time.tm_mon;
    clock.Year =  current_time.tm_year - 100;
    clock.Hours = current_time.tm_hour;
    clock.Minutes = current_time.tm_min;
    clock.Seconds = current_time.tm_sec;

    MonMes.addParameterTime(&clock);

    WriteMesToFiles(MonMes, 0xFF);
}

void EventSpiderStart(void)
{
    CMonMessageEx MonMes(MON_SPIDER_START);
    time_t itime;
    tm current_time;

    time(&itime);
    localtime_r(&itime, &current_time);

    TClock clock;
    clock.Date = current_time.tm_mday;
    clock.Month =  current_time.tm_mon;
    clock.Year =  current_time.tm_year - 100;
    clock.Hours = current_time.tm_hour;
    clock.Minutes = current_time.tm_min;
    clock.Seconds = current_time.tm_sec;

    MonMes.addParameterTime(&clock);

    WriteMesToFiles(MonMes, 0xFF);

}

void EventUserStop(void)
{
    CMonMessageEx MonMes(MON_SPIDER_USER_STOP);

    time_t itime;
    tm current_time;
    time(&itime);
    localtime_r(&itime, &current_time);

    TClock clock;
    clock.Date = current_time.tm_mday;
    clock.Month =  current_time.tm_mon;
    clock.Year =  current_time.tm_year - 100;
    clock.Hours = current_time.tm_hour;
    clock.Minutes = current_time.tm_min;
    clock.Seconds = current_time.tm_sec;

    MonMes.addParameterTime(&clock);

    WriteMesToFiles(MonMes, 0xFF);
}

bool Pipe(int *filedes)
{
    if(pipe(filedes)<0)
        return false;
    return true;
}

int Uname(char *instr, int intrlen)
{
    int pid, status, filedes[2];
    if(!(Pipe(filedes) && make_nonblock(filedes[0]) && make_nonblock(filedes[1])))
        return -1;
    pid = fork();
    switch(pid)
    {
        case -1:
            return -1;
        break;
        case 0:
        {
            dup2(filedes[1],1);
            if(execl("/bin/uname","/bin/uname","-s","-m","-r",NULL)<0)
                return -1;
            exit(EXIT_FAILURE);
        }
        break;
        default: {status =-1; wait(&status);}
    }
    fd_set rset;
    FD_ZERO(&rset); // Set Zero
    FD_SET(filedes[0], &rset);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    char str[1024],*ch;
    if ((select( filedes[0] + 1, &rset, NULL, NULL, &tv)) > 0)
    {
        ch = str;
        while(read(filedes[0], ch++, 1) == 1)
        {
            if(*(ch-1) == '\n' || *(ch-1) == '\r')
            {
                *(ch-1) = 0;
                if(strlen(str) && (strlen(str)+1) < intrlen)
                { 
                    strcpy(instr, str);
                    return 1;
                }
                ch = str;
            }
        }
    }
    return -1;
}

void EventSpiderInfo(void)
{

    CMonMessageEx MonMes(MON_SPIDER_INFO_EX);

    char OS[100];
    if(Uname(OS, 100)<0)
        strcpy(OS,"Linux");
    DWORD dwSPIDERver = 0x02000000;

    MonMes.addParameterString(OS);
    MonMes.addParameterDWord(dwSPIDERver);

    time_t itime;
    tm current_time;
    time(&itime);
    localtime_r(&itime, &current_time);

    TClock clock;
    clock.Date = current_time.tm_mday;
    clock.Month =  current_time.tm_mon;
    clock.Year =  current_time.tm_year - 100;
    clock.Hours = current_time.tm_hour;
    clock.Minutes = current_time.tm_min;
    clock.Seconds = current_time.tm_sec;

    MonMes.addParameterTime(&clock);

    WriteMesToFiles(MonMes, 0xFF);
}



void sig_SIGPIPE_hndlr(int signo)
{
    Loger("Signal SIGPIPE received!\n");
}

bool fRecur = false;

void sig_SIGTERM_hndlr(int signo)
{
    if(!fRecur)
    {
        fRecur = true;
        EventUserStop();
        Loger("Terminating process...\n");
        close(Scomm_fd);
        close(Server_fd);
        for( int i = 0; i < MAX_CLIENT ;i++ )
        {
            close(Client_fd[i]);
        }
    }
    exit(signo);
}

void sig_SIGALRM_hndlr(int signo)
{
    if (timerA > 0) {
            timerA--;
       if (timerA == 0)
        {
           runTimerA = true;
        }
    }

    if (timerB > 0) {
            timerB--;
    if (timerB == 0)
      {
            runTimerB = true;
      }
    }

    if (timerC > 0) {
            timerC--;
       if (timerC == 0)
       {
            runTimerC = true;
        }
    }
    real_timer.it_value.tv_sec = 0;
    real_timer.it_value.tv_usec = 100000;
    setitimer(ITIMER_REAL, &real_timer, NULL);
}



void OnMessage(CSMPMessage *mes)
{
     switch (mes->mes)
     {
        case SMPMESCLIENT_CONNECT_OK:
            EventTCPUp();
            smp.SwitchBinary(Parser.sPassword);
            break;
        case SMPMESCLIENT_CONNECT_ERROR:
            break;
        case SMPMESCLIENT_CLOSE:
            {
                for (int i = 0; i < MAX_MOD; i++)
                    mas_mod[i] = 255;
                EventTCPDown();
            }
            break;
        case SMPMESCLIENT_BINARYMODE_ER:
        {
            EventTCPDown();
            Loger("Invalid password!\n");
            sig_SIGTERM_hndlr(1);
        } break;

        case SMPMESCLIENT_BINARYMODE_OK:
        { 
            Loger("BINARYMODE_OK\n"); 
            CNetMessageBody xmes;
            xmes.dst.nMod = 0xFF;
            xmes.nMessage = NET_MES_REGISTER_MONITOR;
            xmes.un.multi.set.Full();

            uc buf[sizeof(CNetMessageBody)];
            uc* p = xmes.encode(buf);
            smp.SendPacket(buf, p-buf);
            set_timer_b(TIMER_SEND_REGISTER_MONITOR_VALUE);
        }
        break;

        case SMPMESCLIENT_PACKET:
           {
                CNetMessageBody net;
                net.decode((unsigned char*)mes->data, mes->len);
                    //(CNetMessage*)mes->data;
                switch (net.nMessage)
                {
                    case NET_MES_MONITOR:
                        if(mas_mod[net.src.nMod] >= 3)
                            EventModuleUp(net.src.nMod);
                        mas_mod[net.src.nMod] = 0;
					
					   //DWORD dwLen = mes->len - net->nHead();
					   if(net.dataSize > 2*MAXSIZEDATAMON + sizeof(BYTE) + sizeof(TCallID) + sizeof(BYTE))
                        {
						Loger("NetMessage lengh error!\n");
					   }
					   CMonMessageEx MonMes(0);
					   unsigned char *p = MonMes.decode(net.data, net.dataSize);
					   if (p) 
					   {
                            WriteMesToFiles(MonMes, net.src.nMod);
                        }
                        else 
					    {
						  Loger("Error decode MonMessageEx!\n");
                        }
                    break;
                }
            }
     }
    delete mes;
}



void timer_a()
{
}

void timer_b()
{
    for (int i = 0; i < MAX_MOD; i++)
    {
        if (mas_mod[i] == 3)
        {
            EventModuleDown(i);
            mas_mod[i]++;
        }  
        else 
        {
            if (mas_mod[i] < 3)
                   mas_mod[i]++;
        }
    }

    CNetMessageBody xmes;
    xmes.dst.nMod = 0xFF;
    xmes.nMessage = NET_MES_REGISTER_MONITOR;
    xmes.un.multi.set.Full();

    uc buf[sizeof(CNetMessageBody)];
    uc* p = xmes.encode(buf);

    smp.SendPacket(buf, p-buf);

    set_timer_b(TIMER_SEND_REGISTER_MONITOR_VALUE);
}

void timer_c()
{
}



void Reinit_connection()
{
    close(Scomm_fd);
    sact.sa_handler = SIG_IGN;
    sigaction(SIGALRM, &sact, NULL);
    while ((Scomm_fd = Login_ethernet(Parser.sScommIp, Parser.ScommPort)) < 0) 
    {
        sleep(5);
    }
    set_timer_b(-1);
    smp.OnConnect();
    sact.sa_handler = sig_SIGALRM_hndlr;
    sigaction(SIGALRM, &sact, NULL);
    real_timer.it_value.tv_sec = 0;
    real_timer.it_value.tv_usec = 100000;
    setitimer(ITIMER_REAL, &real_timer, NULL);

}



int max(int x, int y)
{
    return x > y ? x : y;
}

void *Server_ptread(void* arg)
{
    char log[80];
    char sClient_ip[MAX_CLIENT][16];
    pthread_detach(pthread_self());
    fd_set fds;
    struct timeval tv;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int retval;
    int i,maxfd;
    while(1)
    {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        for (i = 0; i < MAX_CLIENT; i++) 
        {
            if (Client_fd[i] > 0) 
            {
                FD_SET(Client_fd[i], &fds);
                maxfd = max(maxfd, Client_fd[i]);
            }
        }
        if(Server_fd) FD_SET(Server_fd, &fds);

        if ((retval =  select( max( Server_fd, maxfd) + 1, &fds, NULL, NULL, &tv)) > 0)
        {
            if (FD_ISSET(Server_fd, &fds) > 0)
            {
                int clnt_fd = accept(Server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
                if (clnt_fd < 0)
                {
                    Loger("Server: Client accept error!\n");
                }
                else
                {
                    for (i = 0; i < MAX_CLIENT; i++) 
                                if (!Client_fd[i]) break;
                    if (i < MAX_CLIENT)// Esli kanal s ATS ne rabochij
                    {
                        if (make_nonblock(clnt_fd)<0)
                        {
                            close(clnt_fd);
                           sprintf(log,"Server: Can't make client socket nonblock! IP:%s\n", inet_ntoa(client_addr.sin_addr));
                            Loger(log);
                        }
                        else
                        {
                            Client_fd[i] = clnt_fd;
                            strcpy(sClient_ip[i],inet_ntoa(client_addr.sin_addr));
                            sprintf(log,"Server: Client%d connected! IP:%s\n", i, sClient_ip[i]);
                            Loger(log);
                            write(Client_fd[i], "I_AM_SPIDER\0", strlen("I_AM_SPIDER")+1);
                        }
                    }
                    else//Esli kanal rabochij dobavliajem novogo klienta
                    {
                        close(clnt_fd);
                        sprintf(log,"Server: Client%d accept denied! IP:%s\n", i, inet_ntoa(client_addr.sin_addr));
                        Loger(log);
                    }
                }
            }
            for (i = 0; i < MAX_CLIENT; i++)
            {
                if (FD_ISSET(Client_fd[i], &fds) > 0)
                {
                    unsigned char temp_buff[64];
                    int size  = read(Client_fd[i], temp_buff, 64);
                    if (size > 0) // if rcv data from client socket
                    {
                    }   
                    else if ((size == 0) || ((size < 0)&&(errno != EINTR)))
                    {
                        sprintf(log, "Server: Client%d disconnected! IP:%s\n", i, sClient_ip[i]);
                        Loger(log);
                        close(Client_fd[i]);
                        Client_fd[i] = 0;
                    }
                }
            }
        }
        else if (retval < 0 && errno != EINTR)//Esli oshibka ne sviazana s prepivanijem signalom
        {
            Loger("Server internal error!\n");
        }
    }
    close(Server_fd);
    return(NULL);
}

int main(int argc, char *argv[])
{
    if (Parser.ParseCStringParams(argc, argv) < 0)
    {
        exit(1);
    }

    printf("<-------------------------------Spider v2.0---------------------------->\n");
    if (Parser.FillMainParams() < 0)
    {
        exit(1);
    }
    printf("<---------------------------------------------------------------------->\n");

    StrToLog("Spider started....\n");

    if(Parser.rotation != ROTATION_REALTIME)
    {
        get_file_names();

        char log[200];
        if (open(tfs_file_name, O_RDWR | O_CREAT | O_APPEND , 0666 ) < 0)
        {
        sprintf(log,"Can't open file:%s!\n",tfs_file_name);
        Loger(log);
        exit(1);
        }

        if (open(txt_file_name, O_RDWR | O_CREAT | O_APPEND , 0666 ) < 0)
        {
        sprintf(log,"Can't open file:%s!\n",txt_file_name);
        Loger(log);
        exit(1);
        }
    }

    if( (Server_fd = Create_server_point(Parser.ServerPort)) <0) 
    {
        Loger("Error create server point!\r\n");
        Server_fd = 0;
    }

    for( int i = 0; i < MAX_CLIENT ;i++ )
    {
        Client_fd[i] = 0;
    }

    timerA = timerB = timerC = -1;
    runTimerA = false;
    runTimerB = false;
    runTimerC = false;

    for (int i = 0; i < MAX_MOD; i++)
        mas_mod[i] = 255;

    if (Parser.fDaemon)
        daemon(0,0);

    while((Scomm_fd = Login_ethernet(Parser.sScommIp, Parser.ScommPort)) < 0)
    {
        sleep(5);
    }


    EventSpiderStart();
    EventSpiderInfo();
    smp.OnConnect();

    if(Server_fd)
    {
        if (pthread_create(&Server_tid,NULL,&Server_ptread,NULL)!=0)
        {
            Loger("Can't create server tread!\n");
        }
    }

    sigfillset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = sig_SIGTERM_hndlr;
    sigaction(SIGINT, &sact, NULL);
    sigaction(SIGTERM, &sact, NULL);

    sigemptyset(&sact.sa_mask);
    sact.sa_handler = sig_SIGPIPE_hndlr;
    sigaction(SIGPIPE, &sact, NULL);

    sact.sa_handler = sig_SIGALRM_hndlr;
    //sact.sa_flags |= SA_RESTART;
    sigaction(SIGALRM, &sact, NULL);

    real_timer.it_value.tv_sec = 0;
    real_timer.it_value.tv_usec = 100000;
    setitimer(ITIMER_REAL, &real_timer, NULL);

    struct timeval tv;
    fd_set fds;
    int retval;
   while (1)
   {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(Scomm_fd, &fds);
        if ((retval =  select(Scomm_fd + 1, &fds, NULL, NULL, &tv)) > 0)
        {
            char buffer[1024];
            int size = read(Scomm_fd, buffer, 1024);
            if (size > 0) // if rcv data from tty
            {
                smp.OnReceive(buffer, size);
            }
            else
            {
                if (!size || (size < 0 && errno != EINTR))
                {
                     Loger("Scomm socket closed!\n");
                     smp.OnClose();
                }
            }
        }
        else if (retval < 0 && errno != EINTR)//Esli oshibka ne sviazana s prepivanijem signalom
        {
            Loger("Internal error!\n");
            smp.OnClose();
        }

        if(!smp.fOpenConnection)
        {
            Loger("Reconnecting...\n");
            Reinit_connection();
        }

        if (runTimerA) {
           runTimerA = false;
           timer_a();
        }
        if (runTimerB) {
           runTimerB = false;
           timer_b();
        }
        if (runTimerC) {
           runTimerC = false;
           timer_c();
        }
  }
   return EXIT_SUCCESS;
}
