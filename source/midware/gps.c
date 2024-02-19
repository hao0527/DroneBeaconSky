#include "gps.h"
#include <string.h>

typedef enum {
	GGA_UTC_TIME = 0,
	GGA_LATITUDE,
	GGA_NS_INDICATOR,
	GGA_LONGITUDE,
	GGA_EW_INDICATOR,
	GGA_POSITION_FIX,
	GGA_SATELLITES_USED,
	GGA_HDOP,
	GGA_MSL_ALTITUDE,
	GGA_UNITS1,
	GGA_GEOIDS_SEPARATION,
	GGA_UNITS2,
	GGA_NUM_MAX,
} MessageGGA_e;

typedef enum {
	RMC_UTC_TIME = 0,
	RMC_STATUS,
	RMC_LATITUDE,
	RMC_NS_INDICATOR,
	RMC_LONGITUDE,
	RMC_EW_INDICATOR,
	RMC_SPEED_OVER_GROUND,
	RMC_COURSE_OVER,
	RMC_UTC_DATE,
	RMC_NUM_MAX,
} MessageRMC_e;

GpsInfo_s g_gpsInfo;

static int hexStr2Int(char *pStr)
{
	int num = 0;
	while (1) {
		if (*pStr >= '0' && *pStr <= '9') {
			num *= 16;
			num += *pStr - '0';
			pStr++;
		} else if (*pStr >= 'a' && *pStr <= 'f') {
			num *= 16;
			num += *pStr - 'a' + 10;
			pStr++;
		} else if (*pStr >= 'A' && *pStr <= 'F') {
			num *= 16;
			num += *pStr - 'A' + 10;
			pStr++;
		} else {
			break;
		}
	}
	return num;
}

static int decStr2Int(char *pStr)
{
	int num = 0;
	char isMinus = 0;
	if (*pStr == '-') {
		isMinus = 1;
		pStr++;
	}
	while (*pStr >= '0' && *pStr <= '9') {
		num *= 10;
		num += *pStr - '0';
		pStr++;
	}
	if (isMinus)
		num = -num;
	return num;
}

static float str2Float(char *pStr)
{
	int intNum = 0;
	float num = 0;
	char isMinus = 0;
	intNum = decStr2Int(pStr);
	if (*pStr == '-') {
		isMinus = 1;
		pStr++;
	}
	while (*pStr >= '0' && *pStr <= '9') {
		pStr++;
	}
	if (*pStr == '.') {
		pStr++;
		if (*pStr >= '0' && *pStr <= '9') {
			num = decStr2Int(pStr);
		}
		while (*pStr >= '0' && *pStr <= '9') {
			num /= 10.f;
			pStr++;
		}
	}
	if (isMinus)
		num = intNum - num;
	else
		num = intNum + num;
	return num;
}

static uint16_t findChar(uint8_t findValue, uint8_t *pStr, uint16_t size)
{
	uint16_t i = 0;
	for (i = 0; i < size; i++, pStr++) {
		if (*pStr == findValue)
			return i;
	}
	return 0xffff;
}

static uint8_t calcXorCheck(uint8_t *pStart, uint8_t *pEnd)
{
	uint8_t ret = 0;
	while (pStart < pEnd) {
		ret ^= *pStart;
		pStart++;
	}
	return ret;
}

static uint16_t gps_getDataSize(uint8_t *pData)
{
	uint16_t size = 0;
	while (*pData) {
		size++;
		pData++;
	}
	return size;
}

