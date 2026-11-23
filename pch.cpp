#include "pch.h"
#include <cmath>

// 全局IO对象
PciIO* pci_io;
uint32_t sample_freq = 200;   // (200, 100, 50, 20, 10)
uint32_t ta = 100;    // 采样时间
uint32_t ty = 10;    // 预采样时间

// ********************** 内部函数 **********************
// 参数校验方法
bool attr_assert(int attr, int low_limit, int high_limit){
	if (attr > high_limit) {
		return false;
	}
	if (attr < low_limit) {
		return false;
	}
	return true;
}

// ********************** 测试接口 **********************
// 测试函数
void _stdcall test_call() {
	printf("hello world\n");
}

// 测试IO
void _stdcall test_io() {
	uint64_t addr_user = 0xfff0;
	uint32_t read_data;

    xilinx_io* board_io = pci_io->sys_io[0];

	pci_in32(board_io, addr_user, &read_data);
	pci_out32(board_io, addr_user, 0x11223344);
	pci_in32(board_io, addr_user, &read_data);
}

// 测试DMA
void _stdcall test_DMA() {

    xilinx_io* board_io = pci_io->sys_io[0];
	uint64_t addr = 0x0;

	unsigned int length = 32;//数据长度
	uint32_t* buf = (uint32_t*)calloc(length, sizeof(uint32_t));
	uint32_t* readbuf = (uint32_t*)calloc(length, sizeof(uint32_t));

	unsigned int cnt = 0;
	unsigned int wrong_num = 0;
	//读写测试
	for (int i = 0; i < 10; i++) {//读写次数
		for (int k = 0; k < length; k++) {
			buf[k] = i;
		}
		pci_block_out32(board_io, addr, length, buf);
		//Sleep(1);
		cnt++;
		pci_block_in32(board_io, addr, length, readbuf);
		for (int j = 0; j < length; j++) {
			if (readbuf[j] != buf[j]) {
				wrong_num++;
				break;
			}
		}
		printf("test cnt:%u  wrong num::%u\n", cnt, wrong_num);
	}

	free(buf);
	free(readbuf);
}

// 测试xilinx写
void _stdcall Eph5023_out32(uint32_t cardnum, uint32_t addr, uint32_t data){
    uint64_t addr_user = addr;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, data);
}

// 测试xilinx读
uint32_t _stdcall Eph5023_in32(uint32_t cardnum, uint32_t addr){
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    uint64_t addr_user = addr;
    pci_in32(board_io, addr_user, &read_data);
    return read_data;
}

// ********************** 模块句柄获取 **********************
// 1.获取甄别模块的张数,如果有4张，则num为4，num最大不超过10
int32_t _stdcall Eph5023_GetDeviceNum(uint32_t* num){
    *num = pci_get_dev_nums();
    //*num = 1;
    return 1;
}

// 2.连接模块
int32_t _stdcall Eph5023_Connect(uint32_t index, uint32_t *cardnum){
    uint32_t mode = 1;
    pci_io = pci_io_init(mode,index);
    *cardnum = index;

    switch (pci_io->dev_nums.dev_num)
    {
    case 0:
        printf("Device initialization failed\n");
        break;
    default:
        printf("Device initialization successful\n");
        break;
    }
    return 1;
}

// 3.断开模块
void _stdcall Eph5023_Close(uint32_t cardnum) {
    pci_io_destroy(pci_io , cardnum);
	printf("close device");
}

//4.读取板卡ID
int32_t _stdcall Eph5023_ReadDeviceID(uint32_t cardnum, uint32_t* deviceID) {
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    pci_in32(board_io, 0x0000, &read_data);
    *deviceID = read_data;
    return 1;
}
// ********************** 模块参数设置 **********************

// 1. 长有效时间 (1~10 s)
int32_t _stdcall Eph5023_Set_LRMS_Calc_Time(uint32_t cardnum, uint32_t LpSrms_CalcTime){
    if (!attr_assert(LpSrms_CalcTime, 1, 10)) return 0;
    uint64_t addr_user = 0x0C0C;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, LpSrms_CalcTime);
    return 1;
}

// 2. 短有效时间 (1~10 ms)
int32_t _stdcall Eph5023_Set_SRMS_Calc_Time(uint32_t cardnum, uint32_t LpSrms_CalcTime) {
    if (!attr_assert(LpSrms_CalcTime, 1, 10)) return 0;
    uint64_t addr_user = 0x0C08;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, LpSrms_CalcTime);
    return 1;
}

