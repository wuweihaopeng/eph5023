#ifndef _XILINX_IO_H_
#define _XILINX_IO_H_


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "malloc.h"

#include <windows.h>
#include <setupapi.h>
#include <initguid.H>
#include <stdint.h>
#include <strsafe.h>
#include "io_util.h"
#define XILINX_IO_SUCCESS (0)
#define XILINX_IO_ERROR (-1)
#define PCI_PATH_LEN (256)
typedef LARGE_INTEGER BusAddress;
#define XILINX_USER 0
#define XILINX_H2C_0 1
#define MAX_DEV (10)

#define BOARD_ID_LEN 5
#define SUBBOARD_ID_LEN 4
#define SUBBOARD_ID_POS 16

#define NBIT_MASK(n) ((1LL << (n)) - 1)

#define PRINT_PERMISSON 1

typedef struct
{
    // board_io;
    HANDLE device_h2c;
    HANDLE device_c2h;
    HANDLE device_u;
    HANDLE device_bypass;
  
} xilinx_io;
       
xilinx_io *xilinx_io_init(uint32_t device_num);
int xilinx_io_destroy(xilinx_io *io);
int xilinx_out8(xilinx_io *io, uint64_t offset, uint8_t val);
int xilinx_out8_bypass(xilinx_io *io, uint64_t offset, uint8_t val);
int xilinx_out32(xilinx_io *io, uint64_t offset, uint32_t val32);
int xilinx_block_out32(xilinx_io *io, uint64_t offset, uint32_t length, uint32_t *buf32);
int xilinx_in32(xilinx_io *io, uint64_t offset, uint32_t *val32);
int xilinx_block_in32(xilinx_io *io, uint64_t offset, uint32_t length, uint32_t *buf32);
int xilinx_out32_bypass(xilinx_io *io, uint64_t offset, uint32_t val32);
int xilinx_block_out32_bypass(xilinx_io *io, uint64_t offset, uint32_t length, uint32_t *buf32);
int xilinx_block_out8_bypass(xilinx_io * io, uint64_t offset, uint32_t length, uint8_t * buf8);
int xilinx_in32_bypass(xilinx_io *io, uint64_t offset, uint32_t *val32);
int xilinx_block_in32_bypass(xilinx_io *io, uint64_t offset, uint32_t length, uint32_t *buf32);


#endif
