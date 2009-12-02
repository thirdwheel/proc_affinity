#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>

void printError( TCHAR* msg );

int main(int argc, char *argv[]) {

	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	TCHAR *procname;
	DWORD affinity;

	if (argc != 3) {
		fprintf(stderr, "Syntax: %s <processname> <mask>\n", argv[0]);
		exit(1);
	}
	
	procname = TEXT(argv[1]);
	sscanf(argv[2], "%x", &affinity);
	
	if ((hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE) {
		printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
		exit(2);
	}

	pe32.dwSize = sizeof( PROCESSENTRY32 );

	if (!Process32First(hProcessSnap, &pe32)) {
		printError(TEXT("Process32First")); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		exit(2);
	}
	
	do {
		if (strcmp(procname, pe32.szExeFile) == 0) {
			if ((hProcess=OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe32.th32ProcessID)) == NULL) {
				fprintf(stderr, TEXT("OpenProcess\n"));
			} else {
				printf("Attempting to set affinity mask of process %d to %d\n", 
					pe32.th32ProcessID, affinity);
				if (SetProcessAffinityMask(hProcess, affinity) == 0) {
					printError(TEXT("SetProcessAffinity (of process)\n"));
				}
				CloseHandle(hProcess);
			}
		}
	} while (Process32Next(hProcessSnap, &pe32));
	
	CloseHandle(hProcessSnap);
	return 0;
}

void printError( TCHAR* msg ) {
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL );

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while( ( *p > 31 ) || ( *p == 9 ) )
		++p;
	do { *p-- = 0; } while( ( p >= sysMsg ) &&
		( ( *p == '.' ) || ( *p < 33 ) ) );

	// Display the message
	_tprintf( TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg );
}