// 3. 峰顶因子 (2~16 单位：倍)
int32_t _stdcall Eph5023_Set_LPPeak_K(uint32_t cardnum, uint32_t peak_k) {
    if (!attr_assert(peak_k, 2, 16)) return 0;
    uint64_t addr_user = 0x0C14;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, peak_k);
    return 1;
}

// 4. 预采样时间 (10~30ms)
int32_t _stdcall Eph5023_LPEvent_BackTime(uint32_t cardnum, uint32_t Lpbacktime){
    if (!attr_assert(Lpbacktime, 10, 30)) return 0;
    ty = Lpbacktime;
    uint64_t addr_user = 0x1004;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, Lpbacktime);
    return 1;
}

// 5. 采样频率 (0=200kHz (默认), 1=100kHz, 2=50kHz, 3=20kHz, 4=10k)
int32_t _stdcall Eph5023_SetSpFreq(uint32_t cardnum, uint32_t SampFreq){
    if (!attr_assert(SampFreq, 0, 4)) return 0;
    if (SampFreq == 0){
        sample_freq = 200;
    } else if(SampFreq == 1){
        sample_freq = 100;
    } else if(SampFreq == 2){
        sample_freq = 50;
    } else if(SampFreq == 3){
        sample_freq = 20;
    } else if(SampFreq == 4){
        sample_freq = 10;
    }
    uint64_t addr_user = 0x0400;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, SampFreq);
    return 1;
}

// 6. 采样时间 (60~100ms)
int32_t _stdcall Eph5023_Set_LPDataTime(uint32_t cardnum, uint32_t LpEventTime){
    if (!attr_assert(LpEventTime, 60, 100)) return 0;
    ta = LpEventTime;
    uint64_t addr_user = 0x1000;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, LpEventTime);
    return 1;
}

// 7. 突发波形有效时间 (10~30ms)
int32_t _stdcall Eph5023_Set_LPEvent_ValidTime(uint32_t cardnum, uint32_t LPvalidtime){
    if (!attr_assert(LPvalidtime, 10, 30)) return 0;
    uint64_t addr_user = 0x0C10;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, LPvalidtime);
    return 1;
}

// 8. 事件报警时间 (10~100s)
int32_t _stdcall Eph5023_Set_LPEvent_WarningTime(uint32_t cardnum, uint32_t warntime){
    if (!attr_assert(warntime, 10, 100)) return 0;
    uint64_t addr_user = 0x0C20;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, warntime);
    return 1;
}

// 9. 突发波形间隔时间 (1000~2000 ms)
int32_t Eph5023_Set_LPEvent_IntervalTime(uint32_t cardnum, uint32_t LpintervalTime){
    if (!attr_assert(LpintervalTime, 1000, 2000)) return 0;
    uint64_t addr_user = 0x1008;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, LpintervalTime);
    return 1;
}

// 10. 累计因子 (1~10)
int32_t Eph5023_Set_LPEvent_WarningNum(uint32_t cardnum, uint32_t warnnum){
    if (!attr_assert(warnnum, 1, 10)) return 0;
    uint64_t addr_user = 0x0C24;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, warnnum);
    return 1;
}

// 11. 背景噪声 (0 ~ 100mV)
int32_t Eph5023_Set_BackGroundNoise(uint32_t cardnum, uint32_t bgNoise){
    // if (!attr_assert(bgNoise, 0, 100)) return 0;
    uint64_t addr_user = 0x0C18;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, bgNoise);
    return 1;
}
int32_t Eph5023_Set_SelInConfig(uint32_t cardnum, uint32_t ailse) {
    // if (!attr_assert(bgNoise, 0, 100)) return 0;
    uint64_t addr_user = 0x1C00;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, ailse);
    return 1;
}

int32_t Eph5023_Set_SelOutConfig(uint32_t cardnum, uint32_t ailse) {
    // if (!attr_assert(bgNoise, 0, 100)) return 0;
    uint64_t addr_user = 0x1C04;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, ailse);
    return 1;
}
// 12. 通道突发波形触发状态的清除 (0/1，0不清除/1清除)
int32_t Eph5023_ClearChEventStatus(uint32_t cardnum){
    uint64_t addr_user = 0x0008;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, 1);
    return 1;
}

// 13. 采样启停 (按位表示CH0~CH3，范围0~15。0/1，对应停止/启动)
int32_t Eph5023_StartStop(uint32_t cardnum, uint16_t ifenable){
    if (!attr_assert(ifenable, 0, 15)) return 0;
    uint64_t addr_user= 0x0800;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, addr_user, ifenable);   
    return 1;
}

