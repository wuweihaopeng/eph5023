#include "xilinx_io.h"
#include "locale.h"



DEFINE_GUID(GUID_DEVINTERFACE_XDMA,
            0x74c7e4a9, 0x6d5d, 0x4a70, 0xbc, 0x0d, 0x20, 0x69, 0x1d, 0xff, 0x9e, 0x9d);

DEVPROPKEY Device_LocationPaths = {0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 37};
DEVPROPKEY Device_InstanceID = {0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 256};

// static void sort_path(wchar_t *map[], int len);
static void sort_path(char map[][MAX_PATH + 1], int len);
static int find_dev_index(HDEVINFO device_info, wchar_t *path);
static int get_dev_base_path(GUID guid, HDEVINFO device_info, char *devpath, size_t len_devpath, wchar_t *pci_path);
//初始化设备
// static int init_device(GUID guid, int index, char *dev_path, size_t length){
//     //获取本地计算机设备信息集
//     HDEVINFO device_info = SetupDiGetClassDevs((LPGUID)&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
//     if (device_info == INVALID_HANDLE_VALUE)
//     {
//         fprintf(stderr, "GetDevices INVALID_HANDLE_VALUE\n");
//         exit(-1);
//     }
//     //枚举设备
//     SP_DEVINFO_DATA device_info_data;
//     device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
//     unsigned long device_info_index = 0;
//     static wchar_t  *path_map[PCI_PATH_LEN];
//     while (SetupDiEnumDeviceInfo(device_info, device_info_index, &device_info_data)){
       
//         DWORD Type = DEVPROP_TYPE_STRING;
//         DWORD len = 0;
//         wchar_t* path = NULL;
//         if (SetupDiGetDevicePropertyW(device_info, &device_info_data, &Device_LocationPaths, &Type, NULL, 0, &len, 0) != FALSE){
//            printf("get len error!");
//         }
//         path = malloc(len);
//         if (SetupDiGetDevicePropertyW(device_info, &device_info_data, &Device_LocationPaths, &Type, (PBYTE)path, len, NULL, 0) == FALSE){
//            printf("get path error!");
//         }
//         path_map[device_info_index] = path;
//         ++device_info_index;
//     }
//     sort_path(path_map, device_info_index);
//     get_dev_base_path(GUID_DEVINTERFACE_XDMA, device_info, dev_path, length, path_map[index]);
//     return 0;
// }
static int init_device(GUID guid, int index, char *dev_path, size_t length){
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
    char path_map[MAX_DEV][MAX_PATH + 1];
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

            strncpy(path_map[device_info_index] , dev_detail->DevicePath, length);
            // printf("%s\n", path_map[device_info_index]);
            device_info_index++;
            HeapFree(GetProcessHeap(), 0, dev_detail);   
        }
       
    }
    SetupDiDestroyDeviceInfoList(device_info);
    sort_path(path_map, device_info_index);
    strncpy(dev_path, path_map[index], sizeof(path_map[index]));
    return XILINX_IO_SUCCESS;
}

static int get_dev_base_path(GUID guid, HDEVINFO device_info, char *devpath, size_t len_devpath, wchar_t *pci_path){
    int ret = XILINX_IO_ERROR;
    //获取路径
    SP_DEVICE_INTERFACE_DATA device_bus_interface;
    device_bus_interface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    int index = find_dev_index(device_info, pci_path);
    if (index==-1){
        return ret;
    }
    // enumerate through devices
    if (SetupDiEnumDeviceInterfaces(device_info, NULL, &guid, index, &device_bus_interface))
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
        strncpy(devpath, dev_detail->DevicePath, len_devpath);
        HeapFree(GetProcessHeap(), 0, dev_detail);
        ret = XILINX_IO_SUCCESS;
    }

    SetupDiDestroyDeviceInfoList(device_info);

    return ret;
}



// static void sort_path(wchar_t *map[], int len){
   
