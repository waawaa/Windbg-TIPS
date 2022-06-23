#include <windows.h>
#include <stdio.h>
#include <winternl.h>
#include <stdlib.h>
#include <ntstatus.h>

#pragma comment(lib, "ntdll.lib")



PPEB locate_PEB()
{
	DWORD returnLength;
	PROCESS_BASIC_INFORMATION information;

	NTSTATUS returnValue = NtQueryInformationProcess((HANDLE)0xFFFFFFFF, ProcessBasicInformation, &information, sizeof(PROCESS_BASIC_INFORMATION), &returnLength);
	if (!NT_SUCCESS(returnValue))
	{
		printf("Error: %lu\n", returnValue);
		return 0;
	}
	return  information.PebBaseAddress;

}


unsigned long get_kernel32_addr(PEB* pebAddress)
{
	PPEB pPeb = pebAddress;
	_PEB_LDR_DATA *LdrData = pebAddress->Ldr; /*Locate _PEB_LDR_DATA*/
	_LDR_DATA_TABLE_ENTRY* DataTableEntry = (_LDR_DATA_TABLE_ENTRY*)LdrData->InMemoryOrderModuleList.Flink; /*_LDR_DATA_TABLE_ENTRY pointed by _PEB_LDR_DATA->InMemoryOrderModuleList.Flink*/
	long* newDataTableEntry = (long*)((_LDR_DATA_TABLE_ENTRY*)DataTableEntry->Reserved1); /*Next DataTableEntry pointed by _LDR_DATA_TABLE_ENTRY->Reserved1 (equal to Flink)*/
	wchar_t* name;
	HMODULE dllbase;
	long* dllbase_addr;
	do
	{
		name = DataTableEntry->FullDllName.Buffer; /*Name of the DLL pointed by this _LDR_DATA_TABLE_ENTRY structure*/
		dllbase_addr = (long*)DataTableEntry+4; /*Workaround, because winternl sucks*/
		dllbase = (HMODULE)*dllbase_addr; /*DllBase*/
		printf("Address DllBase is: 0x%p\n", DataTableEntry);

		DataTableEntry = (_LDR_DATA_TABLE_ENTRY*)*newDataTableEntry; /*Update current _LDR_DATA_TABLE_ENTRY with position of next _LDR_DATA_TABLE_ENTRY*/
		newDataTableEntry = (long*)((_LDR_DATA_TABLE_ENTRY*)DataTableEntry->Reserved1); /*Update next _LDR_DATA_TABLE_ENTRY*/
	} while (wcscmp(name, L"KERNEL32.DLL") != 0);
	return (unsigned long)dllbase;

}

typedef UINT WinExec(
	/*[in]*/ LPCSTR lpCmdLine,
	/*[in]*/ UINT   uCmdShow
);

int call_get_proc_address_api(long* address, long Kernel32Handle) /*address = GetProcAddresss, Kernel32Handle = Address of Kernel32.dll*/
{
	long (__stdcall *get_proc_address)(long, const char*) = (long(__stdcall *)(long, const char*)) ((long)*address); /*get_proc_address as a function pointer to address*/
	WinExec *WinExecA = (WinExec*)get_proc_address(Kernel32Handle, "WinExec"); /*Resolve WinExec*/
	printf("Addr in 0x%x\n", *address);
	if (WinExecA == 0)
	{
		printf("GetLastError is: %lu\n", GetLastError());
	}
	unsigned int final_value = WinExecA("calc.exe", SW_NORMAL);
	return final_value;
}
int main()
{
	PPEB pPeb = locate_PEB();
	
	long kernel32addr = get_kernel32_addr(pPeb);
	LPVOID imageBaseAddress = pPeb->Reserved3[1];
	printf("ImageBaseAddress: 0x%X\n", imageBaseAddress);
	IMAGE_DOS_HEADER  *pDosHeaders = (IMAGE_DOS_HEADER*)imageBaseAddress;
	IMAGE_NT_HEADERS  *ntHeaders = (IMAGE_NT_HEADERS*)(pDosHeaders->e_lfanew + (unsigned long long)imageBaseAddress); /*pDosHeaders-e_lfanew === pDosHeaders+0x03c*/
	IMAGE_OPTIONAL_HEADER32 OptionalHeader = ntHeaders->OptionalHeader; /*ntHeaders->OptionalHeader === ntHeaders+0x018*/
	_IMAGE_IMPORT_DESCRIPTOR* pImportDescriptor = (_IMAGE_IMPORT_DESCRIPTOR*)((unsigned long long)imageBaseAddress + OptionalHeader.DataDirectory[1].VirtualAddress);
	printf("Name is: %s\n", (char*)(pImportDescriptor->Name + (unsigned long long)imageBaseAddress));

	while (strcmp((char*)(pImportDescriptor->Name + (unsigned long long)imageBaseAddress), "KERNEL32.dll") != 0)
	{
		pImportDescriptor = (_IMAGE_IMPORT_DESCRIPTOR*)((byte*)pImportDescriptor + sizeof(_IMAGE_IMPORT_DESCRIPTOR));

		printf("Address is 0x%p\n", pImportDescriptor);

		printf("Name is: %s\n", (char*)(pImportDescriptor->Name + (unsigned long long)imageBaseAddress));

	} 

	//printf("Addresss to debug is: 0x%p\n", pImportDescriptor->OriginalFirstThunk + (unsigned long long)imageBaseAddress);
	long* relativeIAT = (long*)(pImportDescriptor->OriginalFirstThunk + (unsigned long long)imageBaseAddress); /*Store the address in relativeIAT*/
	printf("Data is: %p\n", relativeIAT);

	unsigned long index = 0;
	_IMAGE_IMPORT_BY_NAME* importByName = (_IMAGE_IMPORT_BY_NAME*)(*relativeIAT + (unsigned long)imageBaseAddress); /*Real IAT in importByName*/

	char* name = importByName->Name;
	
	while (strcmp(name, "GetProcAddress") != 0)
	{
		relativeIAT = (long*)((byte*)relativeIAT + sizeof(DWORD));  /*Go to the next part of the ARRAY*/
		/*Cast relativeIAT as a byte, so when we add sizeof(DWORD) we go to the next 4 bytes in memory*/
		/*In C every dword has a 4 byte size, so if  you do DWORD+4 really yo are doing DWORD+0x10, instead, if you want to do*/
		/*DWORD+4 you should do DWORD+1 that is DWORD+*/
		importByName = (_IMAGE_IMPORT_BY_NAME*)(*relativeIAT + (unsigned long)imageBaseAddress);
		name = importByName->Name;
		printf("Data is: %s\n", name);
		index += 1;

	}
	long* relativeIATAddr = (long*)(pImportDescriptor->FirstThunk + (unsigned long long)imageBaseAddress);
	long* addr = relativeIATAddr + index;
	call_get_proc_address_api(addr, kernel32addr);

	return 0;
}
