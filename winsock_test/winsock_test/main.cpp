#include "winsock2.h"
#include "stdio.h"
#include "string.h"
#include <windows.h>
#include <tchar.h>
#include <time.h>


#define RECEVESIZE 1000
WSADATA wsaData;
char recve[RECEVESIZE] = "";
char cacheroot[] = "C:\\Users\\b1010162\\Desktop\\img\\";


typedef unsigned __int64 __uint64;

// DWORD2����QWORD�����
inline __uint64 MakeQWord( DWORD hi, DWORD low)
{
	return ((__uint64)hi << 32) | low;
}

// WIN32_FIND_DATA�̏�񂩂�A�t�@�C���Ȃ̂��f�B���N�g���Ȃ̂������ʂ���
// GetDirSize�֐��Ŏg������
bool IsDirectory( WIN32_FIND_DATA *FindData)
{
	return FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

// �t�H���_�̃T�C�Y���ċA�I�ɒ��ׂ�B
// �A�N�Z�X���̊֌W�ȂǂŁA1�ł����s�����璆�~���Ė߂��Ă���B
// �߂�l ���� 0 / ���s 0�ȊO
int GetDirSize( LPCTSTR path, __uint64 *lpSize)
{
	int res = 1;
	*lpSize = 0;
	TCHAR DirSpec[MAX_PATH+1];
	//	_stprintf_p( DirSpec, MAX_PATH+1, _T("%s\\*"), path);
	_stprintf( DirSpec, _T("%s\\*"), path);	// BCC�p

	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile( DirSpec, &FindData);
	if(hFind == INVALID_HANDLE_VALUE)
		goto Error;
	do {
		__uint64 size = 0;
		if(IsDirectory( &FindData)) {
			// �f�B���N�g���B�u.�v�u..�v�͖������āA�ċA�B
			if(!_tcscmp( _T("."), FindData.cFileName) || !_tcscmp( _T(".."), FindData.cFileName))
				continue;
			TCHAR buf[MAX_PATH+1];
			//			_stprintf_p( buf, MAX_PATH+1, _T("%s%s\\"), path, FindData.cFileName);
			_stprintf( buf, _T("%s%s\\"), path, FindData.cFileName);	// BCC�p
			if(GetDirSize( buf, &size))
				goto Error;
			*lpSize += size;
		}
		else {
			// �t�@�C��
			*lpSize += MakeQWord( FindData.nFileSizeHigh, FindData.nFileSizeLow);
		}

		// �f�o�b�O�p
		_tprintf( _T("%s, %llu\n"), FindData.cFileName, MakeQWord( FindData.nFileSizeHigh, FindData.nFileSizeLow));

	}
	while(FindNextFile(hFind, &FindData));
	if(GetLastError() != ERROR_NO_MORE_FILES)
		goto Error;

	// ����
	res = 0;
Error:
	FindClose(hFind);
	return res;
}

struct STRING{
	char name1[256];
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
	//socket�쐬�̃G���[����
	if(sock1 == INVALID_SOCKET){
		printf("socket: %d\n",WSAGetLastError());
		return -1;
	}

	//�ڑ���w��p�\���̂̏����l
	server1.sin_family = AF_INET;
	server1.sin_port = htons(80);
	server1.sin_addr.S_un.S_addr = inet_addr(add);

	if (connect(sock1, (struct sockaddr *)&server1, sizeof(server1)) != 0){
		printf("failed\n");
	}

	//HEAD���N�G�X�g���M
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
		//"//"�̈ʒu��T���Ă���ȍ~
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
		//"//"�̈ʒu��T���Ă���ȍ~
		ptr1 = strstr(url, "//") + 2;
	}
	ptr2 = strchr(ptr1, '/');
	ptr3 = strrchr(url, '\0');
	memcpy(req.name1, ptr2, ptr3 - ptr2);

	return req;
}

struct STRING checkurl(char url[], char req[]){
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
	sock = socket(AF_INET, SOCK_STREAM, 0);
	//socket�쐬�̃G���[����
	if(sock == INVALID_SOCKET){
		printf("socket: %d\n",WSAGetLastError());
		return result;
	}
	//�ڑ���w��p�\���̂̏����l
	server.sin_family = AF_INET;
	server.sin_port = htons(80);
	server.sin_addr.S_un.S_addr = inet_addr(getadd(getdns(url).name1).name1);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0){
		printf("failed\n");
	}

	//HEAD���N�G�X�g���M
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
	}else{
		return result;
	}

	printf("%s\n", recv2);
	printf("%s\n", num);
	printf("%s\n", tmp);
	memcpy(result.name1, tmp, b - a);
	return result;
}

long checksize(char url[]){
	long size = 0;	
	SOCKET sock;
	struct sockaddr_in server;
	char *req;
	req = (char *)malloc(sizeof(char) * strlen(url) + 20);
	char recv2[1024] = "";
	char tmp[64] = "";
	char num[4] = "";
	char *a;
	char *b;
	sprintf(req, "HEAD %s HTTP/1.0\r\n\r\n", getreq(url).name1);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	//socket�쐬�̃G���[����
	if(sock == INVALID_SOCKET){
		printf("socket: %d\n",WSAGetLastError());
		return size;
	}
	//�ڑ���w��p�\���̂̏����l
	server.sin_family = AF_INET;
	server.sin_port = htons(80);
	server.sin_addr.S_un.S_addr = inet_addr(getadd(getdns(url).name1).name1);
	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0){
		printf("failed\n");
	}
	//HEAD���N�G�X�g���M
	int n = send(sock, req, (int)strlen(req), 0);
	if(n < 0){
		printf("send : %d\n", WSAGetLastError());
		return size;
	}
	while(1){
		n = recv(sock, recve, sizeof(recve), 0);
		if(n == SOCKET_ERROR){
			printf("recv : %d\n",WSAGetLastError());
			return -1;
		}else if(n == 0){
			break;
		}
		strncat(recv2, recve, n);
	}
	a = strstr(recv2, "Content-length") + 16;
	if((int)a == 16)	a = strstr(recv2, "Content-Length") + 16;
	if((int)a == 16)	return -1;
	b = strstr(a, "\r\n");
	strncpy(tmp, a, b - a);
	size = (long)atoi(tmp);
	free(req);
	return size;
}

