// NewDeployClient.cpp : 一体化运维客户端处理代码，支持windows、linux（中标）与vxworks系统
// 编写者： 高暠
// 版本号： v1.0
#include "header.h"

// 保存服务器的IP地址
char serverIP[15] = "0.0.0.0"; 
char hostIP[15] = "0.0.0.0"; // 本机IP地址

char filelogName[128]; // 本次启动软件记录日志的名称
char *LogFloder = (char*)"RGLOGS/"; // 存放日志的路径
// 记录日志函数

void writeLog(const char *buf)
{
#ifdef WRITELOG
	if(buf != NULL)
	{
		CreatDir(LogFloder);
		FILE *fp = fopen(filelogName,"a+");
		if(fp == NULL)
			return;
		string time = getTime() + "\t";
		fwrite(time.c_str(),1,time.size(),fp);
		fwrite(buf,1,strlen(buf),fp);
		fclose(fp);
	}
#endif
}


// 周期发送心跳函数
void sendHeart()
{
#ifdef WIN32
	WSADATA wsa;  
    WSAStartup(MAKEWORD(2,2),&wsa); //initial Ws2_32.dll by a process
	
#endif
	int client;
	bool flag = true;
	client = socket( AF_INET, SOCK_DGRAM, 0); 
	if ( client < 0 )
	{
		flag = false;
		writeLog("create heartsocket failed.\n");
	}
	sockaddr_in hostInfo;
	hostInfo.sin_family = AF_INET;
	//hostInfo.sin_addr.s_addr = inet_addr("193.1.103.16");// htonl(INADDR_ANY);
	hostInfo.sin_addr.s_addr =  htonl(INADDR_ANY);
	hostInfo.sin_port = htons( HEARTPORT );
	
    /*if (flag==true && bind( client, (struct sockaddr*) &hostInfo, sizeof(sockaddr_in) ) < 0 )
	{
		flag = false;
		printf( "heartsocket bind failed.\n"  );
	}*/
	Stru_Heart heartbuf;
	
	int ret = 0;
	memcpy(heartbuf.IP,&hostIP,sizeof(hostIP));
	sockaddr_in server;

#ifdef WIN32
	
#endif
#ifdef DVXWORK
	FAST DEV_HDR *pDevHdr;
#endif
	while(flag && progFlag)
	{
        if(strcmp(serverIP,"0.0.0.0") != 0) // 判断当前是否已获知服务器的IP地址
		{
			
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = inet_addr(serverIP);
			server.sin_port = htons( HEARTPORT );
			ret = sendto(client,(char*)&heartbuf,sizeof(heartbuf),0,(struct sockaddr*)&server,sizeof(struct sockaddr));
			if(ret == -1)
			{
				writeLog("send heart error.\n");
			}
			
		}
		
        intval(5000); // 定时
	}
#ifdef WIN32
	closesocket(client);
	WSACleanup();
#endif
#ifdef LINUX
    close(client);
#endif
#ifdef DVXWORK
	close(client);
#endif
}

// 将UTF字符集转换为字符串
#ifdef WIN32
char* UTF8ToUniCode(const char* utf8,int srclen,int &len)
{
	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, srclen, NULL, 0);
	wchar_t* wstr = new wchar_t[wlen+1];
	memset(wstr, 0, wlen+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, srclen, wstr, wlen);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, wlen, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_ACP, 0, wstr, wlen, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	return str;
}

// 将字符串src转换为UTF字符集，返回，其中srclen为src的长度，len为返回的长度
char* UnicodeToUTF8(const char* src,int srclen,int &len)  
{  
    // convert an widechar string to utf8  
    int wlen = MultiByteToWideChar(CP_ACP, 0, src,srclen, NULL, 0); //计算src转换为宽字符串的长度
	wchar_t* wstr = new wchar_t[wlen+1];
	memset(wstr, 0, (wlen+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, src, srclen, wstr, wlen); // 将src转换为宽字符串放在wstr中
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, NULL, 0, NULL, NULL); // 计算宽字符串转换utf的长度
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, str, len, NULL, NULL); // 将宽字符串转换为utf字符
	if(wstr) delete[] wstr;
	return str; 
} 
#endif

