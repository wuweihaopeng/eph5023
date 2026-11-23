#include <wchar.h>
#include "pci_io.h"
#include "locale.h"

int self_mode; // 0为mock模式，1为pcie

DEFINE_GUID(GUID_DEVINTERFACE_XDMA,
            0x74c7e4a9, 0x6d5d, 0x4a70, 0xbc, 0x0d, 0x20, 0x69, 0x1d, 0xff, 0x9e, 0x9d);
DEVPROPKEY Device_LocationPath = {0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 37};


static int enum_device(GUID guid, BoardNum* dev_num){
    //获取本地计算机设备信息集
    HDEVINFO device_info = SetupDiGetClassDevs((LPGUID)&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (device_info == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "GetDevices INVALID_HANDLE_VALUE\n");
        exit(-1);
    }
     //枚举设备
    SP_DEVINFO_DATA device_info_data;
    device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
    unsigned long device_info_index = 0;
    unsigned long digital = 0;
    
    char dig_id[] = "VEN_10EE&DEV_7011&SUBSYS_000710EE";    // 由FPGA PCIE配置界面获取ID
    
    while (SetupDiEnumDeviceInfo(device_info, device_info_index, &device_info_data)){
        
        SP_DEVICE_INTERFACE_DATA device_bus_interface;
        device_bus_interface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        char path[MAX_PATH + 1] = "";
        size_t length = sizeof(path);
        if (SetupDiEnumDeviceInterfaces(device_info, NULL, &guid, device_info_index, &device_bus_interface))
        {

            // get required buffer size
            ULONG detailLength = 0;
            if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_bus_interface, NULL, 0, &detailLength, NULL) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            {
                fprintf(stderr, "SetupDiGetDeviceInterfaceDetail - get length failed\n");
            }

            // allocate space for device bus_interface detail
            PSP_DEVICE_INTERFACE_DETAIL_DATA dev_detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, detailLength);
            if (!dev_detail)
            {
                fprintf(stderr, "HeapAlloc failed\n");
            }
            dev_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            // get device bus_interface detail
            if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_bus_interface, dev_detail, detailLength, NULL, NULL))
            {
                fprintf(stderr, "SetupDiGetDeviceInterfaceDetail - get detail failed\n");
                HeapFree(GetProcessHeap(), 0, dev_detail);
            }

            strncpy(path, dev_detail->DevicePath, length);
            HeapFree(GetProcessHeap(), 0, dev_detail);
        }
        
        ++device_info_index;
        if (strstr(path, dig_id) != NULL) digital++;
    }
    dev_num->dev_num = device_info_index;
    dev_num->digital_num = digital;
    SetupDiDestroyDeviceInfoList(device_info);
    return PCI_IO_SUCCESS;
}

int pci_out32(xilinx_io* board_io ,uint64_t offset, uint32_t val32)
{ 
    if (self_mode == 0){
        printf ("mock_out32: 0x%I64x  data: 0x%8x\n", offset, val32);
        return -1;
    }
    xilinx_out32_bypass(board_io, offset, val32);
    return 0;
}

int pci_in32(xilinx_io *board_io, uint64_t offset, uint32_t *val32)
{   
    if (self_mode == 0){
        printf ("mock_in32 address: 0x%I64x  data: 0x%8x\n", offset, *val32);
        return -1;
    }
    xilinx_in32_bypass(board_io, offset, val32);
    return 0;
}

int pci_block_out32(xilinx_io* board_io, uint64_t offset, uint32_t length, uint32_t *buf32)
{
    if (self_mode == 0){
        printf ("mock_block_out32 offset: 0x%I64x, len: %d, buf: 0x%I32x\n", offset, length, &buf32);
        return -1;
    }
    xilinx_block_out32_bypass(board_io, offset, length, buf32);
    return 0;
}

int pci_block_in32(xilinx_io* board_io, uint64_t offset, uint32_t length, uint32_t *buf32)
{   
    if (self_mode == 0){
        printf ("mock_block_in32 offset: 0x%I64x, len: %d, buf: 0x%I32x\n", offset, length, &buf32);
        return -1;
    }
    xilinx_block_in32_bypass(board_io, offset, length, buf32);
    return 0;
}

// 获取板卡数量
int pci_get_dev_nums(){
    BoardNum dev_nums;
    enum_device(GUID_DEVINTERFACE_XDMA, &dev_nums);
    return dev_nums.dev_num;
}

PciIO *pci_io_init(uint32_t mode, uint32_t index)
{
    self_mode = mode;
    PciIO *self = (PciIO *)malloc(sizeof(PciIO));
    
    enum_device(GUID_DEVINTERFACE_XDMA, &self->dev_nums);
    BoardNum* board = &self->dev_nums;
    if (PRINT_PERMISSON){
        printf("board_num: %d\n", board->dev_num);
    }
    
    //初始化系统内所有设备io
    if (index < board->dev_num) {
        self->sys_io[index] = xilinx_io_init(index);
    }
    
    return self;
}



int pci_io_destroy(PciIO* io, uint32_t cardnum)
{
    if (io == NULL) {
        return -1;
    }

    PciIO* self = (PciIO*)io;
    BoardNum* board = &self->dev_nums;

    // 检查板卡索引是否合法
    if (cardnum >= board->dev_num) {
        return -1;
    }

    // 断开指定板卡
    xilinx_io* temp = self->sys_io[cardnum];
    if (temp != NULL) {
        xilinx_io_destroy(temp);
        self->sys_io[cardnum] = NULL;
    }

    free(self);

    return 0;
}