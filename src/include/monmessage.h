#ifndef  __MONMESSAGE_H__
#define  __MONMESSAGE_H__

//#pragma pack(1)

#include  <include/type.h>
#include  <include/lib.h>


// ��������� ��� ��������� �������, ��� �������� ���������� ������� ��������� �� e-mail: sapr@m-200.com
const unsigned char  LINK_INT_OK                    = 0; // ������� ���������� (�����)
const unsigned char  LINK_INT_ERROR                 = 1; // ������� ���������� ����� (������0
const unsigned char  LINK_EXT_OK                    = 2; // ������� ������� ����� (�������)
const unsigned char  LINK_EXT_ERROR                 = 3; // ������� ������� ����� (�������)
const unsigned char  LINK_NONE                      = 4; // ����������� ����� (������)
const unsigned char  LINK_BLOCK                     = 5; // ����������
// �� ������


// ���������
const   unsigned char   MON_ALIVE                   =  1;   // module:BYTE  time:TClock
const   unsigned char   MON_SPIDER_START            =  2;   // time:TClock
const   unsigned char   MON_SPIDER_MODULE_UP        =  3;   // module:BYTE  time:TClock
const   unsigned char   MON_SPIDER_MODULE_DOWN      =  4;   // module:BYTE  time:TClock
const   unsigned char   MON_SPIDER_TCP_UP           =  5;   // time:TClock
const   unsigned char   MON_SPIDER_TCP_DOWN         =  6;   // time:TClock
const   unsigned char   MON_SPIDER_USER_STOP        =  7;   // time:TClock
const   unsigned char   MON_SPIDER_INFO             =  8;   // OSver:DWORD  SPIDERver:DWORD time:TClock
const   unsigned char   MON_SPIDER_INFO_EX          =  9;   // OSver:String SPIDERver:DWORD time:TClock
const   unsigned char   MON_SPIDER_BAD_PACKET       = 60;   // time:TClock
const   unsigned char   MON_SPIDER_GATE_LOST		= 61;   // module:BYTE time:TClock
// �� ������
const   unsigned char   MON_SEIZURE                 = 10;   // sig:BYTE module:BYTE slot/pcm:BYTE hole/ki:BYTE number:STRING aon:STRING
const   unsigned char   MON_NUMBER                  = 11;   // number:STRING aon:STRING // ����� �� ����� ���
const   unsigned char   MON_ACCEPT                  = 12;   // remID:DWORD
const   unsigned char   MON_CALL                    = 13;   // remID:DWORD sig:BYTE module:BYTE slot/pcm:BYTE port/ki:BYTE number:STRING aon:STRING
const   unsigned char   MON_ANSWER                  = 14;   // ---
const   unsigned char   MON_RELEASE_IN              = 15;   // reason:BYTE
const   unsigned char   MON_RELEASE_OUT             = 16;   // reason:BYTE
const   unsigned char   MON_COMBINE                 = 17;   // secID:DWORD
const   unsigned char   MON_COMOVERLOAD             = 18;   // module:BYTE time:TClock
const   unsigned char   MON_WIZOVERLOAD             = 24;   // module:BYTE time:TClock
const   unsigned char   MON_FATOVERLOAD             = 25;   // module:BYTE time:TClock
const   unsigned char   MON_TIMECHANGE              = 19;   // module:BYTE time:TClock
const   unsigned char   MON_DVO_REDIRECT            = 50;   // dvocode:BYTE number_form:STRING number_to:STRING
const   unsigned char   MON_HOLDEND                 = 51;   // ---
// �� ���������� �����, ID ����� � ��� ������� �� ���������� ������
const   unsigned char   MON_INT_CALL_OUT            = 20;   // module:BYTE pcm:BYTE ki:BYTE
const   unsigned char   MON_INT_CALL_IN             = 21;   // module:BYTE pcm:BYTE ki:BYTE
const   unsigned char   MON_INT_RLS_OUT             = 22;   // module:BYTE pcm:BYTE ki:BYTE
const   unsigned char   MON_INT_RLS_IN              = 23;   // module:BYTE pcm:BYTE ki:BYTE
// ��������� �� ���������� �������
const   unsigned char   MON_LINK_STATE              = 30;   // ����� �������������:BYTE ����� �������:BYTE
                                                            // ������ {
                                                            //      ���������:BYTE
                                                            //      ����� ��������� ��������� ����������:BYTE
                                                            //      ������ �����:BYTE
                                                            //      ����� �����:BYTE
                                                            //    }[����� �������]
// ��������� �� ������ ��� �������������� ������
const   unsigned char   MON_PORT_FREE               = 40;   // module:BYTE slot/pcm:BYTE hole/ki:BYTE


#define MAXSIZEDATAMON    100
const   int MonMessage_MaxMessageSize   = 6 + MAXSIZEDATAMON;   // ����������� ��������� + ������

// ��������� ��� �����, ����������� � �����������
// !��������! ������ ������ � ����� ��������!
//static int CMonMessageEx_readPos_;
class CMonMessageEx
{
private:
//    static int      readPos_;               // ��������� ���������� ��� �������� ��������� ������ ������

    BYTE            time_;                  // ������������� ����� � ��������
    TCallID         id_;                    // ������������� ������
    BYTE            message_;               // �������

    unsigned char   size_;                  // ������
    BYTE            data_[MAXSIZEDATAMON];  // ��������� �������

    // ������ � ������

public:
    //CMonMessageEx();
    CMonMessageEx(BYTE message);

    //void  clearReadPosition() const { CMonMessageEx_readPos_ = 0; }
    void  clearBuffer() { size_ = 0; }
    //int  getFullSize() { return size + sizeof(time_) + sizeof(id_) + sizeof(message_); }

    unsigned char* encode(unsigned char* stream) const;
    unsigned char* decode(unsigned char* stream, int size);

    // ����� � ��������� ����������
    void  time(BYTE time) { time_ = time; }
    BYTE  time() const { return time_; }

    void  id(TCallID id) { id_ = id; }
    TCallID  id() const { return id_; }

    void  message(BYTE message) { message_ = message; }
    BYTE  message() const { return message_; }

    BYTE  bufferSize() const { return size_; }


    // ������
    void addParameterByte(BYTE b)           { data_[size_++] = b; }
    void addParameterDWord(DWORD d) { 
    #ifdef __ARM__
        *(data_+size_++) = (BYTE)d;
        *(data_+size_++) = (BYTE)(d >> 8);
        *(data_+size_++) = (BYTE)(d >> 16);
        *(data_+size_++) = (BYTE)(d >> 24);
    #else
        *(DWORD*)&data_[size_] = d; size_ += sizeof(DWORD); 
    #endif
    }

    void addParameterString(const char* s);
    void addParameterTime(const TClock *cl);

    // ������
    BYTE    getParameterByte(int& readPos) const  { return data_[readPos++]; }
    DWORD   getParameterDWord(int& readPos) const {
        #ifdef __ARM__
            DWORD d = *(data_+readPos++);
                d |= *(data_+readPos++) << 8;
                d |= *(data_+readPos++) << 16;
                d |= *(data_+readPos++) << 24;
        #else
            DWORD d = *(DWORD*)(data_+readPos); 
            readPos += sizeof(DWORD); 
        #endif
        return  d;
    }
    const char*   getParameterStringPtr(int& readPos) const;
    void  getParameterTime(TClock*, int& readPos) const;
    const TClock*  getParameterTimePtr(int& readPos) const;

    bool monMessageToText(char* pTextBuffer, int iTextBufferSize) const;
};

//#pragma pack()

#endif