// 接收服务器发送的广播信息，获得服务器IP地址
void RecvServiceBroadcast()
{
	int sockfd;  
    struct sockaddr_in saddr;  
    int r;  
    char recvline[20];  
    struct sockaddr_in presaddr;  
	
#ifdef WIN32
	WSADATA wsa;  
    WSAStartup(MAKEWORD(2,2),&wsa); //initial Ws2_32.dll by a process
#endif
	bool flag = true;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
    if ( sockfd < 0 )
	{
		writeLog("create serviceBroadcast failed.\n");
		flag = false;
	}
    // 接收广播信息
	/*bool bOpt = true;
    if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt)) < 0)
	{
		printf( "set serviceBroadcast failed %d.\n" );
		return;
	}*/

    saddr.sin_family = AF_INET;  
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    saddr.sin_port = htons(BOARDCASTPORT);  


    if(flag==true && bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
	{
		writeLog("bind serviceBroadcast failed.\n");
		flag = false;
	}
#ifdef WIN32
	int len = sizeof(sockaddr_in);
#endif
#ifdef LINUX
    socklen_t len = sizeof(sockaddr_in);
#endif
#ifdef DVXWORK
    int len = sizeof(sockaddr_in);
#endif
	char sign[5];
    while (flag&&progFlag)  
    {  
		memset(recvline,0,20);
        r = recvfrom(sockfd, recvline, sizeof(recvline), 0 , (struct sockaddr*)&presaddr, &len);  
		memset(sign,0,5);
        // 获得报文标识
		memcpy(sign,recvline,4);
		if(strcmp(sign,"S101") == 0)
		{
            // 获得服务器的IP地址
			memcpy(serverIP,recvline+4,r-4); 
		}
       
    }  
#ifdef WIN32
	closesocket(sockfd);
	WSACleanup();
#endif
#ifdef LINUX
    close(sockfd);
#endif
#ifdef DVXWORK
	close(sockfd);
#endif
}

// 接收服务器发送的命令信息，包括扫描，部署指令
void recvCommand()
{
	int sockfd;  
    struct sockaddr_in saddr;  
    int r;  
    char recvline[382];
    struct sockaddr_in presaddr;  
	
#ifdef WIN32
	WSADATA wsa;  
    WSAStartup(MAKEWORD(2,2),&wsa); //initial Ws2_32.dll by a process
#endif
	bool flag = true;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
    if ( sockfd < 0 )
	{
		writeLog("create serviceBroadcast failed.\n");
		flag = false;
	}
	
    saddr.sin_family = AF_INET;  
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);  
	//saddr.sin_addr.s_addr = inet_addr("193.1.103.16");
    saddr.sin_port = htons(COMMAND);  


    if(flag==true && bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
	{
		writeLog("bind serviceBroadcast failed.\n");
		flag = false;
	}
#ifdef WIN32
	int len = sizeof(sockaddr_in);
#endif
#ifdef LINUX
    socklen_t len = sizeof(sockaddr_in);
#endif
#ifdef DVXWORK
    int len = sizeof(sockaddr_in);
#endif
	char sign[5];
    while (flag&&progFlag)  
    {  
		memset(recvline,0,sizeof(recvline));
        r = recvfrom(sockfd, recvline, sizeof(recvline), 0 , (struct sockaddr*)&presaddr, &len);  
		memset(sign,0,5);
        // 获得报文标识
		memcpy(sign,recvline,4);
        if(strcmp(sign,"S102") == 0) // 表示全盘扫描
		{
			Stru_Scans scans;
			int index = 4;
			memcpy(scans.browserID,recvline+index,sizeof(scans.browserID));
            // 获得申请ID，该信息是在返回扫描结果时有效，对本软件无意义
			index += sizeof(scans.browserID);
			memcpy(scans.deviceID,recvline+index,sizeof(scans.deviceID));
            // 获得设备ID，该信息是在返回扫描结果时有效，对本软件无意义
			index += sizeof(scans.deviceID);
			memcpy(scans.compID,recvline+index,sizeof(scans.compID));
			index += sizeof(scans.compID);
            // 获得扫描路径
			memcpy(scans.scanPath,recvline+index,sizeof(scans.scanPath));
			scanFilesProxy(scans);
		}
			
			/*else if(recvcommand == 3) // 锟斤拷始锟斤拷锟斤拷
			{
				deployFlag = true;
			}
			else if(recvcommand == 4) // 锟斤拷停锟斤拷锟斤拷
			{
				deployFlag = false;
			}
			else if(recvcommand == 5) // 锟斤拷锟斤拷锟斤拷锟斤拷乇锟斤拷锟斤拷
			{
				deployFlag = false;
				progFlag = false;
			}*/
        else if(strcmp(sign,"S103") == 0) // 表示根据指定类型扫描
		{
			Stru_Scans scans;
			int index = 4;
			memcpy(scans.browserID,recvline+index,sizeof(scans.browserID));
            // 获得申请ID，该信息是在返回扫描结果时有效，对本软件无意义
			index += sizeof(scans.browserID);
			memcpy(scans.deviceID,recvline+index,sizeof(scans.deviceID));
            // 获得设备ID，该信息是在返回扫描结果时有效，对本软件无意义
			index += sizeof(scans.deviceID);
			memcpy(scans.compID,recvline+index,sizeof(scans.compID));
			index += sizeof(scans.compID);
			memcpy(scans.scanType,recvline+index,sizeof(scans.scanType));
			index += sizeof(scans.scanType);
			memcpy(scans.scanPath,recvline+index,sizeof(scans.scanPath));
			scanFilesProxy(scans);
			
		}
       
    } 
#ifdef WIN32
	closesocket(sockfd);
	WSACleanup(); 
#endif
#ifdef DVXWORK
	close(sockfd);
#endif
}


// source内容以,分隔，查找innerbuf是否在buffer内容中，如果是返回对应的位置，否则返回-1值
int findIndex(char *source,char *innerbuf)
{
	string buffer(source);
	// 首先按照,对buffer进行分隔，放到链表当中
	vector<string> sourcelist;
	size_t pos = 0, found = 0;
	while ( found != string::npos )
	{
		found = buffer.find(",", pos);
		sourcelist.push_back(string(buffer, pos, found - pos));
		pos = found + 1;
	}
	// 分析innerbuf是否在source中
	for(int i=0;i<(int)sourcelist.size();i++)
	{
		if(strcmp(sourcelist[i].c_str(),innerbuf) == 0)
		{
			// 找到对应的内容
			return i+1;
		}
	}
	return -1;
}
// 判断文件filename是否为type类型 多个后缀类型以,分隔
bool decFileName(char *fileName,char *fileType)
{
	if(fileName == 0 || fileType == 0)
		return false;
	int len = strlen(fileName);
	for(int i=0;i<len;i++)
	{
        if(fileName[i] == '.'&&i!=(len-1))
		{
			char type[40];
			memcpy(&type,&fileName[i+1],len-i+1);
			if(findIndex(fileType,type) > 0)
				return true;
		}
	}
	return false;
}

// 通过tcp返回扫描结果
void sendScanRets(Stru_ScanRetReco retReco)
{
	#ifdef WIN32
	WSADATA wsa;  
    WSAStartup(MAKEWORD(2,2),&wsa); //initial Ws2_32.dll by a process
	
#endif
	int client;
	bool flag = true;
	client = socket( AF_INET, SOCK_STREAM, 0); 
	if ( client < 0 )
	{
		flag = false;
		writeLog("create scan failed.\n");
	}
	
	/*
	int buf;
	int lens = sizeof(buf);
	getsockopt(client,SOL_SOCKET, SO_SNDBUF, (char*)&buf, &lens);
	lens = 10000;
	if(setsockopt(client, SOL_SOCKET, SO_SNDBUF, (char*)&lens, sizeof(lens)) < 0)
    {
        printf("sending scan size configuration fail!\n");
		flag = false;
     }
	 */

	sockaddr_in server;
	if(flag&&progFlag)
	{
		if(strcmp(serverIP,"0.0.0.0") != 0)
		{
            server.sin_family = AF_INET; // TCP/IP
            server.sin_addr.s_addr = inet_addr(serverIP);;
            server.sin_port = htons(SCANRETS);
            if(connect(client,(struct sockaddr*)&server,sizeof(struct sockaddr)) < 0)
			{
				writeLog("scan connect failed\n");
			}
			else
			{
				int len = sizeof(retReco) + retReco.fileNum * (256 + 34);
				char *sendbuf = new char[len];
				memset(sendbuf,0,len);
				int index = 0;
				memcpy(sendbuf,retReco.sign,sizeof(retReco.sign));
				index += sizeof(retReco.sign);
				memcpy(sendbuf+index,retReco.browserID,sizeof(retReco.browserID));
                index += sizeof(retReco.browserID); // 申请ID号
				memcpy(sendbuf+index,retReco.deviceID,sizeof(retReco.deviceID));
                index += sizeof(retReco.deviceID); // 设备ID号
				memcpy(sendbuf+index,retReco.compID,sizeof(retReco.compID));
                index += sizeof(retReco.compID); // 组件ID号
				//memcpy(sendbuf + index,&retReco.fileNum,sizeof(retReco.fileNum));
				//index += sizeof(retReco.fileNum);
                // 遍历扫描内容，按照 文件名称、md5放于发送队列
				for(int i=0;i< retReco.fileNum;i++)
				{
					memcpy(sendbuf+index,retReco.scanRets[i].filePath,sizeof(retReco.scanRets[i].filePath));
					index += sizeof(retReco.scanRets[i].filePath);
					memcpy(sendbuf+index,retReco.scanRets[i].md5Value,sizeof(retReco.scanRets[i].md5Value));
					index += sizeof(retReco.scanRets[i].md5Value);
				}
				int ret = send(client,(const char*)sendbuf,len,0);
				if(ret < 0)
				{
                    // ret = WSAGetLastError();
					writeLog("send scanRet failed.\n");
				}
				
				delete sendbuf;
			}

		}
	}
#ifdef WIN32
	closesocket(client);
	WSACleanup();
#endif
#ifdef LINUX
	close(client);
#endif
#ifdef DVXWORK
	close(client);
#endif
}

// 根据scanPath路径，通过递归扫描类型为scanType的文件，并放于fileList。
void scanFiles(char *scanPath,char *scanType,vector<Stru_ScanRets> &fileList)
{
#ifdef WIN32
    char dirNew[300];
	memset(dirNew,0,300);
	strcpy(dirNew, scanPath);	
    strcat(dirNew, "/*.*");    // 获得该路径下所有的文件
	
    _finddata_t findData;
	intptr_t handle;
    handle = _findfirst(dirNew, &findData);
    if (handle == -1)
    {
		char buffer[40];
		sprintf(buffer,"open dir %s failed\n",dirNew);
		writeLog(buffer);
    	return;
    }
        

    do
    {
        if (findData.attrib & _A_SUBDIR) // 判断该文件是否为文件夹
        {
            if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
                continue;

            // cout << findData.name << "\t<dir>\n";
            // 组成新的文件路径，进行扫描
			strcpy(dirNew, scanPath);
			strcat(dirNew, "/");
			strcat(dirNew, findData.name);

			scanFiles(dirNew,scanType,fileList);
        }
        else
		{
            // 当前为文件
			if((strcmp(scanType,"")==0) || decFileName(findData.name,scanType) == true)
			{
				Stru_ScanRets scanRets;
				char utfPath[300];
				memset(utfPath,0,300);
				strcat(utfPath,scanPath);
				strcat(utfPath,"/");
				strcat(utfPath,findData.name);
				int utflens = 0;
				char *temppath = (char*)UnicodeToUTF8(utfPath,strlen(utfPath),utflens);
				memcpy(scanRets.filePath , temppath,utflens);

                // 计算该文件的MD5值
				MD5_CTX mdvalue;
				mdvalue.GetFileMd5( scanRets.md5Value,  utfPath);
                // 将扫描结果放入队列中
				fileList.push_back(scanRets);
			}
			
		}
    } while (_findnext(handle, &findData) == 0);

    _findclose(handle);
#endif
#ifdef LINUX
	char dirNew[300];
    memset(dirNew, 0, 300);
	strcpy(dirNew, scanPath);
	DIR *pDir;
	pDir = opendir (dirNew);		/* Attempt to open directory. */
	if(pDir == NULL)
	{
		char *buffer = new char[40];
		sprintf(buffer,"open dir %s failed\n",dirNew);
		writeLog((const char*)buffer);
		delete buffer;
		return;
	}
	struct dirent *	pDirEnt = NULL;	

	while((pDirEnt = readdir (pDir))!=NULL)
	{
		if ((strcmp (pDirEnt->d_name, ".") == 0)  || (strcmp (pDirEnt->d_name, "..") == 0))
		    continue;
		struct stat fileStat;
		lstat(pDirEnt->d_name, &fileStat);

		//setbuf(stdout,NULL);
		//	printf("%s\n",pDirEnt->d_name);
		//fflush(stdout);

        if (S_ISDIR (fileStat.st_mode)) // 判断是否为文件夹
		{
			strcpy(dirNew, scanPath);
			strcat(dirNew, "/");
			strcat(dirNew, pDirEnt->d_name);
			scanFiles(dirNew, scanType, fileList);
		}
		else
		{
            // 当前为文件
			// cout << findData.name << "\t" << findData.size << " bytes.\n";
			if ((strcmp(scanType, "") == 0) || decFileName(pDirEnt->d_name,
					scanType) == true)
			{
				Stru_ScanRets scanRets;
				strcat(scanRets.filePath, scanPath); // 锟斤拷锟铰凤拷锟?
				strcat(scanRets.filePath, "/");
				strcat(scanRets.filePath, pDirEnt->d_name);
                // 计算该文件的MD5值
				MD5_CTX mdvalue;
				mdvalue.GetFileMd5(scanRets.md5Value, scanRets.filePath);
                // 将扫描结果放入队列中
				fileList.push_back(scanRets);
			}
		}
	}
	closedir(pDir);
#endif
#ifdef DVXWOKR
	char dirNew[300];
	    memset(dirNew, 0, 300);
		strcpy(dirNew, scanPath);
		DIR *pDir;
		pDir = opendir (dirNew);		/* Attempt to open directory. */
		if(pDir == NULL)
		{
			char *buffer = new char[40];
			sprintf(buffer,"open dir %s failed\n",dirNew);
			writeLog((const char*)buffer);
			delete buffer;
			return;
		}
		struct dirent *	pDirEnt = NULL;

		while((pDirEnt = readdir (pDir))!=NULL)
		{
			if ((strcmp (pDirEnt->d_name, ".") == 0)  || (strcmp (pDirEnt->d_name, "..") == 0))
			    continue;
			struct stat fileStat;
			lstat(pDirEnt->d_name, &fileStat);

			//setbuf(stdout,NULL);
			//	printf("%s\n",pDirEnt->d_name);
			//fflush(stdout);

			if (S_ISDIR (fileStat.st_mode))
			{
				strcpy(dirNew, scanPath);
				strcat(dirNew, "/");
				strcat(dirNew, pDirEnt->d_name);
				scanFiles(dirNew, scanType, fileList);
			}
			else
			{
				// 锟斤拷锟斤拷锟斤拷锟侥硷拷
				// cout << findData.name << "\t" << findData.size << " bytes.\n";
				if ((strcmp(scanType, "") == 0) || decFileName(pDirEnt->d_name,
						scanType) == true)
				{
					Stru_ScanRets scanRets;
					strcat(scanRets.filePath, scanPath); // 锟斤拷锟铰凤拷锟?
					strcat(scanRets.filePath, "/");
					strcat(scanRets.filePath, pDirEnt->d_name);
					// 锟斤拷锟斤拷募锟斤拷锟�d5值
					MD5_CTX mdvalue;
					mdvalue.GetFileMd5(scanRets.md5Value, scanRets.filePath);
					// 锟斤拷扫锟斤拷锟斤拷息锟斤拷锟诫到锟斤拷锟斤拷锟斤拷
					fileList.push_back(scanRets);
				}
			}
		}
		closedir(pDir);
	
#endif
}

#ifdef WIN32
DWORD WINAPI winScanProc(LPVOID para);
#endif
#ifdef LINUX
void *linuxScanProc(void *argc);
#endif
#ifdef DVXWORK
void vxScanProc(void *argc);
#endif
void scanFilesProxy(Stru_Scans scans)
{
	Stru_Scans *scanbuf = new Stru_Scans();
	memcpy(scanbuf,&scans,sizeof(Stru_Scans));
#ifdef WIN32
	HANDLE scanThread=NULL;
	DWORD scanThreadID=0;//锟斤拷锟斤拷锟竭筹拷ID
    scanThread=CreateThread(0,0,winScanProc,(LPVOID)scanbuf,0,&scanThreadID);//锟斤拷锟斤拷锟竭筹拷
#endif
#ifdef LINUX
	pthread_t scanThreadID;
	pthread_create(&scanThreadID,NULL,linuxScanProc,(void*)scanbuf);
#endif
#ifdef DVXWORK
	taskSpawn("scanThread",160,0,6000,(FUNCPTR)vxScanProc,scanbuf,0,0,0,0,0,0,0,0,0);
#endif
}

// 获得当前文件的文件路径 返回值  返回参数中文件的路径  其中filepath不含最后的\\信息
bool find_last_of(char* absfile,char *filepath,int &len)
{
	int findindex = -1;
		for(int i=0;i<(int)strlen(absfile);i++)
		{
            if(absfile[i] == '\\'|| absfile[i] == '/') // 找到\\字符串了
			{
				findindex = i;
				continue;
			}
		}
		if(findindex == -1)
			return false;
		else
		{
			if(filepath == NULL)
			{
				filepath = new char[strlen(absfile)];
				memset(filepath,0,strlen(absfile));
			}
			memcpy(filepath,absfile,findindex);
			
			return true;
		}
		
		return false;	
	
}

int my_mkdir(char *pszDir)
{
#ifdef WIN32
    return mkdir(pszDir);
#endif
#ifdef LINUX
    return mkdir(pszDir,S_IRUSR|S_IWUSR|S_IXUSR | S_IRWXG | S_IRWXO);
#endif
#ifdef DVXWORK
    return mkdir(pszDir);
#endif
}

// 创建文件夹
int CreatDir(char *pDir)
{
	int i = 0;
		int iRet;
		int iLen;
		char* pszDir;

		if(NULL == pDir)
		{
			return 0;
		}
		
		//pszDir = strdup(pDir);
		pszDir = new char[strlen(pDir)+1];
		memset(pszDir,0,strlen(pDir)+1);
		memcpy(pszDir,pDir,strlen(pDir));
		iLen = strlen(pszDir);

        // 遍历依次查找每一级的目录文件
		for (i = 0;i < iLen;i ++)
		{
            if ((pszDir[i] == '\\' || pszDir[i] == '/')&&i!=0)
			{ 
				pszDir[i] = '\0';

                // 判断该目录是存在
				iRet = access(pszDir,0);
				if (iRet != 0)
				{
					//printf("the %s is not exist\n",pszDir);
                    iRet = my_mkdir(pszDir);
					if (iRet != 0)
					{
						//printf("Create %s file is failed\n",pszDir);
						return -1;
					} 
				}
				else
				{
					//printf("the %s is exist\n",pszDir);
				}
				pszDir[i] = '/';
			} 
		}

        iRet = my_mkdir(pszDir);
		free(pszDir);
		return iRet;
}

void switchToLocalSepartorPath(char * pDir)
{
    int length = strlen(pDir);
    for(int i = 0; i < length; i++)
    {
       if(pDir[i] == '\\')
       {
            pDir[i] = '/';
       }
    }
}

// 接收服务器的部署文件消息并生成本地文件
void DeployFiles()
{
	sockaddr_in addrSvr;
#ifdef WIN32
    int len = sizeof(sockaddr_in);
#endif
#ifdef LINUX
    socklen_t len = sizeof(sockaddr_in);
#endif
#ifdef DVXWORK
    int len = sizeof(sockaddr_in);
#endif
	int server;
	bool flag = true;
	server = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if ( server < 0 )
	{
		writeLog( "deploy create socket failed.\n" );
		flag = false;
	}	

    sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = htonl(INADDR_ANY);
	service.sin_port = htons( DEPLOYPORT );

	/* 设置关闭时缓冲处理方式
	struct linger opt;
	opt.l_onoff=1;
	opt.l_linger = 0;
	if(flag==true && setsockopt(server, SOL_SOCKET, SO_LINGER, (char *)&opt, sizeof(linger)) < 0)
	{
		printf( "deploy setLINGER failed.\n" );
		flag = false;
	}
	*/

	int tcp_nodelay = 1;
	
	if (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE,(char* ) &tcp_nodelay, sizeof(int)) < 0)
	{
		writeLog("deploy set alive failed.\n");
	}
	if (setsockopt(server, IPPROTO_TCP, TCP_NODELAY,(char* ) &tcp_nodelay, sizeof(int)) < 0)
	{
		writeLog("deploy set delay failed.\n");
	}
	

	if (flag==true&& bind( server, (sockaddr *) &service, sizeof(sockaddr_in) ) < 0 ) {
		writeLog( "deploy bind failed.\n" );
		flag =false;
		intval(100);
	}

    // 监听 socket
	int ls = listen( server, 5 );
	if(ls == -1)
	{
		writeLog("deploy listen failed.\n");
		flag = false;
	}

	int AcceptSocket;
	sockaddr_in client;
    // 开始接收文件
	while (flag && progFlag) 
	{
#ifdef WIN32
    int addrsize=sizeof(client);
#endif
#ifdef LINUX
    socklen_t addrsize = sizeof(client);
#endif
#ifdef DVXWORK
    int addrsize=sizeof(client);
#endif

		AcceptSocket = accept( server, (sockaddr*) &client, &addrsize );
		if (AcceptSocket > 10000)
		{
			continue;
		}
		// server = AcceptSocket; 

        // 接受文件传输数据
		int RecvLen;
		char recvbuf[REVBUFSIZE] = ""; // 用于将接收的转换UTF8格式
		while(progFlag)
		{ 
            if(deployFlag == false) // 暂停本次文件传输
			{
				break;
			}
			// 判断是否开始接收文件 收到DeployStart表示开始接收，DeployEnd
			memset( recvbuf, 0, sizeof(recvbuf) );
			RecvLen = recvfrom(AcceptSocket, recvbuf, 13, 0,(sockaddr*)&addrSvr, &len); // 其中长度设置为13是解决TCP连包的问题，这里控制只能接收标志信息
            if ( strcmp(recvbuf, "DeployEnd") == 0 ) // 表示本次部署已结束
			{
				// writeLog("Deploy End\n");
				break;
			}
            // 先接受文件的路径和名称 考虑到编码格式，需要从UTF8转换为多字节
			memset( recvbuf, 0, sizeof(recvbuf) );
			char recvbuf1[REVBUFSIZE] = ""; // 用于接收缓冲
			int recvLenTemp = recvfrom( AcceptSocket, recvbuf1, 260, 0,(sockaddr*)&addrSvr, &len ); // 其中长度设置为260是解决TCP连包的问题
			if(recvLenTemp <=0 || recvbuf1 == NULL)
			{
				break;
			}

			// 转换为UTF-8格式
#ifdef WIN32
			char *convertBuf = UTF8ToUniCode(recvbuf1,recvLenTemp,RecvLen);
			memcpy(recvbuf,convertBuf,RecvLen);
			delete convertBuf;
#endif
#ifdef LINUX
			memcpy(recvbuf,recvbuf1,recvLenTemp);
			RecvLen = recvLenTemp;
#endif
#ifdef DVXWORK
			memcpy(recvbuf,recvbuf1,recvLenTemp);
			RecvLen = recvLenTemp;
#endif

			recvbuf[RecvLen] = '\0';

			switchToLocalSepartorPath(recvbuf);
			if ( RecvLen < 0 )
			{
				break;
			}
						
            // 接收到文件后，判断文件夹是否存在，若不存在则创建文件夹
			//printf("the recv %d buffer \n",bytesRecv);
			char *filepath = new char[RecvLen];
			int filepathlen = 0;
			memset(filepath,0,RecvLen);
			// printf("%s\n",recvbuf);
			if(find_last_of(recvbuf,filepath,filepathlen) == true)
			{
                // 判断文件夹是否存在，如果不存在泽创建
				int iRet = access(filepath,0);  	
				if (iRet != 0)  
				{  
					iRet = CreatDir(filepath);  
					if (iRet != 0)  
					{  
						//printf( "Create filepath %s is failed.Recv is possiable failed\n" ,filepath);
						continue;
					}   
				}  
			}
			else
			{
				continue;
			}
			//delete filepath;
			//printf("start to deploy file\n");
			std::ofstream ofs;
            // 打开文件读写
			ofs.open(recvbuf,ios::binary);
            // 开始接收文件
			while(progFlag)
			{ 
				memset(recvbuf,0,sizeof(recvbuf));
                RecvLen = recvfrom(AcceptSocket, recvbuf, REVBUFSIZE, 0,(struct sockaddr*)&addrSvr, &len);
				if (RecvLen < 0)
				{
					break;
				}

				if (strcmp(recvbuf, "fileRecvEnd")==0 )
				{
					break;
				}
				ofs.write(recvbuf,RecvLen);
			}

			ofs.close();
			ofs.clear();
			// 回复收到命令
			char recvEndFlag[16];
			memset(recvEndFlag,0,16);
			strcpy(recvEndFlag,"f");
			send(AcceptSocket,(const char*)recvEndFlag,strlen(recvEndFlag),0);
            
		}
		intval(100);
	}
#ifdef WIN32
	closesocket(server);
	WSACleanup(); 
#endif
#ifdef LINUX
    close(server);
#endif
#ifdef DVXWORK
	close(server);
#endif
}

