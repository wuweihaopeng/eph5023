#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "malloc.h"
#include <SetupAPI.h>
#include <strsafe.h>
#include <stdint.h>

typedef LARGE_INTEGER BusAddress;
#define PCI_USER  0
#define PCI_H2C_0  1   

typedef struct _PhysicalIO PhysicalIO;

typedef struct {
	BOOL verbose;
	HANDLE device_h2c;
	HANDLE device_c2h;
	HANDLE device_u;
	char* file;
	BYTE* data;
	LARGE_INTEGER address;
	DWORD size;
	size_t alignment;
	BOOL binary;
} PciOptions;

struct _PhysicalIO
{
	PciOptions* io;
};