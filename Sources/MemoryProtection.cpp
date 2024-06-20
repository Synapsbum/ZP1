/////////////////////
// Includes
#include "r3dPCH.h"
#include "r3d.h"
#include "..\..\inject detect\Anticheat.h" // aLca :: For CorruptStack
#include <bcrypt.h>

using namespace std;

// XOR

#ifndef _XOR_H
#define _XOR_H
template <int XORSTART, int BUFLEN, int XREFKILLER>

class XorStr
{
private:
	XorStr();
public:
	char s[BUFLEN];

	XorStr(const char * xs);

	~XorStr()
	{
		for (int i = 0; i<BUFLEN; i++) s[i] = 0;
	}
};

template <int XORSTART, int BUFLEN, int XREFKILLER>
XorStr<XORSTART, BUFLEN, XREFKILLER>::XorStr(const char * xs)
{
	int xvalue = XORSTART;
	int i = 0;

	for (; i < (BUFLEN - 1); i++)
	{
		s[i] = xs[i - XREFKILLER] ^ xvalue;
		xvalue += 1;
		xvalue %= 256;
	}

	s[BUFLEN - 1] = 0;
}

#endif

DWORD PROCESSTYPE;
string abcdef;

char userhere[MAX_PATH];
DWORD getlength = MAX_PATH;
string userstr;

#define NT_SUCCESS(x) ((x) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH 0xc0000004

#define SystemHandleInformation 16
#define ObjectBasicInformation 0
#define ObjectNameInformation 1
#define ObjectTypeInformation 2

typedef NTSTATUS(NTAPI *_NtDuplicateObject)(
	HANDLE SourceProcessHandle,
	HANDLE SourceHandle,
	HANDLE TargetProcessHandle,
	PHANDLE TargetHandle,
	ACCESS_MASK DesiredAccess,
	ULONG Attributes,
	ULONG Options
	);


typedef struct _SYSTEM_HANDLE
{
	ULONG ProcessId;
	BYTE ObjectType;
	BYTE Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG HandleCount;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;


typedef NTSTATUS(__stdcall *_NtQuerySystemInformation)(ULONG SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

PVOID GetLibraryProcAddress(PSTR LibraryName, PSTR ProcName)
{
	return GetProcAddress(GetModuleHandleA(LibraryName), ProcName);
}
_NtQuerySystemInformation NtQuerySystemInformationProc = (_NtQuerySystemInformation)GetLibraryProcAddress(/*ntdll.dll*/XorStr<0x78, 10, 0x78280739>("\x16\x0D\x1E\x17\x10\x53\x1A\x13\xEC" + 0x78280739).s, /*NtQuerySystemInformation*/XorStr<0x40, 25, 0x391C1554>("\x0E\x35\x13\x36\x21\x37\x3F\x14\x31\x3A\x3E\x2E\x21\x04\x20\x29\x3F\x23\x3F\x32\x20\x3C\x39\x39" + 0x391C1554).s);
_NtDuplicateObject NtDuplicateObject = (_NtDuplicateObject)GetLibraryProcAddress(/*ntdll.dll*/XorStr<0xB5, 10, 0x2E7952ED>("\xDB\xC2\xD3\xD4\xD5\x94\xDF\xD0\xD1" + 0x2E7952ED).s, /*NtDuplicateObject*/XorStr<0x8F, 18, 0x99FF06DE>("\xC1\xE4\xD5\xE7\xE3\xF8\xFC\xF5\xF6\xEC\xFC\xD5\xF9\xF6\xF8\xFD\xEB" + 0x99FF06DE).s);

typedef NTSTATUS(NTAPI* ZwBasedTermOrg)(IN HANDLE, IN NTSTATUS);
ZwBasedTermOrg orgzwterm = (ZwBasedTermOrg)GetProcAddress(GetModuleHandleA(/*ntdll.dll*/XorStr<0x7E, 10, 0xEC594474>("\x10\x0B\xE4\xED\xEE\xAD\xE0\xE9\xEA" + 0xEC594474).s), /*ZwTerminateProcess*/XorStr<0x88, 19, 0xAB21E698>("\xD2\xFE\xDE\xEE\xFE\xE0\xE7\xE1\xF1\xE5\xF7\xC3\xE6\xFA\xF5\xF2\xEB\xEA" + 0xAB21E698).s);

//

inline DWORD GetCsrssProcessId()
{
	// Don't forget to set dw.Size to the appropriate
	// size (either OSVERSIONINFO or OSVERSIONINFOEX)

	// for a full table of versions however what I have set will
	// trigger on anything XP and newer including Server 2003

	// Gotta love functions pointers
	typedef DWORD(__stdcall *pCsrGetId)();

	// Grab the export from NtDll
	pCsrGetId CsrGetProcessId = (pCsrGetId)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), /*CsrGetProcessId*/XorStr<0xAA, 16, 0x71D8E19F>("\xE9\xD8\xDE\xEA\xCB\xDB\xE0\xC3\xDD\xD0\xD1\xC6\xC5\xFE\xDC" + 0x71D8E19F).s);

	if (CsrGetProcessId)
		return CsrGetProcessId();
	else
		return 0;

}

