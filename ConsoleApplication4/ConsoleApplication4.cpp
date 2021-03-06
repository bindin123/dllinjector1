// ConsoleApplication4.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include "pch.h"
// ConsoleApplication2.cpp: definiuje punkt wejścia dla aplikacji konsolowej.
//

/*#ifdef UNICODE
#define LoadLibraryGeneric  "LoadLibraryW"
#else
#define LoadLibraryGeneric  "LoadLibraryA"
#endif*/

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <thread>
#include <chrono>

using namespace std;

char FileToInject[] = "AC.dll";
const WCHAR* ProcessName = L"ac_client.exe";
typedef HINSTANCE(*fpLoadLibrary)(char*);

bool InjectDLL(DWORD ProcessID);


int main()
{
	DWORD processID = NULL;

	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
	HANDLE hProcSnap;

	while (!processID) {
		system("CLS");
		cout << "Searching for " << ProcessName << endl;
		cout << "Make sure your game is running" << endl;
		hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (Process32First(hProcSnap, &pe32)) {
			do {

				if (!wcscmp(pe32.szExeFile, ProcessName)) {
					processID = pe32.th32ProcessID;
					break;
				}
			} while (Process32Next(hProcSnap, &pe32));

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		}
	}

	while (!InjectDLL(processID)) {
		system("CLS");
		cout << "DLL FAILED TO INJECT! " << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	cout << "DLL INJECTED SUCCESSFULY!" << endl << endl;
	cout << "Closing Injector in 5 seconds" << endl;

	std::this_thread::sleep_for(std::chrono::milliseconds(5000));


	CloseHandle(hProcSnap);
	return 0;

}


bool InjectDLL(DWORD ProcessID) {
	HANDLE hProc;
	LPVOID paramAddr;

	HINSTANCE hDll = LoadLibrary(TEXT("KERNEL32"));

	fpLoadLibrary LoadLibraryAddr = (fpLoadLibrary)GetProcAddress(hDll, "LoadLibraryA");

	hProc = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessID);

	char dllPath[250] = "C:\\ABCD\\";

	strcat_s(dllPath, FileToInject);

	paramAddr = VirtualAllocEx(hProc, 0, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	bool memoryWritten = WriteProcessMemory(hProc, paramAddr, dllPath, strlen(dllPath) + 1, NULL);

	CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, paramAddr, 0, 0);

	CloseHandle(hProc);
	return memoryWritten;
}






/*

#ifdef UNICODE
#define LoadLibraryGeneric  "LoadLibraryW"
#else
#define LoadLibraryGeneric  "LoadLibraryA"
#endif

typedef HINSTANCE (__stdcall *fpLoadLibrary)(TCHAR*);
typedef void* (__stdcall *fpGetProcAddress)(HMODULE, LPCSTR);
typedef void (*fpFunktion)(void);

struct INJECTSTRUCT
{
fpLoadLibrary LoadLibrary;
fpGetProcAddress GetProcAddress;
TCHAR path[255];
char func[255];
};

static DWORD WINAPI threadstart(LPVOID addr)
{
HINSTANCE hDll;
fpFunktion funktion;
INJECTSTRUCT *is = (INJECTSTRUCT*)addr;
hDll = is->LoadLibrary(is->path);
funktion = (fpFunktion)is->GetProcAddress(hDll, is->func);
funktion();
return 0;
}

static void threadend()
{
}

bool EnableDebugPrivilege()
{
HANDLE hThis = GetCurrentProcess();
HANDLE hToken;
OpenProcessToken(hThis, TOKEN_ADJUST_PRIVILEGES, &hToken);
LUID luid;
LookupPrivilegeValue(0, TEXT("seDebugPrivilege"), &luid);
TOKEN_PRIVILEGES priv;
priv.PrivilegeCount = 1;
priv.Privileges[0].Luid = luid;
priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
AdjustTokenPrivileges(hToken, false, &priv, 0, 0, 0);
CloseHandle(hToken);
CloseHandle(hThis);
return true;
}

HANDLE InjectDll(unsigned ProcessId, string DllFile, std::string ExportedFunctionName, stringstream& ErrorStream)
{

INJECTSTRUCT is;
_tcscpy_s(is.path, DllFile.c_str());
strcpy_s(is.func, ExportedFunctionName.c_str());
unsigned funcsize = (unsigned)threadend - (unsigned)threadstart;

EnableDebugPrivilege();

HINSTANCE hDll = LoadLibrary(TEXT("KERNEL32"));
if(hDll == NULL)
{
ErrorStream << TEXT("LoadLibrary failed! Error Code: ") << GetLastError();
return 0;
}

is.LoadLibrary = (fpLoadLibrary)GetProcAddress(hDll, LoadLibraryGeneric);
is.GetProcAddress = (fpGetProcAddress)GetProcAddress(hDll, "GetProcAddress");
if(is.LoadLibrary == NULL || is.GetProcAddress == NULL)
{
ErrorStream << TEXT("GetProcAddress failed! Error Code: ") << GetLastError();
return 0;
}

HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessId);
if(hProc == NULL)
{
ErrorStream << TEXT("OpenProcess failed! Error Code: ") << GetLastError();
return 0;
}

void* start = VirtualAllocEx(hProc, 0, funcsize + sizeof(INJECTSTRUCT), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
if(start == NULL)
{
ErrorStream << TEXT("VirtualAllocEx failed! Error Code: ") << GetLastError();
return 0;
}

if(WriteProcessMemory(hProc, start, (LPVOID)&is, sizeof(INJECTSTRUCT), NULL) == 0)
{
ErrorStream << TEXT("WriteProcessMemory failed! Error Code: ") << GetLastError();
return 0;
}

void* thread = (void*)((unsigned)start + sizeof(INJECTSTRUCT));

if(WriteProcessMemory(hProc, thread, (LPVOID)threadstart, funcsize, NULL) == 0)
{
ErrorStream << TEXT("WriteProcessMemory failed! Error Code: ") << GetLastError();
return 0;
}

HANDLE threadID = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)thread, start, 0, 0);
if(threadID == NULL)
{
ErrorStream << TEXT("CreateRemoteThread failed! Error Code: ") << GetLastError();
return 0;
}

if(CloseHandle(hProc) == 0)
{
ErrorStream << TEXT("CloseHandle failed! Error Code: ") << GetLastError();
return 0;
}

return threadID;
}









// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}












*/

// Uruchomienie programu: Ctrl + F5 lub menu Debugowanie > Uruchom bez debugowania
// Debugowanie programu: F5 lub menu Debugowanie > Rozpocznij debugowanie

// Porady dotyczące rozpoczynania pracy:
//   1. Użyj okna Eksploratora rozwiązań, aby dodać pliki i zarządzać nimi
//   2. Użyj okna programu Team Explorer, aby nawiązać połączenie z kontrolą źródła
//   3. Użyj okna Dane wyjściowe, aby sprawdzić dane wyjściowe kompilacji i inne komunikaty
//   4. Użyj okna Lista błędów, aby zobaczyć błędy
//   5. Wybierz pozycję Projekt > Dodaj nowy element, aby utworzyć nowe pliki kodu, lub wybierz pozycję Projekt > Dodaj istniejący element, aby dodać istniejące pliku kodu do projektu
//   6. Aby w przyszłości ponownie otworzyć ten projekt, przejdź do pozycji Plik > Otwórz > Projekt i wybierz plik sln