/*
// 14. 通道切除 通道复核 干触点输出
    1、Chnum=0时：
    flag：bit[0:3] 依次对应 0到3通道 通道切除, 1通道切除 0通道打开。
    2、Chnum=4时：
    flag bit[1]：1通道复核，0通道不复核。当干触点输出区间事件报警状态信息时才有效。
    flag bit[0]：0干触点输出区间事件报警状态信息；1干触点输出通道突发波形触发状态信息。
*/
int32_t Eph5023_OpenCutCh(uint32_t cardnum, uint16_t chnum, uint16_t flag){
    if (!attr_assert(chnum, 0, 8)) return 0;
    uint64_t addr_user;
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    if (chnum < 4){ // 通道切除
        addr_user = 0x0804;
        pci_out32(board_io, addr_user, flag);
    } else if(chnum == 4){  // 触点输出, 通道复核
        addr_user = 0x0C1C;
        pci_out32(board_io, addr_user, flag);
    }
    return 1;
}

// ********************** 参数读取 **********************
// 1.获取所有参数配置
int32_t Eph5023_GetConfigStatus(uint32_t cardnum, uint32_t *status){
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    pci_in32(board_io, 0x0C0C, &read_data);
    status[0] = read_data;  // 1. 长有效时间 (1~10 s)
    pci_in32(board_io, 0x0C08, &read_data);
    status[1] = read_data;  // 2. 短有效时间 (1~10 ms)
    pci_in32(board_io, 0x0C14, &read_data);
    status[2] = read_data;  // 3. 峰顶因子 (2~16 单位：倍)
    pci_in32(board_io, 0x1004, &read_data);
    status[3] = read_data;  // 4. 预采样时间 (10~30ms)
    pci_in32(board_io, 0x0400, &read_data);
    status[4] = read_data;  // 5. 采样频率 (0=200kHz (默认), 1=100kHz, 2=50kHz, 3=20kHz, 4=10k)
    pci_in32(board_io, 0x1000, &read_data);
    status[5] = read_data;  // 6. 采样时间 (60~100ms)
    pci_in32(board_io, 0x0C10, &read_data);
    status[6] = read_data;  // 7. 突发波形有效时间 (10~30ms)
    pci_in32(board_io, 0x0C20, &read_data);
    status[7] = read_data;  // 8. 事件报警时间 (10~100s)
    pci_in32(board_io, 0x1008, &read_data);
    status[8] = read_data;  // 9. 突发波形间隔时间 (1000~2000 ms)
    pci_in32(board_io, 0x0C24, &read_data);
    status[9] = read_data;  // 10. 累计因子 (1~10)
    pci_in32(board_io, 0x0C18, &read_data);
    status[10] = read_data;  // 11. 背景噪声 (0 ~ 100mV)
    pci_in32(board_io, 0x0008, &read_data);
    status[11] = read_data;  // 12. 通道突发波形触发状态的清除 (0/1，0不清除/1清除)
    pci_in32(board_io, 0x0800, &read_data);
    status[12] = read_data;  // 13. 采样启停 (按位表示CH0~CH3，范围0~15。0/1，对应停止/启动)
    pci_in32(board_io, 0x0804, &read_data);
    status[13] = read_data;  // 14. 通道切除
    pci_in32(board_io, 0x0C14, &read_data);
    status[14] = read_data;  // 15. 触点输出, 通道复核
    return 1;
}

// 2.获取通道突发波形触发状态
/* chStatus：只有一个32位数据，但是仅使用了低四位xxx---xxx1111，最右边1位为1表示第1个通道有突发波形，
为0表示第1个通道无突发波形依次递推；依次递推，右起第4位为1表示第4个通道有突发波形，为0表示第4个通道无突发波形。 */
int32_t Eph5023_GetChEventStatus(uint32_t cardnum, uint32_t *chStatus){
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    pci_in32(board_io, 0x1800, &read_data);
    *chStatus = read_data & 0xF;
    return 1;
}

int32_t Eph5023_GetText1(uint32_t cardnum, uint32_t* data) {
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    pci_in32(board_io, 0x1c04, &read_data);
    *data = read_data;
    return 1;
}
int32_t Eph5023_GetText2(uint32_t cardnum, uint32_t* data) {
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    pci_in32(board_io, 0x1c00, &read_data);
    *data = read_data;
    return 1;
}
// 3.获得通道的长有效值。（Lrms/32768 单位：V。）
int32_t Eph5023_GetLRMS(uint32_t cardnum, uint16_t chnum, uint32_t *Lrms){
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    pci_in32(board_io, 0x1808 + chnum * 4, &read_data);
    *Lrms = read_data;
    return 1;
}