BOOL EqualsMajorVersion(DWORD majorVersion)
{
	OSVERSIONINFOEX osVersionInfo;
	::ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osVersionInfo.dwMajorVersion = majorVersion;
	ULONGLONG maskCondition = ::VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL);
	return ::VerifyVersionInfo(&osVersionInfo, VER_MAJORVERSION, maskCondition);
}
BOOL EqualsMinorVersion(DWORD minorVersion)
{
	OSVERSIONINFOEX osVersionInfo;
	::ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osVersionInfo.dwMinorVersion = minorVersion;
	ULONGLONG maskCondition = ::VerSetConditionMask(0, VER_MINORVERSION, VER_EQUAL);
	return ::VerifyVersionInfo(&osVersionInfo, VER_MINORVERSION, maskCondition);
}

DWORD RvaToOffset(IMAGE_NT_HEADERS * nth, DWORD RVA)
{
	int i;
	int sections;
	PIMAGE_SECTION_HEADER sectionHeader;
	sectionHeader = IMAGE_FIRST_SECTION(nth);
	sections = nth->FileHeader.NumberOfSections;

	for (i = 0; i < sections; i++)
	{
		if (sectionHeader->VirtualAddress <= RVA)
		if ((sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize) > RVA)
		{
			RVA -= sectionHeader->VirtualAddress;
			RVA += sectionHeader->PointerToRawData;
			return RVA;
		}
		sectionHeader++;
	}
	return 0;
}

BOOL UnHookFunction(HMODULE lpModule, LPCSTR lpFuncName, unsigned char *lpBackup)
{
	DWORD dwAddr = (DWORD)GetProcAddress(lpModule, lpFuncName);
	if (WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, lpBackup, 6, 0))
	{
		return TRUE;
	}
	return FALSE;
}


BOOL ControlBytesForHook(const char* modulename, const char* procname)
{
	int sayo = 0;
	HMODULE lib = LoadLibraryA(modulename);
	if (lib){
		DWORD base = (DWORD)lib;
		void *fa = GetProcAddress(lib, procname);
		if (fa){
			//lets do the PE file system cast
			IMAGE_DOS_HEADER * dos = (IMAGE_DOS_HEADER *)lib;
			IMAGE_NT_HEADERS * nth = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
			BYTE *read = (BYTE *)fa;

			DWORD delta = (DWORD)fa - base; // this is the RVA that we need
			//printf("Delta: 0x%x\n", delta);
			//Lets apply the delta as a file offset
			CHAR hereisntdll[256];
			GetModuleFileNameA(LoadLibraryA(modulename), hereisntdll, 256);
			string herebaby = hereisntdll;
			//	cout << herebaby << endl;
			FILE *dll = fopen(herebaby.c_str(), "rb");

			DWORD OffSet = RvaToOffset(nth, delta); // this is what we need

			fseek(dll, OffSet, SEEK_SET);
			BYTE buffer[260];
			fread(buffer, 260, 1, dll);

			//lets test if the bytes are the same
			/*printf("\nFile on disk test bytes!\n\n");
			for (int i = 0; i<5; i++){
			printf("0x%x\n", buffer[i]);
			}*/
			BYTE get6byte[6];
			int promono = 0;
			for (int i = 0; i < 6; i++)
			{
				//printf("0x%x---", buffer[i]);
				//printf("0x%x\n", read[i]);

				if (buffer[i] == read[i])
				{
					promono++;

				}
				get6byte[i] = buffer[i];


			}
			if (promono < 5)
			{
				UnHookFunction(lib, procname, get6byte);

			}


			fclose(dll);
			return true;
		}
		else{
			//printf("%s", "middle");
			return false;
		}
	}
	else
	{
		//printf("%s", "bottom");
		return false;
	}

	FreeLibrary(lib);
}

