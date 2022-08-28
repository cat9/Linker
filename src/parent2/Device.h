#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "../config.h"

#define MAX_DEVICES 4

#define SINGLE_DEVICE_DATA_LENGTH 16
#define DEVICE_ID_LENGTH 7
#define DEVICE_USER_DATA_LENGTH (SINGLE_DEVICE_DATA_LENGTH-DEVICE_ID_LENGTH)

u8 Device_GetDeviceCount();

void Device_Init();

void Device_PrintData();

void Device_ResetAllUserData();

s8 Device_IndexOf(u8 *device_id);

u8 Device_Remove(u8 deviceIndex);

s8 Device_Add(u8 *device_id);

u8 Device_GetStatus(u8 deviceIndex);

void Device_SetStatus(u8 deviceIndex, u8 status);

u8 Device_GetUserData(u8 deviceIndex,u8 column);

void Device_SetUserData(u8 deviceIndex,u8 column, u8 dat);

u8* Device_GetUserDataPtr(u8 deviceIndex);

void Device_Save();

#endif // __DEVICE_H__