//     for (int i=0; i<len; ++i){
//         for (int j=0; j<len-i-1; ++j){
//             if (wcscmp(map[j], map[j+1]) > 0 ){
//                 wchar_t *tmp = map[j];
//                 map[j] = map[j+1];
//                 map[j+1] = tmp;
//             }
//         }
//     }
//     // for (int i=0; i<len; ++i){
//     //     printf("%ls\n", map[i]);
//     // }
// }

static void sort_path(char map[][MAX_PATH + 1], int len){
    for (int i=0; i<len; ++i){
        for (int j=0; j<len-i-1; ++j){
            if (strcmp(map[j], map[j+1]) > 0 ){
                char tmp[MAX_PATH+1];
                strncpy(tmp, map[j], sizeof(map[j]));
                strncpy(map[j], map[j+1], sizeof(map[j+1]));
                strncpy(map[j+1], tmp, sizeof(tmp));
            }
        }
    }
}

static int find_dev_index(HDEVINFO device_info, wchar_t *pci_path){
    
    SP_DEVINFO_DATA device_info_data;
    device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
    unsigned long device_info_index = 0;

    while (SetupDiEnumDeviceInfo(device_info, device_info_index, &device_info_data)){
        
        DWORD Type = DEVPROP_TYPE_STRING;
        DWORD len = 0;
        wchar_t* path = NULL;

        if (SetupDiGetDevicePropertyW(device_info, &device_info_data, &Device_LocationPaths, &Type, NULL, 0, &len, 0) != FALSE){
           printf("get len error!");
        }
        path = malloc(len);
        if (SetupDiGetDevicePropertyW(device_info, &device_info_data, &Device_LocationPaths, &Type, (PBYTE)path, len, NULL, 0) == FALSE){
           printf("get path error!");
        }
        // printf("%ls\n %ls\n ", path, pci_path);
        if (wcscmp(path, pci_path) == 0){
            free(path);
            return device_info_index;
        }
        ++device_info_index;
    }

    return -1;
}

static int isDBoard (uint32_t offset) {
    
    int ret = 0;
    uint32_t baseAddr = 0xa000;
    uint32_t tempAddr = 0;
    if((offset & 0xff00) == 0x7200)
        ret = 1;
    uint32_t addr15_10 = offset & 0xfc00;
    uint32_t addr9_7 = offset & 0x380;
    uint32_t addr6_2 = offset & 0x7c;
    for(int i=0; i<16; i++) {
        tempAddr = baseAddr + i * 0x400;
        if(addr15_10 == tempAddr) {
            if(addr9_7 != 0x280) {
                if(addr9_7 == 0x80) {
                    if(addr6_2 <= 0x18)
                        ret = 1;
                }
                else
                    ret = 1;
            }
                
        }       
    }
    return ret;
}
int xilinx_out8(xilinx_io *io, uint64_t offset, uint8_t val) {
    xilinx_io * self = (xilinx_io *) io;
    BusAddress addr;

    int board_id;
    uint64_t off;
    uint32_t size = 1;

    address_resolve (offset, board_id, off);
    off = off & 0xffff;
    uint32_t offdata = (uint32_t) off;
    int flag = isDBoard (offdata);

    if (flag == 1) {
        // 0x0004写地址
        addr.QuadPart = 0x0004;

        HANDLE device = self->device_u;
        if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
        if (!WriteFile (device, &offdata, size, &size, NULL)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }

        // 0x0008写数据
        addr.QuadPart = 0x0008;

        if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
        if (!WriteFile (device, &val, size, &size, NULL)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }

        if (PRINT_PERMISSON){
            printf ("xilinx_dboard_out8: %I64x  data: %8x\n", off, val);
        }
        
    } else {
        addr.QuadPart = off;


        HANDLE device = self->device_u;
        if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
        if (!WriteFile (device, &val, size, &size, NULL)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }

        if (PRINT_PERMISSON){
            printf ("xilinx_out8 address: %I64x  data: %8x\n", off, val);
        }
       
    }


}