// ********************** 数据读取 **********************
// 1.读取数据结束的地址：模块句柄、存放数据结束地址的指针。
int32_t Eph5023_GetLpEventEndAddr(uint32_t cardnum, uint32_t *lpendaddr){
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    pci_in32(board_io, 0x1804, &read_data);
    *lpendaddr = read_data;  
    return 1;
} 

/**
 * @brief 计算DDR起始地址（8字节对齐 + 正确环形缓冲回绕）
 * @param ori_addr  原始结束地址
 * @param offset    偏移量
 * @return 环形缓冲区内正确的起始地址
 */
uint32_t calc_ddr_start_addr(uint32_t ori_addr, uint32_t offset) {
    const uint32_t DDR_MAX_ADDR = 536870911U;
    const uint32_t DDR_SIZE = DDR_MAX_ADDR + 1;  // 512MB

    // 1. 8字节对齐
    uint32_t aligned_addr = ori_addr & 0xFFFFFFF8;

    // 2. 计算环形地址（正确处理负数回绕）
    int64_t temp_addr = (int64_t)aligned_addr - offset;
    int64_t mod_addr = temp_addr % DDR_SIZE;

    // 正确处理负数环形地址
    if (mod_addr < 0) {
        mod_addr += DDR_SIZE;
    }

    return (uint32_t)mod_addr;
}

