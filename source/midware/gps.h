#ifndef GPS_H_
#define GPS_H_

#include "base_types.h"

#define GPS_BUFFER_NUM 100

typedef struct
{
	float latitude;            // 纬度
	float longitude;           // 经度
	int8_t isNorth;            // 1:North, 0:South, -1:无效
	int8_t isEast;             // 1:East, 0:West, -1:无效
	uint8_t positionFix;       // 0:未定位, 1:非差分定位, 2:差分定位, 3:无效PPS, 6:正在估算
	uint8_t satellitesUsed;    // 卫星数(0~12)
	float HDOP;                // 水平精度
	float MSLAltitude;         // 定位高度
	float geoidsSeparation;    // 地面高度
} MessageGGA_s;

typedef struct
{
	uint32_t utcTime;    // UTC时间
	uint32_t utcDate;    // UTC日期
	int8_t status;       // 1:数据有效, 0:数据无效, -1:异常
} MessageRMC_s;

typedef struct
{
	uint8_t buffer[GPS_BUFFER_NUM];
	boolean_t bufferLocked;
	uint8_t rollcnt;
} MessageBuffer_s;

typedef struct GpsInfo
{
	MessageGGA_s GGAInfo;
	MessageRMC_s RMCInfo;
	MessageBuffer_s GGABuffer;
	MessageBuffer_s RMCBuffer;
} GpsInfo_s;

extern GpsInfo_s g_gpsInfo;

void gps_main(void);

#endif
