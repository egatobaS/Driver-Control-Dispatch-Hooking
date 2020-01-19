#pragma once
#pragma warning( disable : 4099 )

#include <ntdef.h>
#include <ntifs.h>

#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <wdf.h>
#include <ntdef.h>
#include <ntimage.h>
#include <ntifs.h>
#include <intrin.h>


#include <ntdef.h>
#include <ntifs.h>
#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <wdf.h>
#include <ntdef.h>

#include <ntimage.h>
#include <ntifs.h>
#include <intrin.h>

#include "Utils.h"


DRIVER_INITIALIZE DriverInitialize;
extern "C" DRIVER_INITIALIZE DriverEntry;
#pragma alloc_text(INIT, DriverEntry)


#define Printf(...) DbgPrintEx( DPFLTR_SYSTEM_ID, DPFLTR_ERROR_LEVEL, "[zDriver] " __VA_ARGS__ )