// 2.获取触发波形数据
int32_t Eph5023_GetBlock3(uint32_t cardnum, int32_t points, int32_t* data_buf, uint32_t endaddr) {
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    Sleep(100); // 100ms
    const uint32_t DDR_MAX_ADDR = 536870911U;

    // 1. 计算偏移
    uint32_t ty_addr = ty * sample_freq * 8;

    // 2. 读结束地址
    endaddr = 0;
    uint64_t addr_user = 0x1C10;
    
    pci_in32(board_io, addr_user, &endaddr);

    // 3. 计算环形起始地址
    uint32_t start_addr = calc_ddr_start_addr(endaddr, ty_addr);

    // ==============================================
    // 核心修复：环形缓冲区跨边界自动分两次读取
    // ==============================================
    uint32_t remain_len = points;          // 剩余要读的长度
    uint32_t current_addr = start_addr;    // 当前读地址
    uint32_t dest_offset = 0;              // 目标缓冲区偏移


    // 第一段：从 current_addr 读到缓冲区末尾
    uint32_t len1 = DDR_MAX_ADDR - current_addr + 1;
    if (len1 > remain_len) {
        len1 = remain_len;
    }

    if (len1 > 0) {
        pci_block_in32(
            board_io,
            current_addr,
            len1,
            (uint32_t*)(data_buf + dest_offset)
        );

        remain_len -= len1;
        dest_offset += len1;
        current_addr = 0; // 第二段从0开始读
    }

    // 第二段：剩余长度从地址0开始读
    if (remain_len > 0) {
        pci_block_in32(
            board_io,
            current_addr,
            remain_len,
            (uint32_t*)(data_buf + dest_offset)
        );
    }

    pci_out32(board_io, 0x8, 1);

    return 1;
}
int32_t Eph5023_GetBlock1(uint32_t cardnum, int32_t points, int32_t* data_buf, uint32_t endaddr) {
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    Sleep(100); // 100ms
    const uint32_t DDR_MAX_ADDR = 536870911U;

    // 1. 计算偏移
    uint32_t ty_addr = ty * sample_freq * 8;

    // 2. 读结束地址
    endaddr = 0;
    uint64_t addr_user = 0x1804;

    pci_in32(board_io, addr_user, &endaddr);

    // 3. 计算环形起始地址
    uint32_t start_addr = calc_ddr_start_addr(endaddr, ty_addr);

    // ==============================================
    // 核心修复：环形缓冲区跨边界自动分两次读取
    // ==============================================
    uint32_t remain_len = points;          // 剩余要读的长度
    uint32_t current_addr = start_addr;    // 当前读地址
    uint32_t dest_offset = 0;              // 目标缓冲区偏移


    // 第一段：从 current_addr 读到缓冲区末尾
    uint32_t len1 = DDR_MAX_ADDR - current_addr + 1;
    if (len1 > remain_len) {
        len1 = remain_len;
    }

    if (len1 > 0) {
        pci_block_in32(
            board_io,
            current_addr,
            len1,
            (uint32_t*)(data_buf + dest_offset)
        );

        remain_len -= len1;
        dest_offset += len1;
        current_addr = 0; // 第二段从0开始读
    }

    // 第二段：剩余长度从地址0开始读
    if (remain_len > 0) {
        pci_block_in32(
            board_io,
            current_addr,
            remain_len,
            (uint32_t*)(data_buf + dest_offset)
        );
    }

    pci_out32(board_io, 0x8, 1);

    return 1;
}
int32_t Eph5023_GetBlock2(uint32_t cardnum, int32_t points, int32_t* data_buf, uint32_t endaddr) {
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    Sleep(100); // 100ms
    const uint32_t DDR_MAX_ADDR = 536870911U;

    // 1. 计算偏移
    uint32_t ty_addr = ty * sample_freq * 8;

    // 2. 读结束地址
    endaddr = 0;
    uint64_t addr_user = 0x1804;

    pci_in32(board_io, addr_user, &endaddr);

    // 3. 计算环形起始地址
    uint32_t start_addr = calc_ddr_start_addr(endaddr, ty_addr);

    // ==============================================
    // 核心修复：环形缓冲区跨边界自动分两次读取
    // ==============================================
    uint32_t remain_len = points;          // 剩余要读的长度
    uint32_t current_addr = start_addr;    // 当前读地址
    uint32_t dest_offset = 0;              // 目标缓冲区偏移


    // 第一段：从 current_addr 读到缓冲区末尾
    uint32_t len1 = DDR_MAX_ADDR - current_addr + 1;
    if (len1 > remain_len) {
        len1 = remain_len;
    }

    if (len1 > 0) {
        pci_block_in32(
            board_io,
            current_addr,
            len1,
            (uint32_t*)(data_buf + dest_offset)
        );

        remain_len -= len1;
        dest_offset += len1;
        current_addr = 0; // 第二段从0开始读
    }

    // 第二段：剩余长度从地址0开始读
    if (remain_len > 0) {
        pci_block_in32(
            board_io,
            current_addr,
            remain_len,
            (uint32_t*)(data_buf + dest_offset)
        );
    }

    pci_out32(board_io, 0x8, 1);

    return 1;
}
// 3.获取实时波形数据
int32_t Eph5023_GetRealTimeBlock(uint32_t cardnum, int32_t points, int32_t *data_buf){
    xilinx_io *board_io = pci_io->sys_io[cardnum];
    Sleep(100); // 100ms
    const uint32_t DDR_MAX_ADDR = 536870911U;

    // 1. 计算偏移
    uint32_t ta_addr = points * 4;

    // 2. 读结束地址
    uint32_t endaddr = 0;
    pci_in32(board_io, 0x181c, &endaddr);
       
    // 3. 计算环形起始地址
    uint32_t start_addr = calc_ddr_start_addr(endaddr, ta_addr);

    // ==============================================
    // 核心修复：环形缓冲区跨边界自动分两次读取
    // ==============================================
    uint32_t remain_len = points;          // 剩余要读的长度
    uint32_t current_addr = start_addr;    // 当前读地址
    uint32_t dest_offset = 0;              // 目标缓冲区偏移


    // 第一段：从 current_addr 读到缓冲区末尾
    uint32_t len1 = DDR_MAX_ADDR - current_addr + 1;
    if (len1 > remain_len) {
        len1 = remain_len;
    }

    if (len1 > 0) {
        pci_block_in32(
            board_io,
            current_addr,
            len1,
            (uint32_t *)(data_buf + dest_offset)
        );

        remain_len -= len1;
        dest_offset += len1;
        current_addr = 0; // 第二段从0开始读
    }

    // 第二段：剩余长度从地址0开始读
    if (remain_len > 0) {
        pci_block_in32(
            board_io,
            current_addr,
            remain_len,
            (uint32_t *)(data_buf + dest_offset)
        );
    }

    return 1;
}
//4.获取采样启停状态
int32_t Eph5023_GetSampleStatus(uint32_t cardnum, uint16_t* status)
{
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    pci_in32(board_io, 0x0800, &read_data);
    *status = read_data;
    return 1;  
}

//5.设置参数使能
int32_t Eph5023_Commandconfirm(uint32_t cardnum)      
{
    return 1;
}
//获取外部触发状态
int32_t Eph5023_GetExternalTigger(uint32_t cardnum, uint32_t* status)
{
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    uint32_t read_data = 0;
    pci_in32(board_io, 0x1C0C, &read_data);
    *status = read_data;
    return 1;
}
//强制触发
int32_t Eph5023_ForceTigger(uint32_t cardnum)
{
    xilinx_io* board_io = pci_io->sys_io[cardnum];
    pci_out32(board_io, 0x1C08, 1);
    return 1;
}