static uint8_t gps_parseGGA(uint8_t *pData)
{
	uint16_t offset = 0, size = 0, index = 0;
	MessageGGA_s ggaTmp;
	uint8_t *pXorCheckStart = NULL, *pXorCheckEnd = NULL;
	uint8_t xorCheck = 0;
	size = gps_getDataSize(pData);
	// 判断Message ID
	while (1) {
		offset = findChar('$', pData, size);
		if (offset > size)
			return 1;    // findChar失败
		size -= (offset + 1);
		pData += (offset + 1);
		if (!memcmp(pData + 2, "GGA", 3)) {
			pXorCheckStart = pData;    // '$'的地址+1
			break;
		}
	}
	// 解析数据
	while (1) {
		offset = findChar(',', pData, size);
		if (offset > size)
			return 1;
		size -= (offset + 1);
		pData += (offset + 1);
		switch (index) {
		case GGA_UTC_TIME:
			break;    // 不解析GGA报文的UTC Time
		case GGA_LATITUDE:
			ggaTmp.latitude = str2Float((char *)pData);
			break;
		case GGA_NS_INDICATOR:
			if (*pData == 'N')
				ggaTmp.isNorth = 1;
			else if (*pData == 'S')
				ggaTmp.isNorth = 0;
			else
				ggaTmp.isNorth = -1;
			break;
		case GGA_LONGITUDE:
			ggaTmp.longitude = str2Float((char *)pData);
			break;
		case GGA_EW_INDICATOR:
			if (*pData == 'E')
				ggaTmp.isEast = 1;
			else if (*pData == 'W')
				ggaTmp.isEast = 0;
			else
				ggaTmp.isEast = -1;
			break;
		case GGA_POSITION_FIX:
			ggaTmp.positionFix = decStr2Int((char *)pData);
			break;
		case GGA_SATELLITES_USED:
			ggaTmp.satellitesUsed = decStr2Int((char *)pData);
			break;
		case GGA_HDOP:
			ggaTmp.HDOP = str2Float((char *)pData);
			break;
		case GGA_MSL_ALTITUDE:
			ggaTmp.MSLAltitude = str2Float((char *)pData);
			break;
		case GGA_UNITS1:
			if (*pData != 'M')    // 海拔单位不是米
			{
				ggaTmp.MSLAltitude = 8848.86;    // 返回珠穆朗玛峰高度表示错误
			}
			break;
		case GGA_GEOIDS_SEPARATION:
			ggaTmp.geoidsSeparation = str2Float((char *)pData);
			break;
		case GGA_UNITS2:
			if (*pData != 'M') {
				ggaTmp.geoidsSeparation = 8848.86;
			}
			break;
		default:
			break;
		}
		index++;
		if (index >= GGA_NUM_MAX)
			break;
	}
	// 校验和
	offset = findChar('*', pData, size);
	if (offset > size)
		return 1;
	size -= (offset + 1);
	pData += (offset + 1);
	pXorCheckEnd = pData - 1;    // '*'的地址
	xorCheck = hexStr2Int((char *)pData);
	if (xorCheck == calcXorCheck(pXorCheckStart, pXorCheckEnd)) {
		memcpy(&g_gpsInfo.GGAInfo, &ggaTmp, sizeof(MessageGGA_s));
		return 0;
	} else {
		return 2;    // 校验失败
	}
}

static uint8_t gps_parseRMC(uint8_t *pData)
{
	uint16_t offset = 0, size = 0, index = 0;
	MessageRMC_s rmcTmp;
	uint8_t *pXorCheckStart = NULL, *pXorCheckEnd = NULL;
	uint8_t xorCheck = 0;
	size = gps_getDataSize(pData);
	// 判断Message ID
	while (1) {
		offset = findChar('$', pData, size);
		if (offset > size)
			return 1;    // findChar失败
		size -= (offset + 1);
		pData += (offset + 1);
		if (!memcmp(pData + 2, "RMC", 3)) {
			pXorCheckStart = pData;    // '$'的地址+1
			break;
		}
	}
	// 解析数据
	while (1) {
		offset = findChar(',', pData, size);
		if (offset > size)
			return 1;
		size -= (offset + 1);
		pData += (offset + 1);
		switch (index) {
		case RMC_UTC_TIME:
			rmcTmp.utcTime = decStr2Int((char *)pData);
			break;
		case RMC_STATUS:
			if (*pData == 'A')
				rmcTmp.status = 1;
			else if (*pData == 'V')
				rmcTmp.status = 0;
			else
				rmcTmp.status = -1;
			break;
		case RMC_UTC_DATE:
			rmcTmp.utcDate = decStr2Int((char *)pData);
			break;
		default:
			break;
		}
		index++;
		if (index >= RMC_NUM_MAX)
			break;
	}
	// 校验和
	offset = findChar('*', pData, size);
	if (offset > size)
		return 1;
	size -= (offset + 1);
	pData += (offset + 1);
	pXorCheckEnd = pData - 1;    // '*'的地址
	xorCheck = hexStr2Int((char *)pData);
	if (xorCheck == calcXorCheck(pXorCheckStart, pXorCheckEnd)) {
		memcpy(&g_gpsInfo.RMCInfo, &rmcTmp, sizeof(MessageRMC_s));
		return 0;
	} else {
		return 2;    // 校验失败
	}
}

void gps_main(void)
{
	static uint8_t rc1, rc2;
	if (rc1 != g_gpsInfo.GGABuffer.rollcnt && !g_gpsInfo.GGABuffer.bufferLocked) {
		g_gpsInfo.GGABuffer.bufferLocked = TRUE;
		gps_parseGGA(g_gpsInfo.GGABuffer.buffer);
		g_gpsInfo.GGABuffer.bufferLocked = FALSE;
		rc1 = g_gpsInfo.GGABuffer.rollcnt;
	}
	if (rc2 != g_gpsInfo.RMCBuffer.rollcnt && !g_gpsInfo.RMCBuffer.bufferLocked) {
		g_gpsInfo.RMCBuffer.bufferLocked = TRUE;
		gps_parseRMC(g_gpsInfo.RMCBuffer.buffer);
		g_gpsInfo.RMCBuffer.bufferLocked = FALSE;
		rc2 = g_gpsInfo.RMCBuffer.rollcnt;
	}
}
