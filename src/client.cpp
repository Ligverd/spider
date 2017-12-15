#include "main.h"

CSMPClient::CSMPClient (DWORD _idc_back)
{
    idc_back = _idc_back;
    fBinaryRead = false;
    fBinaryWrite = false;
    recvLen = 0;
    fOpenConnection = false;
}

CSMPClient::~CSMPClient ()
{

}

void CSMPClient::OnClose(void)
{
    if (fOpenConnection)
    {
        fOpenConnection = false;
        CSMPMessage* mes = new CSMPMessage(SMPMESCLIENT_CLOSE);
        if(mes)
        {
            OnMessage(mes);
        }
        else
            Loger("Warning: Out of memory!\n");
    }
}

void CSMPClient::OnConnect(void)
{
    fBinaryRead = false;
    fBinaryWrite = false;
    recvLen = 0;
    fOpenConnection = true;
    CSMPMessage* mes = new CSMPMessage(SMPMESCLIENT_CONNECT_OK);
    if(mes)
    {
        OnMessage(mes);
    }
    else
        Loger("Warning: Out of memory!\n");
}


void CSMPClient::OnReceive(char *buffer, int len)
{
    memcpy(recvBuf + recvLen, buffer, len);
    recvLen += len;
    if (!fBinaryRead)
    {
        int done = 0;
        for (int p = 0; p < recvLen && !fBinaryRead; )
        {
            if (recvBuf[p] == 0xA)
            {
                memmove(recvBuf + p, recvBuf + p + 1, recvLen - p - 1);
                recvLen --;
                continue;
            }
            if (recvBuf[p] == 0xD)
            {
                if (!strncmp((char *)(recvBuf + done), "BINARYMODE-OK", 10+1+2))
                {
                    fBinaryRead = true;
                    CSMPMessage *mes = new CSMPMessage(SMPMESCLIENT_BINARYMODE_OK);
                    if(mes)
                    {
                        OnMessage(mes);
                    }
                    else
                        Loger("Warning: Out of memory!\n");
                }
                if (!strncmp((char *)(recvBuf + done), "BINARYMODE-ER", 10+1+2))
                {
                    fBinaryRead = true;
                    CSMPMessage *mes = new CSMPMessage(SMPMESCLIENT_BINARYMODE_ER);
                    if(mes)
                    {
                        OnMessage(mes);
                    }
                    else
                        Loger("Warning: Out of memory!\n");
                }
                else
                {
                    ReceivePacket(recvBuf + done, p - done);
                }
                p ++;
                if (recvBuf[p] == 0xA)
                    p ++;
                done = p;
            }
            else
                p ++;
        } //if binary mode ok
        if (done < recvLen)
        {
            memmove(recvBuf, recvBuf  + done, recvLen - done);
            recvLen -= done;
        }
        else
            recvLen = 0;
    }
    if (fBinaryRead)
    {
        while (recvLen >= 2)
        {
            WORD size = (WORD)(*recvBuf) + ((WORD)*(recvBuf+1) << 8);
            if (size > MAXSMPPACKET)
            {
                CSMPMessage* mes = new CSMPMessage(SMPMESCLIENT_BADPACKET);
                if(mes)
                {
                    OnMessage(mes);
                }
                else
                    Loger("Warning: Out of memory!\n");
                recvLen = 0;
                return;
            }
            if (size + 2 > recvLen)
                return;
            ReceivePacket(recvBuf + 2, size);
            if (recvLen > size + 2)
            {
                memmove(recvBuf, recvBuf + size + 2, recvLen - size - 2);
                recvLen -= size + 2;
            }
            else
                recvLen = 0;
        }
    }

}

void CSMPClient::ReceivePacket (BYTE *data, short len )
{
	if (!fBinaryRead)
	{
        if (len < 0)
            return;
        CSMPMessage* mes = (CSMPMessage*) new BYTE[sizeof(CSMPMessage) + 1 + len];
        if(mes)
        {
            memcpy(mes->data, (char*)data, len);
            mes->data[len] = 0;
            mes->mes = SMPMESCLIENT_STRING;
            mes->len = len + 1;
            OnMessage(mes);
        }
        else
            Loger("Warning: Out of memory!\n");
	}
	else
	{
        if (len <= 0)
            return;
        CSMPMessage* mes = (CSMPMessage*) new BYTE[sizeof(CSMPMessage) + len];
        if(mes)
        {
            mes->mes = SMPMESCLIENT_PACKET;
            mes->len = len;
            memcpy(mes->data, data, len);
            OnMessage(mes);
        }
        else
            Loger("Warning: Out of memory!\n");
	}
}

void CSMPClient::SendPacket ( BYTE* data, short len )
{
    if (len > 0 && len < MAXSMPPACKET && fBinaryWrite && fOpenConnection)
    {
        struct timeval tv;
        fd_set wset;
        int retval;
        do 
        {
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            FD_ZERO(&wset);
            FD_SET(Scomm_fd, &wset);
            if ((retval = select(Scomm_fd + 1, NULL, &wset, NULL, &tv)) > 0) 
            {
                int ERROR=-1;
                socklen_t opt_size = sizeof(ERROR);
                getsockopt(Scomm_fd, SOL_SOCKET, SO_ERROR, &ERROR, &opt_size);
                if(ERROR == 0)
                {
                    write(Scomm_fd, &len, 2);
                    write(Scomm_fd, data, len);
                }
                else
                {
                    Loger("Scomm socket error!\n");
                    OnClose();
                    break;
                }
            }
            else if(retval < 0 && errno != EINTR)
            {
                Loger("Scomm socket internal error!\n");
                OnClose();
                break;
            }
        } while(retval <= 0);
    }
}

void CSMPClient::SendString ( char* str )
{
     if (!fBinaryWrite && strlen(str) < MAXSMPPACKET && fOpenConnection)
    {
        struct timeval tv;
        fd_set wset;
        int retval;
        do 
        {
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            FD_ZERO(&wset);
            FD_SET(Scomm_fd, &wset);
            if ((retval = select(Scomm_fd + 1, NULL, &wset, NULL, &tv)) > 0) 
            {
                int ERROR=-1;
                socklen_t opt_size = sizeof(ERROR);
                getsockopt(Scomm_fd, SOL_SOCKET, SO_ERROR,&ERROR, &opt_size);
                if(ERROR == 0)
                {
                    //char buff[MAXSMPPACKET + 2];
                    //memcpy(buff,str, strlen(str));
                    //memcpy(buff + strlen(str), "\r\n",2);
                    write(Scomm_fd, str, strlen(str)/*+2*/);
                }
                else
                {
                    Loger("Scomm socket error!\n");
                    OnClose();
                    break;
                }
            }
            else if(retval < 0 &&  errno != EINTR)
            {
                Loger("Scomm socket internal error!\n");
                OnClose();
                break;
            }
        } while(retval <= 0);
    }
}

void CSMPClient::SwitchBinary (const char *password)
{
    char str[10+1+6+strlen("\r\n")+1];
    sprintf(str, "BINARYMODE-%s\r\n", strlen(password) == 6 ? password : "100100");
    if (!fBinaryWrite)
    {
        SendString(str);
        fBinaryWrite = true;
    }
}