// 以下为windows线程函数
#ifdef WIN32
DWORD WINAPI winSendHeartProc(LPVOID para)
{
	sendHeart();
	return 0;
}

DWORD WINAPI winBroadCastProc(LPVOID para)
{
	RecvServiceBroadcast();
	return 0;
}

DWORD WINAPI winDeployProc(LPVOID para)
{
	DeployFiles();
	return 0;
}

DWORD WINAPI recvComdProc(LPVOID para)
{
	recvCommand();
	return 0;
}

// window线程，负责扫描本地文件并发送扫描结果
DWORD WINAPI winScanProc(LPVOID para)
{
	Stru_Scans* scans = (Stru_Scans*)para;
	if(scans == NULL)
	{
		return 0;
	}
	Stru_ScanRetReco scanRetReco;
    // 开始本地文件扫描
	scanFiles(scans->scanPath,scans->scanType,scanRetReco.scanRets);
	memcpy(scanRetReco.browserID,scans->browserID,sizeof(scans->browserID));
	memcpy(scanRetReco.deviceID,scans->deviceID,sizeof(scans->deviceID));
	memcpy(scanRetReco.compID,scans->compID,sizeof(scans->compID));
	delete scans;
    // 发送扫描的结果
	scanRetReco.fileNum = scanRetReco.scanRets.size();
	sendScanRets(scanRetReco);
	scanRetReco.scanRets.clear();
	return 0;
}
#endif

