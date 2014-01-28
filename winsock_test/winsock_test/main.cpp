#include "winsock2.h"
#include "stdio.h"
#include "string.h"
#include <windows.h>
#include <tchar.h>
#include <time.h>
#include <stdlib.h>


#define RECEVESIZE 1000
WSADATA wsaData;
char recve[RECEVESIZE] = "";
char cacheroot[] = "C:\\Users\\b1010162\\Desktop\\img\\";
char root_ini[] = "C:\\Users\\b1010162\\Desktop\\demo\\cache.ini";
char *newname;

typedef unsigned __int64 __uint64;

// DWORD2つからQWORDを作る
inline __uint64 MakeQWord( DWORD hi, DWORD low)
{
	return ((__uint64)hi << 32) | low;
}

// WIN32_FIND_DATAの情報から、ファイルなのかディレクトリなのかを識別する
// GetDirSize関数で使うだけ
bool IsDirectory( WIN32_FIND_DATA *FindData)
{
	return FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

// フォルダのサイズを再帰的に調べる。
// アクセス権の関係などで、1つでも失敗したら中止して戻ってくる。
// 戻り値 成功 0 / 失敗 0以外
int GetDirSize( LPCTSTR path, __uint64 *lpSize)
{
	int res = 1;
	*lpSize = 0;
	TCHAR DirSpec[MAX_PATH+1];
	//	_stprintf_p( DirSpec, MAX_PATH+1, _T("%s\\*"), path);
	_stprintf( DirSpec, _T("%s\\*"), path);	// BCC用

	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile( DirSpec, &FindData);
	if(hFind == INVALID_HANDLE_VALUE)
		goto Error;
	do {
		__uint64 size = 0;
		if(IsDirectory( &FindData)) {
			// ディレクトリ。「.」「..」は無視して、再帰。
			if(!_tcscmp( _T("."), FindData.cFileName) || !_tcscmp( _T(".."), FindData.cFileName))
				continue;
			TCHAR buf[MAX_PATH+1];
			//			_stprintf_p( buf, MAX_PATH+1, _T("%s%s\\"), path, FindData.cFileName);
			_stprintf( buf, _T("%s%s\\"), path, FindData.cFileName);	// BCC用
			if(GetDirSize( buf, &size))
				goto Error;
			*lpSize += size;
		}
		else {
			// ファイル
			*lpSize += MakeQWord( FindData.nFileSizeHigh, FindData.nFileSizeLow);
		}

		// デバッグ用
		_tprintf( _T("%s, %llu\n"), FindData.cFileName, MakeQWord( FindData.nFileSizeHigh, FindData.nFileSizeLow));

	}
	while(FindNextFile(hFind, &FindData));
	if(GetLastError() != ERROR_NO_MORE_FILES)
		goto Error;

	// 成功
	res = 0;
Error:
	FindClose(hFind);
	return res;
}

struct STRING{
	char name1[256];
};

struct RECORD{
	char filename[256];
	char url[256];
	char modi[256];
	long size;
};

struct STRING getadd(char dnsname[]){
	LPHOSTENT host;
	struct STRING result = {""};
	WSAStartup(2 , &wsaData);
	host = gethostbyname(dnsname);

	if (host == NULL) {
		printf("%s\n", dnsname);
		strcat(result.name1, dnsname);
		return result;
	}

	char dot[] = ".";
	int ip_add1 = (BYTE)*((host->h_addr_list[0]));
	int ip_add2 = (BYTE)*((host->h_addr_list[0]) + 1);
	int ip_add3 = (BYTE)*((host->h_addr_list[0]) + 2);
	int ip_add4 = (BYTE)*((host->h_addr_list[0]) + 3);

	char add1[8];
	char add2[8];
	char add3[8];
	char add4[8];

	sprintf(add1, "%d", ip_add1);
	sprintf(add2, "%d", ip_add2);
	sprintf(add3, "%d", ip_add3);
	sprintf(add4, "%d", ip_add4);

	sprintf(result.name1, "%s.%s.%s.%s", add1, add2, add3, add4);

	WSACleanup();
	return result;
}

int get_size(char add[], char req[]){
	int size = 0;
	SOCKET sock1;
	struct sockaddr_in server1;

	sock1 = socket(AF_INET, SOCK_STREAM, 0);
	//socket作成のエラー処理
	if(sock1 == INVALID_SOCKET){
		printf("socket: %d\n",WSAGetLastError());
		return -1;
	}

	//接続先指定用構造体の初期値
	server1.sin_family = AF_INET;
	server1.sin_port = htons(80);
	server1.sin_addr.S_un.S_addr = inet_addr(add);

	if (connect(sock1, (struct sockaddr *)&server1, sizeof(server1)) != 0){
		printf("failed\n");
	}

	//HEADリクエスト送信
	int n = send(sock1, req, (int)strlen(req), 0);
	if(n < 0){
		printf("send : %d\n", WSAGetLastError());
		return 1;
	}
	while(1){
		n = recv(sock1, recve, sizeof(recve), 0);
		if(n == SOCKET_ERROR){
			printf("recv : %d\n",WSAGetLastError());
			break;
		}else if(n == 0){
			break;
		}
		size += n;
	}
	closesocket(sock1);
	return size;
}

struct STRING getdns(char url[]){
	struct STRING dns = {""};
	char *ptr1 = url;
	char *ptr2;

	if(strstr(url, "//")){
		//"//"の位置を探してそれ以降
		ptr1 = strstr(url, "//") + 2;
	}
	ptr2 = strchr(ptr1, '/');//
	memcpy(dns.name1, ptr1, ptr2 - ptr1);//

	return dns;
}

struct STRING getfilename(char url[]){
	struct STRING filename = {""};
	char *ptr = strrchr(url, '/') + 1;;
	char *eptr;
	eptr = strrchr(url, '\0');
	memcpy(filename.name1, ptr, eptr - ptr); 
	return filename;
}

struct STRING getreq(char url[]){
	struct STRING req = {""};
	char *ptr1 = url;
	char *ptr2;
	char *ptr3;
	if(strstr(url, "//")){
		//"//"の位置を探してそれ以降
		ptr1 = strstr(url, "//") + 2;
	}
	ptr2 = strchr(ptr1, '/');
	ptr3 = strrchr(url, '\0');
	memcpy(req.name1, ptr2, ptr3 - ptr2);

	return req;
}

struct STRING checkurl(char url[]){
	struct STRING result = {""};
	SOCKET sock;
	struct sockaddr_in server;
	strcpy(result.name1, url);
	char recv2[1024] = "";
	char tmp[64] = "";
	char num[4] = "";
	char *a;
	char *b;
	char *c;
	char req[128] = "";
	sock = socket(AF_INET, SOCK_STREAM, 0);

	sprintf(req, "HEAD %s HTTP/1.0\r\n\r\n", getreq(url).name1);
	//socket作成のエラー処理
	if(sock == INVALID_SOCKET){
		printf("socket: %d\n",WSAGetLastError());
		return result;
	}
	//接続先指定用構造体の初期値
	server.sin_family = AF_INET;
	server.sin_port = htons(80);
	server.sin_addr.S_un.S_addr = inet_addr(getadd(getdns(url).name1).name1);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0){
		printf("failed\n");
	}

	//HEADリクエスト送信
	int n = send(sock, req, (int)strlen(req), 0);
	if(n < 0){
		printf("send : %d\n", WSAGetLastError());
		return result;
	}
	while(1){
		n = recv(sock, recve, sizeof(recve), 0);
		if(n == SOCKET_ERROR){
			printf("recv : %d\n",WSAGetLastError());
			break;
		}else if(n == 0){
			break;
		}
		strncat(recv2, recve, n);
	}
	strncat(num, recv2 + 9, 3);
	if(num == "301"){
		a = strstr(recv2, "Location") + 7;
		b = strstr(a, "\r\n");
		strncpy(tmp, a, b - a);
	}
	sprintf(result.name1, "%s", num);
	//printf("%s\n", recv2);
	//printf("%s\n", num);
	//printf("%s\n", tmp);
	//memcpy(result.name1, tmp, b - a);
	return result;
}