struct STRING extension(char filename[]){
	struct STRING ext = {""};
	char *ptr1 = filename;
	char *ptr2;
	//"//"�̈ʒu��T���Ă���ȍ~
	ptr1 = strstr(filename, ".");
	ptr2 = strchr(ptr1, '\0');
	memcpy(ext.name1, ptr1, ptr2 - ptr1);

	return ext;
}

struct STRING name(char filename[]){
	struct STRING name = {""};
	char *ptr1 = filename;
	char *ptr2;
	//"//"�̈ʒu��T���Ă���ȍ~
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
		printf("%s�t�@�C�����J���܂���\n", filename);
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

	//GET���N�G�X�g�쐬
	get_request = (char *)malloc(sizeof(char) * strlen(getreq(url).name1) + 21);
	sprintf(get_request, "GET %s HTTP/1.0\r\n\r\n", getreq(url).name1);

	//HEAD���N�G�X�g�쐬
	head_request = (char *)malloc(sizeof(char) * strlen(getreq(url).name1) + 21);
	sprintf(head_request, "HEAD %s HTTP/1.0\r\n\r\n", getreq(url).name1);

	//�\�P�b�g�쐬
	sock = socket(AF_INET, SOCK_STREAM, 0);
	//socket�쐬�̃G���[����
	if(sock == INVALID_SOCKET){
		printf("socket: %d\n",WSAGetLastError());
		return 1;
	}
	//�ڑ���w��p�\���̂̏����l
	server.sin_family = AF_INET;
	server.sin_port = htons(80);
	server.sin_addr.S_un.S_addr = inet_addr(getadd(getdns(url).name1).name1);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0){
		printf("failed\n");
	}
	head_size = get_size(getadd(getdns(url).name1).name1, head_request);
	//GET���N�G�X�g���M
	int n = send(sock, get_request, (int)strlen(get_request), 0);
	if(n < 0){
		printf("send : %d\n", WSAGetLastError());
		return 1;
	}
	//�T�[�o�����HTTP���b�Z�[�W��M
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
			//��������
			fwrite(recve, n, 1, fp);
		}else{
			if(write_size + n > head_size){
				//�K�v�ȕ�������������
				memcpy(tmp, recve + (head_size - write_size), n -(head_size - write_size));
				fwrite(tmp, n -(head_size - write_size), 1, fp);
			}else{
				fwrite(recve, n, 1, fp);
			}
			write_size += n;
		}
	}

	//�I������
	closesocket(sock);
	WSACleanup();
	fclose(fp);

	char *newname;
	newname = (char *)malloc(sizeof(char) * (strlen(cacheroot) + strlen(name(getfilename(url).name1).name1) + strlen(extension(getfilename(url).name1).name1) + 3));
	sprintf(newname, "%s%s_%d%s", cacheroot, name(getfilename(url).name1).name1, num,extension(getfilename(url).name1).name1);

	//�t�@�C���̈ړ�
	if(rename(getfilename(url).name1, newname) == 0)	printf("�t�@�C����%s�Ɉړ����܂���\n", cacheroot);
	else	printf("�t�@�C���̈ړ��Ɏ��s���܂���\n");
	
	free(head_request);
	free(get_request);
	free(newname);
	
	return 0;
}

int main(){
	clock_t t1, t2;
	t1 = clock();
	//winsock������
	if(WSAStartup(MAKEWORD(2,0),&wsaData) != 0){
		printf("WSAStartup failed\n");
		return 1;
	}
	char url[] = "http://asia.olympus-imaging.com/products/dslr/e420/sample/images/sample_02.jpg";
	char filename[] = "C:\\Users\\b1010162\\Desktop\\img\\lena.jpg";
	printf("URL = %s\n", url);
	//printf("%s\n", url);
	//memset(url, '\0', strlen(url));
	//strcat(url, "http://www.ricoh.co.jp/dc/cx/cx1/img/sample_04.jpg");
	//strcat(test, a);
	//sprintf(test, "%s_%d", getfilename(url).name1, b);
	//printf("%s\n", test);
	//char req[] = "HEAD /content/m13085/latest/lena.jpg HTTP/1.0\r\n\r\n";
	make_pic(url, 0);
	//printf("url = %s\n", url);
	//printf("dns = %s\n", getdns(url).name1);
	//printf("filename = %s\n", getfilename(url).name1);
	//printf("ip = %s\n", getadd(getdns(url).name1).name1);
	//printf("req = %s\n", getreq(url).name1);
	//printf("checkurl = %s\n", checkurl(url, req).name1);
	//printf("%s\n", extension(getfilename(url).name1).name1);
	//printf("%s\n", name(getfilename(url).name1).name1);
	//printf("filesize = %d\n", filesize(filename));
	//printf("checksize = %d\n", checksize(url));


	//__uint64  size;
	//if(GetDirSize(_T("C:\\Users\\b1010162\\Desktop\\img"), &size))
	//	printf( "�͂����s\n");

	//printf( "%llu\n", size);

	//printf("size = %d\n", checksize(url));
	t2 = clock();
	printf("time = %f\n", (double)(t2 - t1) / CLOCKS_PER_SEC);
	WSACleanup();
	return 0;
}