// 以下为linux线程函数
#ifdef LINUX
void *linuxSendHeartProc(void *argc)
{
	sendHeart();
	pthread_exit(NULL);
	return 0;
}

void *linuxBroadCastProc(void *argc)
{
	RecvServiceBroadcast();
	pthread_exit(NULL);
	return 0;
}

void *linuxDeployProc(void *argc)
{
	DeployFiles();
	pthread_exit(NULL);
	return 0;
}

void *linuxrecvComdProc(void *argc)
{
	recvCommand();
	pthread_exit(NULL);
	return 0;
}

// linux线程，负责扫描本地文件并发送扫描结果
void *linuxScanProc(void *para)
{
	Stru_Scans* scans = (Stru_Scans*)para;
	if(scans == NULL)
	{
		pthread_exit(NULL);
		return 0;
	}
	Stru_ScanRetReco scanRetReco;
    // 开始本地文件的扫描
	scanFiles(scans->scanPath,scans->scanType,scanRetReco.scanRets);
	memcpy(scanRetReco.browserID,scans->browserID,sizeof(scans->browserID));
	memcpy(scanRetReco.deviceID,scans->deviceID,sizeof(scans->deviceID));
	memcpy(scanRetReco.compID,scans->compID,sizeof(scans->compID));
	delete scans;
    // 将扫描结果发送
	scanRetReco.fileNum = scanRetReco.scanRets.size();
	sendScanRets(scanRetReco);
	scanRetReco.scanRets.clear();
	pthread_exit(NULL);
	return 0;
}
#endif

