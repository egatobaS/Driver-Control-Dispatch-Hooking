#include "main.h"

DRIVER_INITIALIZE DriverEntry;
#pragma alloc_text(INIT, DriverEntry)

PDRIVER_DISPATCH ACPIOriginalDispatch = 0;

NTSTATUS CustomDispatch(PDEVICE_OBJECT device, PIRP irp)
{
	PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);

	//Here you can do your custom calls
	Printf("Got Call with Control code: %X\n", ioc->Parameters.DeviceIoControl.IoControlCode);

	return ACPIOriginalDispatch(device, irp);
}

NTSTATUS DriverEntry(_In_  struct _DRIVER_OBJECT* DriverObject, _In_  PUNICODE_STRING RegistryPath)
{
	PDRIVER_OBJECT ACPIDriverObject = nullptr;

	UNICODE_STRING DriverObjectName = RTL_CONSTANT_STRING(L"\\Driver\\ACPI");
	ObReferenceObjectByName(&DriverObjectName, OBJ_CASE_INSENSITIVE, 0, 0, *IoDriverObjectType, KernelMode, 0, (PVOID*)&ACPIDriverObject);

	if (ACPIDriverObject)
	{
		ACPIOriginalDispatch = ACPIDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL];

		/*
			add rsp, 8h //remove the previus return addrss from the stack since we don't care to return to it
			mov rax, 0DEADBEEFCAFEBEEFh
			jmp rax //Jump to our custom dispatcher!
		*/
		ULONG64 DispatchHookAddr = (ULONG64)DispatchHook;

		*(ULONG64*)(DispatchHookAddr + 0x6) = (ULONG64)CustomDispatch;

		ULONG64 TraceMessageHookInst = FindPattern((UINT64)ACPIDriverObject->DriverStart, ACPIDriverObject->DriverSize, (BYTE*)"\xB8\x0C\x00\x00\x00\x44\x0F\xB7\xC8\x8D\x50\x00", "xxxxxxxxxxx?");

		if (TraceMessageHookInst)
		{
			TraceMessageHookInst += 0xC;

			ULONG64 pfnWppTraceMessagePtr = (ULONG64)ResolveRelativeAddress((PVOID)TraceMessageHookInst, 3, 7);

			if (pfnWppTraceMessagePtr)
			{
				*(ULONG64*)(pfnWppTraceMessagePtr) = DispatchHookAddr;

				ACPIDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = (PDRIVER_DISPATCH)TraceMessageHookInst;

				Printf("ACAPI IRP_MJ_DEVICE_CONTROL Hooked!\n");
			}
		}
	}

	return 0;
}




