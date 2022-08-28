#include "Device.h"
#include "../driver/EEPROM.h"
#include "../driver/UART.h"
#include "../utils/Memery.h"

u8 xdata sub_device_list[MAX_DEVICES * SINGLE_DEVICE_DATA_LENGTH];
u8 sub_device_count = 0;


void Device_Init()
{
    EEPROM_read_n(0x0000, &sub_device_count, 1);
    if (sub_device_count > MAX_DEVICES)
    {
        sub_device_count = 0;
    }
    if (sub_device_count > 0)
    {
        EEPROM_read_n(0x0001, sub_device_list, sub_device_count * SINGLE_DEVICE_DATA_LENGTH);
    }
}

void Device_PrintData(){
    u8 i, len;
    LOGB(0xaa);
    LOGB(sub_device_count);
    LOGB(0xbb);
    if (sub_device_count > 0)
    {
        len = sub_device_count * SINGLE_DEVICE_DATA_LENGTH;
        for (i = 0; i < len; i++)
        {
            LOGB(sub_device_list[i]);
        }
    }
    LOGB(0xcc);
}

void Device_ResetAllUserData()
{
    u8 i;
    u8 *ptr;
    for (i = 0; i < sub_device_count; i++)
    {
        ptr = sub_device_list + i * SINGLE_DEVICE_DATA_LENGTH + DEVICE_ID_LENGTH;
        mem_fill(ptr, 0, DEVICE_USER_DATA_LENGTH);
    }
}



s8 Device_IndexOf(u8 *device_id)
{
    u8 i;
    u8 *ptr;
    for (i = 0; i < sub_device_count; i++)
    {
        ptr = sub_device_list + i * SINGLE_DEVICE_DATA_LENGTH;
        if (mem_equal(ptr, device_id, DEVICE_ID_LENGTH))
        {
            return i;
        }
    }
    return -1;
}

u8 Device_Remove(u8 deviceIndex)
{
    u8 *src;
    u8 *dst;
    if (deviceIndex < sub_device_count)
    {
        if (deviceIndex != sub_device_count - 1)
        {
            dst = sub_device_list + deviceIndex * SINGLE_DEVICE_DATA_LENGTH;
            src = dst + SINGLE_DEVICE_DATA_LENGTH;
            mem_copy(dst, src, (sub_device_count - deviceIndex - 1) * SINGLE_DEVICE_DATA_LENGTH);
        }
        sub_device_count--;
        return 1;
    }
    return 0;
}

// s8 Device_Add(u8 *device_id)
// {
//     s8 deviceIndex;
//     u8 *src;
//     u8 *dst;
//     if (sub_device_count < MAX_DEVICES)
//     {
//         deviceIndex = Device_IndexOf(device_id);
//         if (deviceIndex == -1)
//         {
//             dst = sub_device_list + sub_device_count * SINGLE_DEVICE_DATA_LENGTH;
//             mem_copy(dst, device_id, DEVICE_ID_LENGTH);
//             dst += DEVICE_ID_LENGTH;
//             for(deviceIndex=0;deviceIndex<SINGLE_DEVICE_DATA_LENGTH-DEVICE_ID_LENGTH;deviceIndex++)
//             {
//                 *dst = 0;
//                 dst++;
//             }
//             deviceIndex = sub_device_count;
//             sub_device_count++;
//             return deviceIndex;
//         }
//         return -2;
//     }
//     return -1;
// }

s8 Device_Add(u8 *device_id)
{
    u8 index;
    u8 *dst;
    if (sub_device_count < MAX_DEVICES)
    {
        dst = sub_device_list + sub_device_count * SINGLE_DEVICE_DATA_LENGTH;
        mem_copy(dst, device_id, DEVICE_ID_LENGTH);
        dst += DEVICE_ID_LENGTH;
        for (index = 0; index < SINGLE_DEVICE_DATA_LENGTH - DEVICE_ID_LENGTH; index++)
        {
            *dst = 0;
            dst++;
        }
        index = sub_device_count;
        sub_device_count++;
        return index;
    }
    return -1;
}

u8 Device_GetStatus(u8 deviceIndex)
{
    u8 *dst = sub_device_list + deviceIndex * SINGLE_DEVICE_DATA_LENGTH;
    return *(dst + DEVICE_ID_LENGTH);
}

void Device_SetStatus(u8 deviceIndex, u8 status)
{
    u8 *dst = sub_device_list + deviceIndex * SINGLE_DEVICE_DATA_LENGTH;
    *(dst + DEVICE_ID_LENGTH) = status;
}

u8 Device_GetUserData(u8 deviceIndex, u8 column)
{
    u8 *dst = sub_device_list + deviceIndex * SINGLE_DEVICE_DATA_LENGTH;
    return *(dst + DEVICE_ID_LENGTH + column);
}

void Device_SetUserData(u8 deviceIndex, u8 column, u8 dat)
{
    u8 *dst = sub_device_list + deviceIndex * SINGLE_DEVICE_DATA_LENGTH;
    *(dst + DEVICE_ID_LENGTH + column) = dat;
}

u8 *Device_GetUserDataPtr(u8 deviceIndex)
{
    u8 *dst = sub_device_list + deviceIndex * SINGLE_DEVICE_DATA_LENGTH;
    return dst + DEVICE_ID_LENGTH;
}

u8 Device_GetDeviceCount()
{
    return sub_device_count;
}

void Device_Save()
{
    EEPROM_SectorErase(0x0000);
    EEPROM_write_n(0x0000, &sub_device_count, 1);
    EEPROM_write_n(0x0001, sub_device_list, sub_device_count * SINGLE_DEVICE_DATA_LENGTH);
}