void CALLBACK MessageBoxTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime)
{
	PostQuitMessage(0);
	exit(0);
	ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
	TerminateThread(GetCurrentThread(), 0);
	orgzwterm(GetCurrentProcess(), 0);
}

UINT TimedMessageBox(HWND hwndParent, LPCTSTR ptszMessage, LPCTSTR ptszTitle, UINT flags, DWORD dwTimeout)
{
	UINT idTimer = 0;
	UINT uiResult = 0;
	MSG msg;

	// Set a timer to dismiss the message box.
	idTimer = SetTimer(NULL, 0, dwTimeout, (TIMERPROC)MessageBoxTimer);

	uiResult = MessageBox(hwndParent, ptszMessage, ptszTitle, flags);

	// Finished with the timer.
	KillTimer(NULL, idTimer);


	// Check for WM_QUIT message in the queue. If it exists, time out occured. 
	// Eat the message so that the application doesn't quit.
	if (PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE))
	{
		//If timed out, then return zero.
		uiResult = 0;
	}

	return uiResult;
}

char *GetProcessUsername(HANDLE *phProcess, BOOL bIncDomain)
{
	static char sname[300];
	HANDLE tok = 0;
	HANDLE hProcess;
	TOKEN_USER *ptu;
	DWORD nlen, dlen;
	char name[300], dom[300], tubuf[300], *pret = 0;
	int iUse;

	//if phProcess is NULL we get process handle of this
	//process.
	hProcess = phProcess ? *phProcess : GetCurrentProcess();

	//open the processes token
	if (!OpenProcessToken(hProcess, TOKEN_QUERY, &tok)) goto ert;

	//get the SID of the token
	ptu = (TOKEN_USER*)tubuf;
	if (!GetTokenInformation(tok, (TOKEN_INFORMATION_CLASS)1, ptu, 300, &nlen)) goto ert;

	//get the account/domain name of the SID
	dlen = 300;
	nlen = 300;
	if (!LookupAccountSidA(0, ptu->User.Sid, name, &nlen, dom, &dlen, (PSID_NAME_USE)&iUse)) goto ert;


	//copy info to our static buffer
	if (dlen && bIncDomain) {
		strcpy(sname, dom);
		strcat(sname, "");
		strcat(sname, name);
	}
	else {
		strcpy(sname, name);
	}
	//set our return variable
	pret = sname;
	abcdef = pret;
ert:
	if (tok) CloseHandle(tok);
	return pret;
}

