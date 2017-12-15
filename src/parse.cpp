#include "main.h"

CParser::CParser()
{
    sScommIp = NULL;
    sOutDir = NULL;
    sFileNameBase = NULL;
    sPassword = NULL;
    sLogFile = NULL;
    in_addr_t ScommPort = 10001;
    in_addr_t ServerPort = 0;
    rotation = 1;
    nTimeUpdateMin = 0;
    fDaemon = false;
}

CParser::~CParser()
{
    if(sScommIp) free(sScommIp);
    if(sOutDir) free(sOutDir);
    if(sFileNameBase) free(sFileNameBase);
    if(sPassword) free(sPassword);
    if(sLogFile) free(sLogFile);
}

int CParser::CheckDec(const char *str)
{
    int i;
    for (i = 0; i < strlen(str); i++)
        if (str[i] > '9' || str[i] < '0')
            return 0;
    return 1;
}

int CParser::CheckIp(const char * ipstr)
{

    if (!strcmp(ipstr,"localhost"))
    {
        return 1;
    }

    int len = strlen(ipstr);

    if (len>6 && len<16)
    {
        char i=0,p=0,dot=0,s[len+1];
        memset(s,0,len+1);  
        while(i<=len)
        {
            if(i==len)
            {
                if(dot!=3) return 0;
                if((atoi(s)<0) || (atoi(s)>255)) return 0;
                break;
            }
            if (ipstr[i]=='.')
            {
                dot++;
                if((atoi(s)<0) || (atoi(s)>255) || p == 0) return 0;
                memset(s,0,p);
                i++;
                p=0;
            }
            if (ipstr[i]> '9' || ipstr[i]< '0')return 0;
            s[p++]=ipstr[i++];
        }
    }
    else return 0;
    return 1;
}


int CParser::ParseCStringParams (int argc, char *argv[])
{
    if (argc == 2 &&  !strcmp(argv[1],"-h"))
    {
        PrintHelp();
        return -1;
    }
    int i;
    for( i = 1; i<argc; i++)
    {
        if(!strcmp(argv[i],"-scommip"))
        {
            i++;
            if (i < argc)
            {
                if(CheckIp(argv[i]))
                {
                    sScommIp = (char*)malloc(strlen(argv[i])+1);
                    if (!sScommIp)
                    {
                        printf("Out of memory!\n");
                        return -1;
                    }
                    strcpy(sScommIp, argv[i]);
                }
                else
                {
                    printf("Scomm ip address error!\n");
                    PrintHelp();
                    return -1;
                }
            }
        }
        if(!strcmp(argv[i],"-logfile"))
        {
            i++;
            if (i < argc)
            {
                sLogFile = (char*)malloc(strlen(argv[i])+1);
                if (!sLogFile)
                {
                    printf("Out of memory!\n");
                    return -1;
                }
                strcpy(sLogFile, argv[i]);
            }
        }
        else if(!strcmp(argv[i],"-outdir"))
        {
            if(!sOutDir)
            {
                i++;
                if (i < argc)
                {
                    sOutDir = (char*)malloc(strlen(argv[i])+1);
                    if (!sOutDir)
                    {
                        printf("Out of memory!\n");
                        return -1;
                    }
                    strcpy(sOutDir, argv[i]);
                }
            }
        }
        else if(!strcmp(argv[i],"-filename"))
        {
            if(!sFileNameBase)
            {
                i++;
                if (i < argc)
                {
                    sFileNameBase = (char*)malloc(strlen(argv[i])+1);
                    if (!sFileNameBase)
                    {
                        printf("Out of memory!\n");
                        return -1;
                    }
                    strcpy(sFileNameBase, argv[i]);
                }
            }
        }
        else if(!strcmp(argv[i],"-outfile"))
        {
            if(!sOutDir && !sFileNameBase)
            {
                i++;
                if (i < argc)
                {
                    int len = strlen(argv[i]) - strlen(strrchr( argv[i], (int)'/'));
                    sOutDir = (char*)malloc(len + 1);
                    if (!sOutDir)
                    {
                        printf("Out of memory!\n");
                        return -1;
                    }
                    sOutDir[len] = 0;
                    memcpy(sOutDir, argv[i], len);

                    sFileNameBase = (char*)malloc(strlen(strrchr( argv[i], (int)'/')+1)+1);
                    if (!sFileNameBase)
                    {
                        printf("Out of memory!\n");
                        return -1;
                    }
                    strcpy(sFileNameBase, strrchr( argv[i], (int)'/')+1);
                }
            }
        }
        else if(!strcmp(argv[i],"-password"))
        {
            i++;
            if (i < argc)
            {
                sPassword = (char*)malloc(strlen(argv[i])+1);
                if (!sPassword)
                {
                        printf("Out of memory!\n");
                        return -1;
                }
                strcpy(sPassword, argv[i]);
            }
        }
        else if(!strcmp(argv[i],"-scommport"))
        {
            i++;
            if (i < argc)
            {
                if (CheckDec(argv[i]))
                {
                    ScommPort = atoi(argv[i]);
                }
            }
        }
        else if(!strcmp(argv[i],"-serverport"))
        {
            i++;
            if (i < argc)
            {
                if (CheckDec(argv[i]))
                {
                    ServerPort = atoi(argv[i]);
                }
            }
        }
        else if(!strcmp(argv[i],"-rotation"))
        {
            i++;
            if (i < argc)
            {
                if (CheckDec(argv[i]))
                {
                    rotation = atoi(argv[i]);
                }
            }
        }
        else if(!strcmp(argv[i],"-timeupdate"))
        {
            i++;
            if (i < argc)
            {
                if (CheckDec(argv[i]))
                {
                    nTimeUpdateMin = atoi(argv[i]);
                }
            }
        }
        else if(!strcmp(argv[i],"-d"))
        {
            fDaemon = true;
        }
    }
    return 1;
}