#ifdef DVXWORK
void vxSendHeartProc(void *argc)
{
	sendHeart();
	return 0;
}

void vxBroadCastProc(void *argc)
{
	RecvServiceBroadcast();
	return 0;
}

void vxDeployProc(void *argc)
{
	DeployFiles();
	return 0;
}

void vxrecvComdProc(void *argc)
{
	recvCommand();
	return 0;
}

void vxScanProc(void *para)
{
	Stru_Scans* scans = (Stru_Scans*)para;
	if(scans == NULL)
	{
		pthread_exit(NULL);
		return 0;
	}
	Stru_ScanRetReco scanRetReco;
    // 开始本地文件的扫描
	scanFiles(scans->scanPath,scans->scanType,scanRetReco.scanRets);
	memcpy(scanRetReco.browserID,scans->browserID,sizeof(scans->browserID));
	memcpy(scanRetReco.deviceID,scans->deviceID,sizeof(scans->deviceID));
	memcpy(scanRetReco.compID,scans->compID,sizeof(scans->compID));
	delete scans;
    // 将扫描结果发送
	scanRetReco.fileNum = scanRetReco.scanRets.size();
	sendScanRets(scanRetReco);
	scanRetReco.scanRets.clear();
	return 0;
}
#endif
void exitProj()
{
	progFlag = false;
}

