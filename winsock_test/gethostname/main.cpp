#include <stdio.h>
#include <winsock2.h>

int main(int argc , char *argv[]) {
	LPHOSTENT host;
	WSADATA wsaData;
	int iCount;
	char address[128];
	scanf("%s", &address);
	if (argc == 1) return 0;
	
	WSAStartup(2 , &wsaData);
	host = gethostbyname(address);
	if (host == NULL) {
		fprintf(stderr , "ホスト名の取得に失敗しました : %s" , address);
		return 0;
	}

	printf("公式名 = %s\n" , host->h_name);
	for(iCount = 0 ; host->h_aliases[iCount] ; iCount++) {
		printf("別名 = %s\n" , host->h_aliases[iCount]);
	}

	for(iCount = 0 ; host->h_addr_list[iCount] ; iCount++) {
		printf("IP = %d.%d.%d.%d\n" ,
			(BYTE)*((host->h_addr_list[iCount])) ,
			(BYTE)*((host->h_addr_list[iCount]) + 1) ,
			(BYTE)*((host->h_addr_list[iCount]) + 2) ,
			(BYTE)*((host->h_addr_list[iCount]) + 3)
		);
	}

	WSACleanup();
	while(1){}
	return 0;
}