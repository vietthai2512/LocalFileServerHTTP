#include <stdio.h>
#include <WinSock2.h>

void Concat(char** phtml, char* str)
{
	int oldlen = (*phtml) == NULL ? 0 : strlen((*phtml));
	int tmplen = strlen(str);
	*phtml = (char*)realloc(*phtml, oldlen + tmplen + 1);
	memset(*phtml + oldlen, 0, tmplen + 1);
	sprintf(*phtml + oldlen, "%s", str);
}

char* ScanFolder(const char* folder)
{
	char* html = NULL;
	Concat(&html, (char*)"<html>");
	WIN32_FIND_DATAA FindData;
	char findpath[1024];
	memset(findpath, 0, sizeof(findpath));
	sprintf(findpath, "%s\\*.*", folder);
	HANDLE hFind = FindFirstFileA(findpath, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		char link[1024];
		sprintf(link, "<a href=\\%s\\%s></a><br>", FindData.cFileName, FindData.cFileName);
		Concat(&html, link);
		while (FindNextFileA(hFind, &FindData))
		{
			sprintf(link, "<a href=\\%s\\>%s</a><br>", FindData.cFileName, FindData.cFileName);
			Concat(&html, link);
		}
	}

	Concat(&html, (char*)"</html>");

	return html; 
}

DWORD WINAPI ClientThread(LPVOID param)
{
	SOCKET c = (SOCKET)param;
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	recv(c, buffer, sizeof(buffer), 0);
	char action[1024];
	char path[1024];
	memset(action, 0, sizeof(action));
	memset(path, 0, sizeof(path));
	sscanf(buffer, "%s%s", action, path);

	if (strcmp(action, "GET") == 0)
	{
		char* html = ScanFolder("C:");
		char* response = (char*)calloc(strlen(html) + 1024, 1);
		sprintf(response, 
			"HTTP/1.1 200 OK\r\nServer: THAI_LOCAL\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s",
			strlen(html), html);
		send(c, response, strlen(response), 0);
		closesocket(c);
		free(response);
		free(html);
		response = NULL; html = NULL;
	}
	return 0;
}

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN saddr;
	saddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	saddr.sin_port = htons(8888);
	saddr.sin_family = AF_INET;

	bind(s, (sockaddr*)&saddr, sizeof(saddr));
	listen(s, 10);
	while (true)
	{
		SOCKADDR_IN caddr;
		int clen = sizeof(caddr);
		SOCKET c = accept(s, (sockaddr*)&caddr, &clen);
		CreateThread(NULL, 0, ClientThread, (LPVOID)c, 0, NULL);
	}
}