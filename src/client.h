#ifndef _CLIENT_H
#define _CLIENT_H

#include "include/type.h"

#define WM_COMMAND 0x100

#define MAXSMPPACKET 200
#define MAXSMPBUFFER 10000
#define MAX_MODULE 64


#define SMPMESCLIENT_NONE          0
#define SMPMESCLIENT_CONNECT_OK    1
#define SMPMESCLIENT_CONNECT_ERROR 2
#define SMPMESCLIENT_CLOSE         3
#define SMPMESCLIENT_BINARYMODE    4
#define SMPMESCLIENT_BADPACKET     5
#define SMPMESCLIENT_STRING        6
#define SMPMESCLIENT_PACKET        7
#define SMPMESCLIENT_BINARYMODE_OK 8
#define SMPMESCLIENT_BINARYMODE_ER 9

class CSMPMessage
{
public:
    CSMPMessage( int _mes ) { mes = _mes; len = 0; }

    int mes;
    int len;
    BYTE data[1];
};

class CSMPClient
{
public:
    CSMPClient (DWORD _idc_back);
    ~CSMPClient ();

    void SwitchBinary ( const char * password);

    void SendPacket ( BYTE* data, short len );
    void SendString ( char* str );

    void OnConnect(void);
    void OnClose(void);

    void OnReceive(char *buffer, int len);
    void ReceivePacket ( BYTE* data, short len );
     bool fOpenConnection;
private:
    bool fBinaryRead, fBinaryWrite;
    BYTE recvBuf[MAXSMPBUFFER];
    int recvLen;
    DWORD idc_back;
};

#endif
