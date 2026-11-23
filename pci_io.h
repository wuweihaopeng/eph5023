     #ifndef _PCI_IO_H_
#define _PCI_IO_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "malloc.h"
#include <stdint.h>


#include <windows.h>
#include <setupapi.h>
#include <initguid.H>
#include "xilinx_io.h"
#include <strsafe.h>
#include "framework.h"


#define PCI_IO_SUCCESS (0)
#define PCI_IO_ERROR (-1)
#define PCI_DEV_MAX_NUM (32)

typedef LARGE_INTEGER BusAddress;
typedef struct{
    int dev_num;
    int digital_num;
} BoardNum;

typedef struct
{
    // PciSysIO;
    BoardNum dev_nums;
    xilinx_io* sys_io[PCI_DEV_MAX_NUM];
   
} PciIO;

#ifdef __cplusplus
extern "C" {
#endif

PciIO *pci_io_init(uint32_t mode, uint32_t index);
int pci_io_destroy(PciIO* io, uint32_t cardnum);
int pci_out32(xilinx_io*io, uint64_t offset, uint32_t val32);
int pci_block_out32(xilinx_io*io, uint64_t offset, uint32_t length, uint32_t *buf32);
int pci_in32(xilinx_io*io, uint64_t offset, uint32_t *val32);
int pci_block_in32(xilinx_io*io, uint64_t offset, uint32_t length, uint32_t *buf32);
int pci_get_dev_nums();

#ifdef __cplusplus
}
#endif
#endif