struct RECORD getmeta(char url[]){
	SOCKET sock;
	struct sockaddr_in server;
	struct RECORD meta = {"", "", "", 0};
	char recv2[1024] = "";
	char tmp[64];
	char *req;
	char *ptr1;
	char *ptr2;

	req = (char *)malloc(sizeof(char) * strlen(url) + 20);
	sprintf(req, "HEAD %s HTTP/1.0\r\n\r\n", getreq(url).name1);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	//socket作成のエラー処理
	if(sock == INVALID_SOCKET){
		printf("socket: %d\n",WSAGetLastError());
		return meta;
	}

	//接続先指定用構造体の初期値
	server.sin_family = AF_INET;
	server.sin_port = htons(80);
	server.sin_addr.S_un.S_addr = inet_addr(getadd(getdns(url).name1).name1);
	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0){
		printf("failed\n");
	}

	//HEADリクエスト送信
	int n = send(sock, req, (int)strlen(req), 0);
	if(n < 0){
		printf("send : %d\n", WSAGetLastError());
		return meta;
	}

	//HEAD取得
	while(1){
		n = recv(sock, recve, sizeof(recve), 0);
		if(n == SOCKET_ERROR){
			printf("recv : %d\n",WSAGetLastError());
			return meta;
		}else if(n == 0){
			break;
		}
		strncat(recv2, recve, n);
	}

	//サイズ取得
	ptr1 = strstr(recv2, "Content-length") + 16;
	if((int)ptr1 == 16)	ptr1 = strstr(recv2, "Content-Length") + 16;
	if((int)ptr1 == 16)	return meta;
	ptr2 = strstr(ptr1, "\r\n");
	strncpy(tmp, ptr1, ptr2 - ptr1);
	meta.size = (long)atoi(tmp);
	
	//Modifie取得
	ptr1 = strstr(recv2, "Last-Modified:") + 15;
	if((int)ptr1 == 15)	ptr1 = strstr(recv2, "Last-modified:") + 15;
	if((int)ptr1 == 15)	return meta;
	ptr2 = strstr(ptr1, "\r\n");
	strncpy(meta.modi, ptr1, ptr2 - ptr1);

	free(req);
	return meta;
}