int CParser::FillMainParams (void)
{
    if (sScommIp) 
    {
        printf("Scomm ip address:%s\n", sScommIp);
    }
    else
    {
        printf("No scomm ip address!\n");
        PrintHelp();
        return -1;
    }

    if (sOutDir) 
    {
        printf("Output directory:%s\n", sOutDir);
    }
    else
    {
        printf("No output directory!\n");
        PrintHelp();
        return -1;
    }

    if (!sFileNameBase)
    {
        sFileNameBase = (char*)malloc(strlen("comlog")+1);
        if (!sFileNameBase)
        {
            printf("Out of memory!\n");
            return -1;
        }
        strcpy(sFileNameBase, "comlog");
    }
    printf("File name base:%s\n",sFileNameBase);

    if (!sPassword)
    {
        sPassword = (char*)malloc(strlen("default")+1);
        if (!sPassword)
        {
            printf("Out of memory!\n");
            return -1;
        }
        strcpy(sPassword, "default");
    }
    printf("Password:%s\n",sPassword);

    if(!ScommPort) ScommPort = 10001;
    printf("Scomm port:%d\n",ScommPort);

    if(!ServerPort) ServerPort = ScommPort + 1;
    printf("Server port:%d\n", ServerPort);

    switch(rotation)
    {
        case ROTATION_DAY:
            printf("Rotation:day\n");
        break;
        case ROTATION_DECADE:
            printf("Rotation:decade\n");
        break;
        case ROTATION_MONTH:
            printf("Rotation:month\n");
        break;
        case ROTATION_REALTIME:
            printf("Rotation:realtime\n");
        break;
        default:
        {
            printf("Rotation unknown!\n");
            PrintHelp();
            return -1;
        }
    }

    if (!sLogFile)
    {
        sLogFile = (char*)malloc(strlen(sOutDir)+strlen("/spider.log")+1);
        if (!sLogFile)
        {
            printf("Out of memory!\n");
            return -1;
        }
        strcpy(sLogFile,sOutDir);
        strcat(sLogFile,"/spider.log");
    }
    printf("Logfile:%s\n", sLogFile);

    if(nTimeUpdateMin)
    {
        printf("TimeUpdate:%u min\n", nTimeUpdateMin);
    }

    if (fDaemon) printf("Daemon mode!\n");
    return 1;
}

void CParser::PrintHelp()
{
printf("\nNAME\n\
\tspider - tarification collector from ATS M-200\n\
\nSYNOPSIS\n\
\tspider -scommip X.X.X.X -outdir dir [-scommport N] [-serverport P]\n\
\t[-filename name] [-rotation r] [-password pppppp] [-logfile path] [-d]\n\n\
\tX.X.X.X - scomm IP address. X = 0..255\n\
\tdir - tfs and txt files directory.\n\
\tN - scomm TCP port. N = 0..65535 (default N = 10001)\n\
\tP - spider server TCP port. P = 0..65535, P != N (default P = N+1)\n\
\tr - tfs and txt files rotation. r = 1..4 (default r = 1)\n\
\tname - tfs and txt filename base. (default name = comlog)\n\
\tpppppp - password for ATS binary mode. p = 0..9 (default pppppp = 100100)\n\
\tpath - path for spider logfile. (default path = dir/spider.log)\n\
\t-d - daemon mode.\n\
\nEXAMPLES\n\
\tUsing in daemon mode:\n\
[MTA@Vasia /home]./spider -scommip localhost -outdir /home/Vasia -d\n\
\tUsing with no logfile:\n\
[MTA@Vasia /home]./spider -scommip 192.168.0.1 -scommport 20000 -outdir /home/Vasia -logfile /dev/null\n\
\tUsing with nonstandart filename base:\n\
[MTA@Vasia /home]./spider -scommip localhost -outdir /home/Vasia -filename mylogs\n");
}
