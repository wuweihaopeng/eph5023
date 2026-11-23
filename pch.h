// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。
#pragma comment(lib, "setupapi.lib")

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "pci_io.h"

// ********************** 测试接口 **********************

// 测试函数,测试DLL调用成功
_declspec(dllexport) void _stdcall test_call();

// 测试xilinx IO读写
_declspec(dllexport) void _stdcall test_io();

// 测试xilinx DMA读写
_declspec(dllexport) void _stdcall test_DMA();

// 测试xilinx写
_declspec(dllexport) void _stdcall Eph5023_out32(uint32_t cardnum, uint32_t addr, uint32_t data);

// 测试xilinx读
_declspec(dllexport) uint32_t _stdcall Eph5023_in32(uint32_t cardnum, uint32_t addr);


// ********************** 模块句柄获取 **********************
// 获取模块的张数
_declspec(dllexport) int32_t _stdcall Eph5023_GetDeviceNum(uint32_t* num);

// 连接模块
_declspec(dllexport) int32_t _stdcall Eph5023_Connect(uint32_t mode);

_declspec(dllexport) int32_t _stdcall Eph5023_ReadDeviceID(uint32_t cardnum, uint32_t* deviceID);
// 断开模块
_declspec(dllexport) void _stdcall Eph5023_Close();


// ********************** 模块参数设置 **********************
// 1. 长有效时间 (1~10 s)
_declspec(dllexport) int32_t _stdcall Eph5023_Set_LRMS_Calc_Time(uint32_t cardnum, uint32_t LpSrms_CalcTime);

// 2. 短有效时间 (1~10 ms)
_declspec(dllexport) int32_t _stdcall Eph5023_Set_SRMS_Calc_Time(uint32_t cardnum, uint32_t LpLrms_CalcTime);

// 3. 峰顶因子 (2~16 单位：倍)
_declspec(dllexport) int32_t _stdcall Eph5023_Set_LPPeak_K(uint32_t cardnum, uint32_t peak_k);

// 4. 预采样时间 (10~30ms)
_declspec(dllexport) int32_t _stdcall Eph5023_LPEvent_BackTime(uint32_t cardnum, uint32_t Lpbacktime);

// 5. 采样频率(0=200kHz (默认), 1=100kHz, 2=50kHz, 3=20kHz, 4=10k)
_declspec(dllexport) int32_t _stdcall Eph5023_SetSpFreq(uint32_t cardnum, uint32_t SampFreq);

// 6. 采样时间 (60~100ms)
_declspec(dllexport) int32_t _stdcall Eph5023_Set_LPDataTime(uint32_t cardnum, uint32_t LpEventTime);

// 7. 突发波形有效时间 (10~30ms)
_declspec(dllexport) int32_t _stdcall Eph5023_Set_LPEvent_ValidTime(uint32_t cardnum, uint32_t LPvalidtime);

// 8. 事件报警时间 (10~100s)
_declspec(dllexport) int32_t _stdcall Eph5023_Set_LPEvent_WarningTime(uint32_t cardnum, uint32_t warntime);

// 9. 突发波形间隔时间 (1000~2000 ms)
_declspec(dllexport) int32_t Eph5023_Set_LPEvent_IntervalTime(uint32_t cardnum, uint32_t LpintervalTime);

// 10. 累计因子 (1~10)
_declspec(dllexport) int32_t Eph5023_Set_LPEvent_WarningNum(uint32_t cardnum, uint32_t warnnum);

// 11. 背景噪声 (0 ~ 100mV)
_declspec(dllexport) int32_t Eph5023_Set_BackGroundNoise(uint32_t cardnum, uint32_t bgNoise);

// 12. 通道突发波形触发状态的清除
_declspec(dllexport) int32_t Eph5023_ClearChEventStatus(uint32_t cardnum);

// 13. 采样启停 (按位表示CH0~CH3，范围0~15。0/1，对应停止/启动)
_declspec(dllexport) int32_t Eph5023_StartStop(uint32_t cardnum, uint16_t ifenable);

// 14. 通道切除 通道复核 干触点输出
_declspec(dllexport) int32_t Eph5023_OpenCutCh(uint32_t cardnum, uint16_t chnum, uint16_t flag);


// ********************** 参数读取 **********************
// 1.获取所有参数配置
_declspec(dllexport) int32_t Eph5023_GetConfigStatus(uint32_t cardnum, uint32_t *status);

// 2.获取通道突发波形触发状态
/* chStatus：只有一个32位数据，但是仅使用了低四位xxx---xxx1111，最右边1位为1表示第1个通道有突发波形，
为0表示第1个通道无突发波形依次递推；依次递推，右起第4位为1表示第4个通道有突发波形，为0表示第4个通道无突发波形。 */
_declspec(dllexport) int32_t Eph5023_GetChEventStatus(uint32_t cardnum, uint32_t *chStatus);

// 3.获得通道的长有效值。（Lrms/3276.8 单位：V。）
_declspec(dllexport) int32_t Eph5023_GetLRMS(uint32_t cardnum, uint16_t chnum, uint32_t *Lrms);


// ********************** 数据读取 **********************
// 1.读取数据结束的地址：模块句柄、存放数据结束地址的指针。
_declspec(dllexport) int32_t Eph5023_GetLpEventEndAddr(uint32_t cardnum, uint32_t *lpendaddr);

// 2.获取触发波形数据
_declspec(dllexport) int32_t Eph5023_GetBlock2(uint32_t cardnum, int32_t points, int32_t* data_buf, uint32_t endaddr);
_declspec(dllexport) int32_t Eph5023_GetBlock1(uint32_t cardnum, int32_t points, int32_t* data_buf, uint32_t endaddr);
_declspec(dllexport) int32_t Eph5023_GetBlock3(uint32_t cardnum, int32_t points, int32_t* data_buf, uint32_t endaddr);
// 3.获取实时波形数据
_declspec(dllexport) int32_t Eph5023_GetRealTimeBlock(uint32_t cardnum, int32_t points, int32_t* data_buf);

_declspec(dllexport) int32_t Eph5023_GetSampleStatus(uint32_t cardnum, uint16_t* status);

_declspec(dllexport) int32_t Eph5023_Set_SelOutConfig(uint32_t cardnum, uint32_t ailse);

_declspec(dllexport) int32_t Eph5023_Set_SelInConfig(uint32_t cardnum, uint32_t ailse);

_declspec(dllexport) int32_t Eph5023_GetText1(uint32_t cardnum, uint32_t* data);
_declspec(dllexport) int32_t Eph5023_GetText2(uint32_t cardnum, uint32_t* data);
_declspec(dllexport) int32_t Eph5023_ForceTigger(uint32_t cardnum);
_declspec(dllexport) int32_t Eph5023_GetExternalTigger(uint32_t cardnum, uint32_t* status);


#endif //PCH_H
