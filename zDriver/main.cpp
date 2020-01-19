#include "main.h"

DRIVER_INITIALIZE DriverEntry;
#pragma alloc_text(INIT, DriverEntry)

//This is a .asm file
/* 
.code
DispatchHook proc
	add rsp, 8h
	mov rax, 0DEADBEEFCAFEBEEFh
	jmp rax
DispatchHook endp

end
*/

extern "C" void DispatchHook();

PDRIVER_DISPATCH ACPIOriginalDispatch = 0;

#define IOCTL_DISK_BASE                 FILE_DEVICE_DISK
#define IOCTL_DISK_GET_DRIVE_GEOMETRY   CTL_CODE(IOCTL_DISK_BASE, 0x0000, METHOD_BUFFERED, FILE_ANY_ACCESS)


NTSTATUS CustomDispatch(PDEVICE_OBJECT device, PIRP irp)
{
	PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);

	//Here you can do your custom calls

	if (ioc->Parameters.DeviceIoControl.IoControlCode == IOCTL_DISK_GET_DRIVE_GEOMETRY)
	{
		const char* Buffer = (const char*)irp->AssociatedIrp.SystemBuffer;

		if (Buffer)
		{
			Printf("[Hook] Got Call with Control code: %X %s\n", ioc->Parameters.DeviceIoControl.IoControlCode, Buffer);
		}
	}
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