int xilinx_out8_bypass (xilinx_io * io, uint64_t offset, uint8_t val8) {
    xilinx_io * self = (xilinx_io *) io;
    BusAddress addr;
    uint32_t size = 1;

    addr.QuadPart = offset;

    HANDLE device = self->device_bypass;
    if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (!WriteFile (device, &val8, size, &size, NULL)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (PRINT_PERMISSON){
        printf ("xilinx_board_out8: %I64x  data: %8x\n", offset, val8);
    }
        
    return 0;
}

#define OUT32_DATA_SIZE (4)
int xilinx_out32 (xilinx_io * io, uint64_t offset, uint32_t val32) {
    xilinx_io * self = (xilinx_io *) io;
    BusAddress addr;
    int board_id;
    uint64_t off;
    uint32_t size = OUT32_DATA_SIZE;

    address_resolve (offset, board_id, off);
    off = off & 0xffff;
    uint32_t offdata = (uint32_t) off;
    int flag = isDBoard (offdata);

    if (flag == 1) {
        // 0x0004写地址
        addr.QuadPart = 0x0004;

        HANDLE device = self->device_u;
        if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
        if (!WriteFile (device, &offdata, size, &size, NULL)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }

        // 0x0008写数据
        addr.QuadPart = 0x0008;

        if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
        if (!WriteFile (device, &val32, size, &size, NULL)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }

        if (PRINT_PERMISSON){
            printf ("xilinx_dboard_out32: %I64x  data: %8x\n", off, val32);
        }
        
    } else {
        addr.QuadPart = off;


        HANDLE device = self->device_u;
        if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
        if (!WriteFile (device, &val32, size, &size, NULL)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }

        if (PRINT_PERMISSON){
            printf ("xilinx_dboard_out32: %I64x  data: %8x\n", off, val32);
        }
        
    }

    return 0;
}

int xilinx_out32_bypass (xilinx_io * io, uint64_t offset, uint32_t val32) {
    xilinx_io * self = (xilinx_io *) io;
    BusAddress addr;
    uint32_t size = OUT32_DATA_SIZE;

    addr.QuadPart = offset;

    HANDLE device = self->device_bypass;
    if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (!WriteFile (device, &val32, size, &size, NULL)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (PRINT_PERMISSON){
        printf ("xilinx_board_out32: %I64x  data: %8x\n", offset, val32);
    }
        
    return 0;
}

int xilinx_block_out32 (xilinx_io * io, uint64_t offset, uint32_t length, uint32_t * buf32) {
    xilinx_io * self = (xilinx_io *) io;
    BusAddress addr;
    int board_id;
    uint64_t off;

    if (PRINT_PERMISSON){
        printf ("xilinx_block_out offset:%I64x\n", offset);
    }
   
    address_resolve (offset, board_id, off);
    off = off & 0xffff;
    addr.QuadPart = off;
    uint32_t block_out_len = length * OUT32_DATA_SIZE;

    HANDLE device = self->device_h2c;
    if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (!WriteFile (device, buf32, block_out_len, &block_out_len, NULL)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }

    if (PRINT_PERMISSON){
        printf ("xilinx_move_out32 offset: %I64x, len: %d, buf: %I32x\n", off, length, &buf32);
        for (int i = 0; i < length; i++)
            printf ("%x  ", buf32[i]);
        printf ("\n");
    }

    return 0;
}

int xilinx_block_out32_bypass(xilinx_io * io, uint64_t offset, uint32_t length, uint32_t * buf32) {
    xilinx_io * self = (xilinx_io *) io;

    BusAddress addr;
    addr.QuadPart = offset;

    uint32_t block_out_len = length * OUT32_DATA_SIZE;

    HANDLE device = self->device_h2c;
    if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (!WriteFile (device, buf32, block_out_len, &block_out_len, NULL)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }

    if (PRINT_PERMISSON){
        printf ("xilinx_move_out32 offset: %I64x, len: %d, buf: %I32x\n", offset, length, &buf32);
        for (int i = 0; i < length; i++)
            printf ("%x  ", buf32[i]);
        printf ("\n");
    }

    return 0;

}

int xilinx_block_out8_bypass(xilinx_io * io, uint64_t offset, uint32_t length, uint8_t * buf8) {
    xilinx_io * self = (xilinx_io *) io;

    BusAddress addr;
    addr.QuadPart = offset;

    uint32_t block_out_len = length;
    HANDLE device = self->device_bypass;


    if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (!WriteFile (device, buf8, block_out_len, &block_out_len, NULL)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }

    if (PRINT_PERMISSON){
        printf ("xilinx_move_out8 offset: %I64x, len: %d, buf: %I32x\n", offset, length, &buf8);
        for (int i = 0; i < length; i++)
            printf ("%x  ", buf8[i]);
        printf ("\n");
    }

    return 0;

}

#define IN32_DATA_SIZE (4)
int xilinx_in32 (xilinx_io * io, uint64_t offset, uint32_t * val32) {
    xilinx_io * self = (xilinx_io *) io;
    BusAddress addr;
    int board_id;
    uint64_t off;
    uint32_t size = IN32_DATA_SIZE;

    address_resolve (offset, board_id, off);

    off = off & 0xffff;
    uint32_t offdata = (uint32_t) off;
    int flag = isDBoard (offdata);

    if (flag == 1) {
        addr.QuadPart = 0x000c;


        HANDLE device = self->device_u;
        if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
        if (!WriteFile (device, &offdata, size, &size, NULL)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }

        // usleep (1000);

        if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
        if (!ReadFile (device, val32, size, &size, NULL)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
    } else {
        addr.QuadPart = off;
        HANDLE device = self->device_u;
        if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }

        if (!ReadFile (device, val32, size, &size, NULL)) {
            fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
            xilinx_io_destroy (io);
            return -1;
        }
    }
    
    if (PRINT_PERMISSON){
        if (flag == 1){
            printf ("xilinx_in32 dboard: %I64x  data: %8x\n", off, *val32);
        }else{
            printf ("xilinx_in32 address: %I64x  data: %8x\n", off, *val32);
        }
    }
    
    return 0;
}

int xilinx_in32_bypass (xilinx_io * io, uint64_t offset, uint32_t * val32) {
    xilinx_io * self = (xilinx_io *) io;
    BusAddress addr;

    uint32_t size = IN32_DATA_SIZE;

    addr.QuadPart = offset;
    HANDLE device = self->device_bypass;
    if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (!ReadFile (device, val32, size, &size, NULL)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    
    if (PRINT_PERMISSON){
        printf ("xilinx_in32 address: %I64x  data: %8x\n", offset, *val32);
    }
    
    return 0;
}

int xilinx_block_in32 (xilinx_io * io, uint64_t offset, uint32_t length, uint32_t * buf32) {
    xilinx_io * self = (xilinx_io *) io;
    BusAddress addr;
    int board_id;
    uint64_t off;

    address_resolve (offset, board_id, off);
    addr.QuadPart = off;
    off = off & 0xffff;
    uint32_t size = IN32_DATA_SIZE * length;

    HANDLE device = self->device_c2h;
    if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }

    if (!ReadFile (device, buf32, size, &size, NULL)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (PRINT_PERMISSON){
        printf ("xilinx_move_in32 offset: %I64x, len: %d, buf: %I32x\n", off, length, &buf32);
        for (int i = 0; i < length; i++)
            printf ("0x%x  ", buf32[i]);
        printf ("\n");
    }
    return 0;
}

int xilinx_block_in32_bypass (xilinx_io * io, uint64_t offset, uint32_t length, uint32_t * buf32) {
    xilinx_io * self = (xilinx_io *) io;
    BusAddress addr;
    
    addr.QuadPart = offset;
    uint32_t size = IN32_DATA_SIZE * length;
    HANDLE device = self->device_c2h;

    if (INVALID_SET_FILE_POINTER == SetFilePointerEx (device, addr, NULL, FILE_BEGIN)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }

    if (!ReadFile (device, buf32, size, &size, NULL)) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        xilinx_io_destroy (io);
        return -1;
    }
    if (PRINT_PERMISSON){
        printf ("xilinx_move_in32 offset: %I64x, len: %d, buf: %I32x\n", offset, length, &buf32);
        // for (int i = 0; i < length; i++)
        //     printf ("0x%x  ", buf32[i]);
        // printf ("\n");
    }
    return 0;
}

xilinx_io *xilinx_io_init(uint32_t device_index)
{
    xilinx_io *self = (xilinx_io *)malloc(sizeof(xilinx_io));
    // self->busio = bus_io;
    char device_base_path[MAX_PATH + 1] = "";
    init_device(GUID_DEVINTERFACE_XDMA, device_index, device_base_path, sizeof(device_base_path));
    
    char device_path_u[MAX_PATH + 1] = "";
   
    // strcpy_s(device_path_u, sizeof device_path_u, device_base_path);
    // strcat_s(device_path_u, sizeof device_path_u, "\\");
    // strcat_s(device_path_u, sizeof device_path_u, "user");
    // // printf("%s\n", device_base_path);
    // HANDLE device = CreateFile(device_path_u, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    // if (device == INVALID_HANDLE_VALUE)
    // {
    //     fprintf(stderr, "win32 wrong: %ld\n", GetLastError());
    //     free(self);
    //     return NULL;
    // }
    // // printf("%s\n", device_path_u);
    // self->device_u = device;

    char device_path_w[MAX_PATH + 1] = "";
    strcpy_s (device_path_w, sizeof device_path_w, device_base_path);
    strcat_s (device_path_w, sizeof device_path_w, "\\");
    strcat_s (device_path_w, sizeof device_path_w, "h2c_0");
    // open device file
    // printf("%s\n", device_path_w);
    HANDLE device = CreateFile (device_path_w, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (device == INVALID_HANDLE_VALUE) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        free (self);
        return NULL;
    }

    self->device_h2c = device;

    char device_path_r[MAX_PATH + 1] = "";
    strcpy_s (device_path_r, sizeof device_path_r, device_base_path);
    strcat_s (device_path_r, sizeof device_path_r, "\\");
    strcat_s (device_path_r, sizeof device_path_r, "c2h_0");
    // open device file
    // printf("%s\n", device_path_r);
    device = CreateFile (device_path_r, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (device == INVALID_HANDLE_VALUE) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        free (self);
        return NULL;
    }

    self->device_c2h = device;



    char device_path_bypass[MAX_PATH + 1] = "";
    strcpy_s (device_path_bypass, sizeof device_path_bypass, device_base_path);
    strcat_s (device_path_bypass, sizeof device_path_bypass, "\\");
    strcat_s (device_path_bypass, sizeof device_path_bypass, "bypass");
    // open device file
    // printf("%s\n", device_path_w);
    device = CreateFile (device_path_bypass, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (device == INVALID_HANDLE_VALUE) {
        fprintf (stderr, "win32 wrong: %ld\n", GetLastError ());
        free (self);
        return NULL;
    }

    self->device_bypass = device;
    
    return self;



}

int xilinx_io_destroy(xilinx_io *io)
{
    xilinx_io *self = (xilinx_io *)io;

    CloseHandle(self->device_u);
    CloseHandle(self->device_h2c);
    CloseHandle(self->device_c2h);

    free(self);

    return 0;
}