struct STRING extension(char filename[]){
	struct STRING ext = {""};
	char *ptr1 = filename;
	char *ptr2;
	//"//"の位置を探してそれ以降
	ptr1 = strstr(filename, ".");
	ptr2 = strchr(ptr1, '\0');
	memcpy(ext.name1, ptr1, ptr2 - ptr1);

	return ext;
}

struct STRING name(char filename[]){
	struct STRING name = {""};
	char *ptr1 = filename;
	char *ptr2;
	//"//"の位置を探してそれ以降
	ptr2 = strchr(ptr1, '.');
	memcpy(name.name1, ptr1, ptr2 - ptr1);

	return name;
}

long filesize(char filename[]){
	FILE *fp;
	long size = 0;
	fpos_t sz;
	char *fname;
	fname = (char *)malloc(sizeof(char) * strlen(filename));
	sprintf(fname, "%s", filename);
	fp = fopen(fname, "rb");
	if(fp == NULL){
		printf("%sファイルが開けません\n", filename);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &sz);
	size = (long)sz;
	fclose(fp);
	return size;
}

int make_pic(char url[], int num){
	SOCKET sock;
	struct sockaddr_in server;
	FILE *fp;
	int head_size = 0;
	int write_size = 0;
	char url2[256];
	char *get_request;
	char *head_request;
	char tmp[RECEVESIZE] = {'\0'};
	int i = 0;

	//GETリクエスト作成
	get_request = (char *)malloc(sizeof(char) * strlen(getreq(url).name1) + 21);
	sprintf(get_request, "GET %s HTTP/1.0\r\n\r\n", getreq(url).name1);

	//HEADリクエスト作成
	head_request = (char *)malloc(sizeof(char) * strlen(getreq(url).name1) + 21);
	sprintf(head_request, "HEAD %s HTTP/1.0\r\n\r\n", getreq(url).name1);

	//ソケット作成
	sock = socket(AF_INET, SOCK_STREAM, 0);
	//socket作成のエラー処理
	if(sock == INVALID_SOCKET){
		printf("socket: %d\n",WSAGetLastError());
		return 1;
	}
	//接続先指定用構造体の初期値
	server.sin_family = AF_INET;
	server.sin_port = htons(80);
	server.sin_addr.S_un.S_addr = inet_addr(getadd(getdns(url).name1).name1);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0){
		printf("failed\n");
	}
	head_size = get_size(getadd(getdns(url).name1).name1, head_request);
	//GETリクエスト送信
	int n = send(sock, get_request, (int)strlen(get_request), 0);
	if(n < 0){
		printf("send : %d\n", WSAGetLastError());
		return 1;
	}
	//サーバからのHTTPメッセージ受信
	fp = fopen(getfilename(url).name1, "wb");
	while(1){
		n = recv(sock, recve, sizeof(recve), 0);
		if(n == SOCKET_ERROR){
			printf("recv : %d\n",WSAGetLastError());
			break;
		}else if(n == 0){
			break;
		}

		if(write_size >= head_size){
			//書き込む
			fwrite(recve, n, 1, fp);
		}else{
			if(write_size + n > head_size){
				//必要な分だけ書き込む
				memcpy(tmp, recve + (head_size - write_size), n -(head_size - write_size));
				fwrite(tmp, n -(head_size - write_size), 1, fp);
			}else{
				fwrite(recve, n, 1, fp);
			}
			write_size += n;
		}
	}

	//終了処理
	closesocket(sock);
	WSACleanup();
	fclose(fp);


	newname = (char *)malloc(sizeof(char) * (strlen(cacheroot) + strlen(name(getfilename(url).name1).name1) + strlen(extension(getfilename(url).name1).name1) + 3));
	sprintf(newname, "%s%s_%d%s", cacheroot, name(getfilename(url).name1).name1, num,extension(getfilename(url).name1).name1);

	//ファイルの移動
	if(rename(getfilename(url).name1, newname) == 0)	printf("ファイルを%sに移動しました\n", cacheroot);
	else	printf("ファイルの移動に失敗しました\n");

	free(head_request);
	free(get_request);
	free(newname);

	return 0;
}