// 返回系统时间的名称
string getTime()
{
     time_t timep;
     time (&timep);
     char tmp[64];
     strftime(tmp, sizeof(tmp), "%Y-%m-%d %H-%M-%S",localtime(&timep) );
     return tmp;
}

int main(int argc, char* argv[])
{
#ifdef WIN32
	WSADATA wsa;  
    WSAStartup(MAKEWORD(2,2),&wsa); //initial Ws2_32.dll by a process
	
#endif
	// 记录日志的名称
	string logName = string(LogFloder) + "log-" + getTime() + ".txt";
	strcpy(filelogName,logName.c_str());
	progFlag = true;
	deployFlag = true; 
	char hostName[255];
	gethostname(hostName,sizeof(hostName));
	struct hostent *p=gethostbyname(hostName); 
	if(p->h_addr_list[0] != 0)
	{
		sockaddr_in hostIPs; 
		memset(&hostIPs,0,sizeof(hostIPs));
		memcpy(&hostIPs.sin_addr.s_addr,p->h_addr_list[0],p->h_length);
		memcpy(&hostIP,&hostIPs.sin_addr,sizeof(hostIPs.sin_addr));
	}
#ifdef WIN32
	HANDLE heartThread=NULL,broadcastThread = NULL,deployThread=NULL,recvComdThread;
    DWORD heartdwThreadID=0,broadcastThreadID=0,deployThreadID=0,comdID=0;
    heartThread=CreateThread(0,0,winSendHeartProc,NULL,0,&heartdwThreadID); // 启动发送心跳线程
    broadcastThread=CreateThread(0,0,winBroadCastProc,NULL,0,&broadcastThreadID);// 启动接收广播线程
    deployThread=CreateThread(0,0,winDeployProc,NULL,0,&deployThreadID); // 启动接收文件部署线程
    recvComdThread=CreateThread(0,0,recvComdProc,NULL,0,&comdID);// 启动接收命令线程
	//Stru_Scans scans;
	//strcpy(scans.scanPath,"D:\\test\\test1");
	//scanFilesProxy(scans);
    WaitForSingleObject(heartThread,INFINITE); // 等待线程结束
#endif
#ifdef LINUX
	int ret;
	pthread_t heartdwThreadID,broadcastThreadID,deployThreadID,comdID;
    ret = pthread_create(&broadcastThreadID,NULL,linuxBroadCastProc,NULL); // 启动发送心跳线程
    ret = pthread_create(&heartdwThreadID,NULL,linuxSendHeartProc,NULL); // 启动接收广播线程
    ret = pthread_create(&deployThreadID,NULL,linuxDeployProc,NULL); // 启动接收文件部署线程
    ret = pthread_create(&comdID,NULL,linuxrecvComdProc,NULL); // 启动接收命令线程

	if(heartdwThreadID != 0)
	{
        pthread_join(heartdwThreadID,NULL); // 等待线程结束
	}
	/*if(broadcastThreadID != 0)
	{
		pthread_join(broadcastThreadID,NULL);
	}
	if(deployThreadID != 0)
	{
		pthread_join(deployThreadID,NULL);
	}
	if(comdID !=0)
	{
		pthread_join(deployThreadID,NULL);
	}*/
#endif
#ifdef DVXWORK
	taskSpawn("broadcastThread",130,0,2000,(FUNCPTR)vxBroadCastProc,0,0,0,0,0,0,0,0,0,0); // 启动发送心跳线程
	taskSpawn("heartdwThread",160,0,2000,(FUNCPTR)vxSendHeartProc,0,0,0,0,0,0,0,0,0,0); // 启动接收广播线程
	taskSpawn("deployThread",120,0,8000,(FUNCPTR)vxDeployProc,0,0,0,0,0,0,0,0,0,0); // 启动接收文件部署线程
	taskSpawn("recvComd",110,0,2000,(FUNCPTR)vxrecvComdProc,0,0,0,0,0,0,0,0,0,0); // 启动接收命令线程
#endif
	return 0;
}

