


#include "md5.h"




#ifdef WIN32
#include <stdio.h>
#include <string>
#include <WinSock2.h>
#include <Windows.h>
#include <io.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <direct.h>
 #pragma comment(lib, "ws2_32.lib")
#endif

#ifdef LINUX
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>  
#include <netdb.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>  
#include <sys/types.h> 
#endif

#ifdef DVXWORK
#include <vxworks.h>
#include <stdio.h>
#include <string>
#include <sockLib.h>
#include <inetLib.h>
#include <unistd.h>
#include <stat.h>
#include <string.h>
#   include <iosLib.h>
#   include <string.h>
#   include <netDrv.h>
#   include <hrFsLib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <utime.h>
#include <sys/stat.h>
#include <hostLib.h>
#include <vector>
#include <fstream>
#endif

using namespace std;

#define HEARTPORT 6004 // 发送心跳端口
#define BOARDCASTPORT 3086 // 接收广播端口
#define COMMAND 3087 // 接收命令端口
#define SCANRETS 6005 // 发送扫描端口
#define DEPLOYPORT 3088


#define REVBUFSIZE 1024

// 心跳信息结构体
struct Stru_Heart
{
	//char sign[4];
	char IP[15]; // ���ゆ�烽���ゆ��P���ゆ�峰��
Stru_Heart()
{
	//memcpy(sign,"C101",4);
	memset(IP,0,15);
}
};

// 服务器下发的命令信息结构体
struct Stru_Command
{
	char sign[4];
	char command;
Stru_Command()
{
	memcpy(sign,"S102",4);
}
};

// 返回的扫描信息结构体
struct Stru_Scans
{
	char scanPath[256];
	char scanType[128];
	char browserID[37];
	char deviceID[37];
	char compID[37];
Stru_Scans()
{
	memset(scanPath,0,256);
	memset(scanType,0,10);
	memset(browserID,0,37);
	memset(deviceID,0,37);
	memset(compID,0,37);
}
};

// 扫描内容结构体
struct Stru_ScanRets
{
	char filePath[256];
	char md5Value[34];
Stru_ScanRets()
{
	memset(filePath,0,256);
	memset(md5Value,0,34);
}
};

// 内部维护的扫描信息结构体
struct Stru_ScanRetReco
{
    char sign[4];
    char browserID[36];
    char deviceID[36];
    char compID[36];
    int fileNum;
    vector<Stru_ScanRets> scanRets;
Stru_ScanRetReco()
{
	memcpy(&sign,"C102",4);
	memset(browserID,0,36);
	memset(deviceID,0,36);
	memset(compID,0,36);
	fileNum = 0;
	scanRets.clear();
}
};

// 定时器
void intval(int mtime)
{
#ifdef WIN32
	Sleep(mtime);
#endif
#ifdef LINUX
	sleep(mtime/1000);
#endif
}

// 发送设备心跳处理函数
void sendHeart();

// 接收服务器广播信息
void RecvServiceBroadcast();

// 接收服务器的命令信息
void recvCommand();

// 扫描文件函数
void scanFiles(Stru_Scans scans);
void scanFilesProxy(Stru_Scans scans);

// 部署函数
void deployFiles();

void exitProj();

char *UnicodeToUTF8(const char* src,int srclen,int &len); // 将UniCode转换为UTF-8编码
char *UTF8ToUnicode(const char* src,int srclen,int &len); // 将UTF-8转换为UniCode编码

bool deployFlag; // 部署标志位
bool progFlag; // 程序运行标志，true表示正常运行，false表示退出