void enumsystemhandles()
{
	NTSTATUS status;
	PSYSTEM_HANDLE_INFORMATION handleInfo;
	ULONG handleInfoSize = 0x10000;
	HANDLE processHandle;
	ULONG i;
	handleInfo = (PSYSTEM_HANDLE_INFORMATION)malloc(handleInfoSize);
	while ((status = NtQuerySystemInformationProc(SystemHandleInformation, handleInfo, handleInfoSize, NULL)) == STATUS_INFO_LENGTH_MISMATCH)
		handleInfo = (PSYSTEM_HANDLE_INFORMATION)realloc(handleInfo, handleInfoSize *= 2);


	if (!NT_SUCCESS(status))
	{
		BOOL controlzwterm = ControlBytesForHook(/*ntdll.dll*/XorStr<0x1A, 10, 0x93081F54>("\x74\x6F\x78\x71\x72\x31\x44\x4D\x4E" + 0x93081F54).s, /*ZwTerminateProcess*/XorStr<0xB2, 19, 0x8992E8B5>("\xE8\xC4\xE0\xD0\xC4\xDA\xD1\xD7\xDB\xCF\xD9\xED\xCC\xD0\xA3\xA4\xB1\xB0" + 0x8992E8B5).s);
		//r3dOutToLog(/*Hile Algilandigi icin oyundan atildiniz.\n*/XorStr<0x7F, 42, 0x835E4108>("\x37\xE9\xED\xE7\xA3\xC5\xE9\xE1\xEE\xE4\xE8\xE4\xEF\xE5\xEA\xE7\xAF\xF9\xF2\xFB\xFD\xB4\xFA\xEF\xE2\xF6\xFD\xFB\xF5\xBC\xFC\xEA\xF6\xCC\xC5\xCB\xCD\xCD\xDF\x88\xAD" + 0x835E4108).s);
		TimedMessageBox(NULL, /*Error code: 0x003\nA Problem occured!*/XorStr<0xD2, 37, 0x893B236F>("\x97\xA1\xA6\xBA\xA4\xF7\xBB\xB6\xBE\xBE\xE6\xFD\xEE\xA7\xD0\xD1\xD1\xE9\xA5\xC5\xB6\x95\x87\x8B\x86\x8E\x81\xCD\x81\x8C\x93\x84\x80\x96\x90\xD4" + 0x893B236F).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
		Sleep(5000);
		PostQuitMessage(0);
		exit(0);
		ExitProcess(0);
		TerminateProcess(GetCurrentProcess(), 0);
		TerminateThread(GetCurrentThread(), 0);
		orgzwterm(GetCurrentProcess(), 0);
	}

	for (i = 0; i < handleInfo->HandleCount; i++)
	{
		SYSTEM_HANDLE handle = handleInfo->Handles[i];
		HANDLE dupHandle = NULL;

		if ((PVOID)handle.ObjectType == (PVOID)PROCESSTYPE)
		{
			if (processHandle = OpenProcess(PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, handle.ProcessId))
			{
				if (!NT_SUCCESS(NtDuplicateObject(
					processHandle,
					(HANDLE)handle.Handle,
					GetCurrentProcess(),
					&dupHandle,
					PROCESS_QUERY_INFORMATION,
					0,
					0
					)))
				{

					CloseHandle(dupHandle);
					CloseHandle(processHandle);
					continue;
				}

				DWORD m_dwPID = GetProcessId(dupHandle); // aLca :: FIX ME! Upd: Done.

				if (m_dwPID == GetCurrentProcessId())
				{
					if (GetCurrentProcessId() != handle.ProcessId)
					{
						if (GetCsrssProcessId() != handle.ProcessId)
						{
							char *getit = GetProcessUsername(&processHandle, 0);
							string yeninesil;
							if (getit != NULL) {
								yeninesil = getit;
							}
							if (!yeninesil.empty() && yeninesil.find(userstr.c_str()) != std::string::npos)
							{
								if (!DuplicateHandle(processHandle, (HANDLE)handle.Handle, NULL, NULL, 0, FALSE, 0x1))
								{
									BOOL controlzwterm = ControlBytesForHook(/*ntdll.dll*/XorStr<0x1A, 10, 0x93081F54>("\x74\x6F\x78\x71\x72\x31\x44\x4D\x4E" + 0x93081F54).s, /*ZwTerminateProcess*/XorStr<0xBD, 19, 0x7937E483>("\xE7\xC9\xEB\xA5\xB3\xAF\xAA\xAA\xA4\xB2\xA2\x98\xBB\xA5\xA8\xA9\xBE\xBD" + 0x7937E483).s);
									//r3dOutToLog(/*You have been kicked out for being detected.\n*/XorStr<0x7F, 42, 0x835E4108>("\x37\xE9\xED\xE7\xA3\xC5\xE9\xE1\xEE\xE4\xE8\xE4\xEF\xE5\xEA\xE7\xAF\xF9\xF2\xFB\xFD\xB4\xFA\xEF\xE2\xF6\xFD\xFB\xF5\xBC\xFC\xEA\xF6\xCC\xC5\xCB\xCD\xCD\xDF\x88\xAD" + 0x835E4108).s);
									TimedMessageBox(NULL, /*Error code: 0x004\nA Problem occured!*/XorStr<0xBB, 37, 0xD7F8D08B>("\xFE\xCE\xCF\xD1\xCD\xE0\xA2\xAD\xA7\xA1\xFF\xE6\xF7\xB0\xF9\xFA\xFF\xC6\x8C\xEE\x9F\xA2\xBE\xB0\xBF\xB1\xB8\xF6\xB8\xBB\xBA\xAF\xA9\xB9\xB9\xFF" + 0xD7F8D08B).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
									Sleep(5000);
									PostQuitMessage(0);
									exit(0);
									ExitProcess(0);
									TerminateProcess(GetCurrentProcess(), 0);
									TerminateThread(GetCurrentThread(), 0);
									orgzwterm(GetCurrentProcess(), 0);
								}
							}
						}
					}
				}
			}
			CloseHandle(dupHandle);
			CloseHandle(processHandle);
		}
	}

	free(handleInfo);
	handleInfo = NULL;
}

HANDLE H1; // Anti Thread suspension
HANDLE H2; // Memory Protection
HANDLE H3; // Anti Debug Stuff
HANDLE H4; // Debug Breakpoints
HANDLE H5;

void Suspend()
{
#ifdef _DEBUG
	r3dOutToLog("Anti Killthread Started!\n");
#endif
	while (true)
	{
		Sleep(2500);
		ResumeThread(H1);
		ResumeThread(H2);
		ResumeThread(H3);
		ResumeThread(H4);
		// ResumeThread(H5);

		if (WaitForSingleObject(H1, 0) == WAIT_OBJECT_0)
			H1 = NULL;

		if (H1 == NULL)
		{
			TimedMessageBox(NULL, /*Error code: 0x01C\nA Problem occured!*/XorStr<0x5F, 37, 0x5907C7DB>("\x1A\x12\x13\x0D\x11\x44\x06\x09\x03\x0D\x53\x4A\x5B\x14\x5D\x5F\x2C\x7A\x30\x52\x23\x06\x1A\x14\x1B\x1D\x14\x5A\x14\x1F\x1E\x0B\x0D\xE5\xE5\xA3" + 0x5907C7DB).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
			Sleep(5000);
			PostQuitMessage(0);
			exit(0);
			ExitProcess(0);
			TerminateProcess(GetCurrentProcess(), 0);
			TerminateThread(GetCurrentThread(), 0);

			if (WaitForSingleObject(H2, 0) == WAIT_OBJECT_0)
				H2 = NULL;

			if (H2 == NULL)
			{
				TimedMessageBox(NULL, /*Error code: 0x01F\nA Problem occured!*/XorStr<0x22, 37, 0xDC80F199>("\x67\x51\x56\x4A\x54\x07\x4B\x46\x4E\x4E\x16\x0D\x1E\x57\x00\x00\x74\x39\x75\x15\x66\x45\x57\x5B\x56\x5E\x51\x1D\x51\x5C\x23\x34\x30\x26\x20\x64" + 0xDC80F199).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
				Sleep(5000);
				PostQuitMessage(0);
				exit(0);
				ExitProcess(0);
				TerminateProcess(GetCurrentProcess(), 0);
				TerminateThread(GetCurrentThread(), 0);
			}

			if (WaitForSingleObject(H3, 0) == WAIT_OBJECT_0)
				H3 = NULL;

			if (H3 == NULL)
			{
				TimedMessageBox(NULL, /*Error code: 0x01F\nA Problem occured!*/XorStr<0x22, 37, 0xDC80F199>("\x67\x51\x56\x4A\x54\x07\x4B\x46\x4E\x4E\x16\x0D\x1E\x57\x00\x00\x74\x39\x75\x15\x66\x45\x57\x5B\x56\x5E\x51\x1D\x51\x5C\x23\x34\x30\x26\x20\x64" + 0xDC80F199).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
				Sleep(5000);
				PostQuitMessage(0);
				exit(0);
				ExitProcess(0);
				TerminateProcess(GetCurrentProcess(), 0);
				TerminateThread(GetCurrentThread(), 0);
			}

			if (WaitForSingleObject(H4, 0) == WAIT_OBJECT_0)
				H4 = NULL;

			if (H4 == NULL)
			{
				TimedMessageBox(NULL, /*Error code: 0x01F\nA Problem occured!*/XorStr<0x22, 37, 0xDC80F199>("\x67\x51\x56\x4A\x54\x07\x4B\x46\x4E\x4E\x16\x0D\x1E\x57\x00\x00\x74\x39\x75\x15\x66\x45\x57\x5B\x56\x5E\x51\x1D\x51\x5C\x23\x34\x30\x26\x20\x64" + 0xDC80F199).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
				Sleep(5000);
				PostQuitMessage(0);
				exit(0);
				ExitProcess(0);
				TerminateProcess(GetCurrentProcess(), 0);
				TerminateThread(GetCurrentThread(), 0);
			}

			//if (WaitForSingleObject(H5, 0) == WAIT_OBJECT_0)
			//	H5 = NULL;

			//if (H5 == NULL)
			//{
			//	TimedMessageBox(NULL, /*Error code: 0x01F\nA Problem occured!*/XorStr<0x22, 37, 0xDC80F199>("\x67\x51\x56\x4A\x54\x07\x4B\x46\x4E\x4E\x16\x0D\x1E\x57\x00\x00\x74\x39\x75\x15\x66\x45\x57\x5B\x56\x5E\x51\x1D\x51\x5C\x23\x34\x30\x26\x20\x64" + 0xDC80F199).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
			//	Sleep(5000);
			//	PostQuitMessage(0);
			//	exit(0);
			//	ExitProcess(0);
			//	TerminateProcess(GetCurrentProcess(), 0);
			//	TerminateThread(GetCurrentThread(), 0);
			//}
		}
	}
}

void enableDebugPrivileges()
{
#ifdef _DEBUG
	r3dOutToLog("Starting Debug Priv.\n");
#endif
	HANDLE Token;
	TOKEN_PRIVILEGES tp;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &Token))
	{
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (AdjustTokenPrivileges(Token, 0, &tp, sizeof(tp), NULL, NULL) == 0){
			BOOL controlzwterm = ControlBytesForHook(/*ntdll.dll*/XorStr<0xB5, 10, 0x7C063CA1>("\xDB\xC2\xD3\xD4\xD5\x94\xDF\xD0\xD1" + 0x7C063CA1).s, /*ZwTerminateProcess*/XorStr<0x42, 19, 0x06E5E96B>("\x18\x34\x10\x20\x34\x2A\x21\x27\x2B\x3F\x29\x1D\x3C\x20\x33\x34\x21\x20" + 0x06E5E96B).s);
			TimedMessageBox(NULL, /*Error code: 0x005\nA Problem occured!*/XorStr<0xB8, 37, 0xAAF906FE>("\xFD\xCB\xC8\xD4\xCE\x9D\xDD\xD0\xA4\xA4\xF8\xE3\xF4\xBD\xF6\xF7\xFD\xC3\x8B\xEB\x9C\xBF\xA1\xAD\xBC\xB4\xBF\xF3\xBB\xB6\xB5\xA2\xAA\xBC\xBE\xFA" + 0xAAF906FE).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
			Sleep(5000);
			PostQuitMessage(0);
			exit(0);
			ExitProcess(0);
			TerminateProcess(GetCurrentProcess(), 0);
			TerminateThread(GetCurrentThread(), 0);
			orgzwterm(GetCurrentProcess(), 0);
		}
		else {
			//SUCCESS
		}
	}
}

//int __stdcall MemoryProtectionLog(long lcode, long ParamSize, void * pParam)
//{
//	int	nRet = 0;
//	TCHAR	szMsg[MAX_PATH];
//	int __stdcall Memory_callback_Client(long lCode, long lParamSize, void* pParam);
//
//	switch (lcode)
//	{
//		case MEM_PROTECTION_FAILED:
//		{
//			char szMsg[255];
//			sprintf(szMsg, "AHNHS_ENGINE_DETECT_GAME_HACK\n%s", (char*)pParam);
//			// ShowMsgAndExitWithTimer(szMsg);
//
//			break;
//		}
//	}
//
//	if (nRet != MEM_PROTECTION_FAILED)
//	{
//		switch (nRet)
//		{
//		case MEM_PROTECTION_FAILED:
//		default:
//		{
//			r3dOutToLog("Memory Protection Failed! :(\n");
//		}
//		wsprintf(szMsg, "HACKSHIELD Error.(%x)", nRet);
//		r3dOutToLog(szMsg, "HACKSHIELD Error.(%x)", nRet);
//		r3dOutToLog("Crashing here :)\n");
//		// abort();
//		break;
//		}
//		MessageBox(NULL, szMsg, "Error", MB_OK);
//		return FALSE;
//	}
//
//	return 1;
//}


// Packets
/*
int running;
int lCode;
float time;
*/
extern float LastMemLog;
extern HANDLE MemoryLog;
extern DWORD temp;
extern int Hearthbeat_Thread();

void r3dWinApi()
{
	while (true)
	{
		enumsystemhandles();
		Sleep(3000);
	}
}

// aLca :: Memory Protection Ends here
//		:: Anti Debug Starts here

void anti_dump(){
	__asm // Fake Main size (anti dump) our main will be like 1 gb in memory and dumper should crash :D
	{
		MOV EAX, DWORD PTR FS : [0x30]
			MOV EAX, [EAX + 0Ch]
			MOV EAX, [EAX + 0Ch]
			ADD DWORD PTR[EAX + 20h], 3000h
	}
}

void olly_crash(){
	char p = 0;
	__asm
	{
		mov eax, fs: [30h]
			mov al, [eax + 2h]
			mov p, al
	}
	//debugger detected!
	if (p){
		//that method will crash OllyDbg
		OutputDebugString(TEXT("%s%s%s%s%s%s%s%s%s%s%s")
			TEXT("%s%s%s%s%s%s%s%s%s%s%s%s%s")
			TEXT("%s%s%s%s%s%s%s%s%s%s%s%s%s")
			TEXT("%s%s%s%s%s%s%s%s%s%s%s%s%s"));
		//do other things here.
	}
}

void AntiDebug()
{
	typedef unsigned long(__stdcall *pfnNtQueryInformationProcess)(IN  HANDLE, IN  unsigned int, OUT PVOID, IN ULONG, OUT PULONG);
	const int ProcessDbgPort = 7;
	pfnNtQueryInformationProcess NtQueryInfoProcess = NULL;
	unsigned long Ret;
	unsigned long IsRemotePresent = 0;
	HMODULE hNtDll = LoadLibrary(TEXT("ntdll.dll"));

	if (hNtDll == NULL) {}

	NtQueryInfoProcess = (pfnNtQueryInformationProcess)
		GetProcAddress(hNtDll, /*NtQueryInformationProcess*/XorStr<0xFF, 26, 0xC7298AD4>("\xB1\x74\x50\x77\x66\x76\x7C\x4F\x69\x6E\x66\x78\x66\x6D\x79\x67\x60\x7E\x41\x60\x7C\x77\x70\x65\x64" + 0xC7298AD4).s);

	if (NtQueryInfoProcess == NULL) {}

	Ret = NtQueryInfoProcess(GetCurrentProcess(), ProcessDbgPort, &IsRemotePresent, sizeof(unsigned long), NULL);

	if (Ret == 0x00000000 && IsRemotePresent != 0) {
		TimedMessageBox(NULL, /*Error code: 0x000\nA Problem occured!*/XorStr<0xDE, 37, 0x1A093CB8>("\x9B\xAD\x92\x8E\x90\xC3\x87\x8A\x82\x82\xD2\xC9\xDA\x93\xDC\xDD\xDE\xE5\xB1\xD1\xA2\x81\x9B\x97\x9A\x92\x95\xD9\x95\x98\x9F\x88\x8C\x9A\x64\x20" + 0x1A093CB8).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
		Sleep(5000);
		PostQuitMessage(0);
		exit(0);
		ExitProcess(0);
		TerminateProcess(GetCurrentProcess(), 0);
		TerminateThread(GetCurrentThread(), 0);
		orgzwterm(GetCurrentProcess(), 0);
	}
}

#include <AclAPI.h> // SetSecurityInfo, SE_KERNEL_OBJECT

DWORD ProtectProcess(void)
{
	HANDLE hProcess = GetCurrentProcess();
	PACL pEmptyDacl;
	DWORD dwErr;

	// using malloc guarantees proper alignment
	pEmptyDacl = (PACL)malloc(sizeof(ACL));

	if (!InitializeAcl(pEmptyDacl, sizeof(ACL), ACL_REVISION))
	{
		dwErr = GetLastError();
	}
	else
	{
		dwErr = SetSecurityInfo(hProcess, SE_KERNEL_OBJECT,
			DACL_SECURITY_INFORMATION, NULL, NULL, pEmptyDacl, NULL);
	}

	free(pEmptyDacl);
	return dwErr;
}

int Anti_Debugging();

// unsigned __stdcall DebuggingCheck();

HMODULE	hNtdll;
HMODULE hWinsta;

void WINAPI NewDbgUiRemoteBreakin(void);

#define MAKE_JUMP_OFFSET(_BaseOffset, _TargetOffset) ((LONG)(_TargetOffset) - (LONG)(_BaseOffset) - 5)

typedef int (WINAPI *WINSTATERMINATEPROC)(HANDLE hServer, DWORD dwProcessId, UINT uExitCode);

int Anti_Debugging()
{
	DWORD	oldProtect;
	PVOID	pDbgUiRemoteBreakin = NULL;

	hNtdll = GetModuleHandle(/*ntdll.DLL*/XorStr<0x33, 10, 0xA431FFF8>("\x5D\x40\x51\x5A\x5B\x16\x7D\x76\x77" + 0xA431FFF8).s);
	if (hNtdll == NULL)	return -11;

	PVOID	pNewDbgUiRemoteBreakin = NewDbgUiRemoteBreakin;

	BYTE	Buffer[5] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };

	pDbgUiRemoteBreakin = GetProcAddress(hNtdll, /*DbgUiRemoteBreakin*/XorStr<0x0D, 19, 0x168077FA>("\x49\x6C\x68\x45\x78\x40\x76\x79\x7A\x62\x72\x5A\x6B\x7F\x7A\x77\x74\x70" + 0x168077FA).s);

	if (!pDbgUiRemoteBreakin) return -12;
	else
	{
		//4byte
		*(LONG *)(Buffer + 1) = MAKE_JUMP_OFFSET(pDbgUiRemoteBreakin, pNewDbgUiRemoteBreakin);   //Offset 계산 

		VirtualProtect(pDbgUiRemoteBreakin, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

		//DbgUiRemoteBreakin 함수에 overwrite
		RtlMoveMemory(pDbgUiRemoteBreakin, Buffer, 5);

		//권한 복구, 
		VirtualProtect(pDbgUiRemoteBreakin, 5, oldProtect, &oldProtect);
	}

	return 0;
}

void WINAPI NewDbgUiRemoteBreakin(void)
{
	Sleep(1000);
	TimedMessageBox(NULL, /*Error code: 0x00A\nA Problem occured!*/XorStr<0x62, 37, 0xA3420754>("\x27\x11\x16\x0A\x14\x47\x0B\x06\x0E\x0E\x56\x4D\x5E\x17\x40\x41\x33\x79\x35\x55\x26\x05\x17\x1B\x16\x1E\x11\x5D\x11\x1C\xE3\xF4\xF0\xE6\xE0\xA4" + 0xA3420754).s, /*Stockholm Syndrome Productions LLC*/XorStr<0xA6, 24, 0xF1395363>("\x53\x74\x6F\x63\x6B\x68\x6F\x6C\x6D\x20\x53\x79\x6E\x64\x72\x6F\x6D\x65\x20\x50\x72\x6F\x64\x75\x63\x74\x69\x6F\x6E\x73\x20\x4C\x4C\x43" + 0xF1395363).s, MB_OK | MB_ICONWARNING, 5000);
	Sleep(5000);
	PostQuitMessage(0);
	exit(0);
	ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
	TerminateThread(GetCurrentThread(), 0);
	orgzwterm(GetCurrentProcess(), 0);

	hWinsta = LoadLibrary(TEXT("winsta.dll"));

	WINSTATERMINATEPROC WinStationTerminateProcess = (WINSTATERMINATEPROC)GetProcAddress(hWinsta, /*WinStationTerminateProcess*/XorStr<0xC8, 27, 0x1FFA0781>("\x9F\xA0\xA4\x98\xB8\xAC\xBA\xA6\xBF\xBF\x86\xB6\xA6\xB8\xBF\xB9\xB9\xAD\xBF\x8B\xAE\xB2\xBD\xBA\x93\x92" + 0x1FFA0781).s);

	WinStationTerminateProcess(NULL, GetCurrentProcessId(), DBG_TERMINATE_PROCESS);

	ExitProcess(DBG_TERMINATE_PROCESS);
}

unsigned __stdcall DebuggingCheck()
{
	/*
	while(1)
	{
	if(IsDebuggerPresent())
	printf("DeDe\n");
	Sleep(1000);
	}
	*/
#ifdef _DEBUG
	r3dOutToLog("Debugging Check started!\n");
#endif
	int result = 0;
	result = Anti_Debugging();
	if (result == -11); //printf("Anti_Debug_GetModuleHandle_ntdll Fail!! \n");
	else if (result == -12); //printf("Anti_Debug_GetProcAddress_DbgUiRemoteBreakin Fail!\n");
	else if (result == -13); //printf("Anti_Debug_LoadLibrary_winsta.dll !!\n");
	else //printf("Install_ANTI_Debugging ... OK!! \n");
		// r3dOutToLog("Debugging Check reporting for Duty!\n");
		return 0;
}


void AntiDebugStuff()
{
#ifdef _DEBUG
	r3dOutToLog("Anti Debug Stuff started!\n");
#endif

again:
		AntiDebug();
		olly_crash();
		anti_dump();
		ProtectProcess();
		// r3dOutToLog("Anti Debug Stuff reporting for Duty!\n");
		Sleep(2000);
		goto again;
}

//		:: Anti Debug Stops here
