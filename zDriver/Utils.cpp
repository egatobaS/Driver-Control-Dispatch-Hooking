#include "main.h"

BOOLEAN bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return 0;

	return (*szMask) == 0;
}

UINT64 FindPattern(UINT64 dwAddress, UINT64 dwLen, BYTE* bMask, char* szMask)
{
	for (UINT64 i = 0; i < dwLen; i++)
		if (bDataCompare((BYTE*)(dwAddress + i), bMask, szMask))
			return (UINT64)(dwAddress + i);

	return 0;
}

PVOID NTAPI GetKernelProcAddress(LPCWSTR SystemRoutineName)
{
	UNICODE_STRING Name;
	RtlInitUnicodeString(&Name, SystemRoutineName);
	return MmGetSystemRoutineAddress(&Name);
}

ULONG64 GeModuleBase(const char* Findmodule)
{
	ULONG modulesSize = 0;
	NTSTATUS ReturnCode = ZwQuerySystemInformation(SystemModuleInformation, 0, modulesSize, &modulesSize);

	if (!modulesSize)
		return 0;

	PRTL_PROCESS_MODULES ModuleList = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, modulesSize, 'ENON'); // 'ENON'

	ReturnCode = ZwQuerySystemInformation(SystemModuleInformation, ModuleList, modulesSize, &modulesSize);

	if (!NT_SUCCESS(ReturnCode))
		return 0;

	PRTL_PROCESS_MODULE_INFORMATION module = ModuleList->Modules;

	for (ULONG i = 0; i < ModuleList->NumberOfModules; i++)
	{
		if (strstr((char*)module[i].FullPathName, Findmodule))
		{
			if (ModuleList)
				ExFreePoolWithTag(ModuleList, 'ENON');

			return (UINT64)module[i].ImageBase;
		}
	}

	if (ModuleList)
		ExFreePoolWithTag(ModuleList, 'ENON');

	return 0;

}

PVOID ResolveRelativeAddress(PVOID Instruction, ULONG OffsetOffset, ULONG InstructionSize)
{
	ULONG_PTR Instr = (ULONG_PTR)Instruction;
	LONG RipOffset = *(PLONG)(Instr + OffsetOffset);
	PVOID ResolvedAddr = (PVOID)(Instr + InstructionSize + RipOffset);
	return ResolvedAddr;
}