int make_root_ini(void){
	FILE *fp;
	char croot[128];

	printf("キャッシュファイルの絶対パスを入力してください\n");
	scanf("%s",&croot);

	fp = fopen(root_ini, "w");
	fprintf(fp, "cacheroot: %s\r\n", croot);
	fclose(fp);

	return 0;
}

int make_ini(void){	
	double csize = 0; 
	FILE *fp;

	printf("キャッシュファイルのサイズ(GB)を入力してください\n");
	scanf("%lf", &csize);

	fprintf(fp, "cachesize: %fGB\r\n", csize);

	fclose(fp);
	return 0;
}

struct STRING get_root_ini(void){
	struct STRING root = {""};
	FILE *fp;
	char tmp[64];
	char *ptr1;
	char *ptr2;

	fp = fopen(root_ini, "r");
	if(fp == NULL){
		make_root_ini();
		return get_root_ini();
	}
	fgets(tmp, sizeof(tmp), fp);

	ptr1 = strstr(tmp, " ") + 1;
	ptr2 = tmp + strlen(tmp);

	memcpy(root.name1, ptr1, ptr2 - ptr1);
	fclose(fp);
	return root;
}

struct STRING current_time(){
	struct STRING date = {""};
	time_t timer;
	struct tm *local;

	time(&timer);
	local = localtime(&timer);

	sprintf(date.name1, "%4d/%2d/%2d",
		local->tm_year + 1900,
		local->tm_mon + 1,
		local->tm_mday);

	return date;
}

void add_cache(char filename[], char url[]){
	FILE *fp;
	char cachefile[128] = "";
	struct RECORD record = {"", "", "", 0};

	sprintf(cachefile, "%smanage.cache", get_root_ini().name1);
	record = getmeta(url);

	fp = fopen(cachefile, "a+");
	fprintf(fp, "%s, %s, %s, %d\r\n", filename, url, record.modi, record.size);

	fclose(fp);
}

void del_cache(char url[]){
	FILE *fp1, *fp2;
	char tmp[256] = "";
	char cachefile[64] = "";
	sprintf(cachefile, "%smanage.cache", cacheroot);

	fp1 = fopen(cachefile, "r");
	fp2 = fopen("tmp.cache", "w");

	while(fgets(tmp, 256, fp1) != NULL){
		if(strstr(tmp, url) == NULL)	fputs(tmp, fp2);
	}
	fclose(fp1);
	fclose(fp2);
	remove(cachefile);
	int a = rename("tmp.cache", cachefile);
}

int main(){
	clock_t t1, t2;
	t1 = clock();
	//winsock初期化
	if(WSAStartup(MAKEWORD(2,0),&wsaData) != 0){
		printf("WSAStartup failed\n");
		return 1;
	}
	char url[] = "http://www.ricoh.co.jp/dc/cx/cx1/img/sample_04.jpg";
	char filename[] = "C:\\Users\\b1010162\\Desktop\\img\\lena.jpg";
	//make_pic(url, 0);
	//char temp[128] = "";
	//sprintf(temp, "%s",checkurl(url).name1);
	//printf("%s\n", checkurl(url).name1);
	//if(strcmp(checkurl(url).name1, "200") != 0)	printf("bad request\n");
	//__uint64  size;
	//if(GetDirSize(_T("C:\\Users\\b1010162\\Desktop\\img"), &size))
	//	printf( "はい失敗\n");
	//printf( "%llu\n", size);
	//printf("size = %d\n", checksize(url));

	//make_ini();
	//make_root_ini();
	//printf("root = %s\n", get_root_ini().name1);
	//make_cache_manage();
	//add_cache("test.hoge", "http://hogehoge.jp", "0"); 
	//del_cache("http://www.ricoh.co.jp/dc/cx/cx1/img/sample_04.jpg");

	add_cache("test.test", url);
	t2 = clock();
	printf("time = %f\n", (double)(t2 - t1) / CLOCKS_PER_SEC);
	WSACleanup();
	return 0;
}
