#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include "MvCameraControl.h"
#include <time.h>
#include <string>
#include <conio.h>
#include <direct.h>
using namespace std;

bool g_bExit = false;
char mytime[64] = {'a'};

/*
void printTime()
{
    time_t timep;
    time(&timep);
    printf("time():%ld\n", timep);
}
*/

void getTime()
{
    time_t timep;
    time(&timep);
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    //sprintf(mytime, "%d-%02d-%02d-%02d%02d%02d%03d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
    sprintf(mytime, "capture_%d%02d%02d%ld", sys.wYear, sys.wMonth, sys.wDay, timep);
    printf("time: %s", mytime);
    mkdir(mytime);
}

int startShowPic()
{
    //WinExec("adb connect 192.168.51.203:5555", SW_HIDE);
    WinExec("adb -s 192.168.51.203:5555 shell rm /sdcard/Android/data/com.roborock.sensordemo/cache/records/*", SW_HIDE);
    WinExec("adb -s 192.168.51.203:5555 shell setprop record 1", SW_HIDE);
    WinExec("adb -s 192.168.51.203:5555 shell am start -n com.roborock.showpic/.MainActivity", SW_HIDE);
    return 0;
}

int stopShowPic()
{
    WinExec("adb -s 192.168.51.203:5555 shell am force-stop com.roborock.showpic", SW_HIDE);
    char cmd[128] = {};
    sprintf(cmd, "adb -s 192.168.51.203:5555 pull /sdcard/Android/data/com.roborock.sensordemo/cache/records %s/tofs", mytime);
    WinExec(cmd, SW_HIDE);
    //WinExec("adb -s 192.168.51.203 pull /sdcard/Android/data/com.roborock.sensordemo/cache/records tofs", SW_HIDE);
    return 0;
}

// ch:等待按键输入 | en:Wait for key press
void WaitForKeyPress(void)
{
    while(!_kbhit())
    {
        Sleep(10);
    }
    _getch();
}

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
        printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
    }
    else
    {
        printf("Not support.\n");
    }

    return true;
}

int saveImage(void* pUser, MV_SAVE_IAMGE_TYPE enSaveImageType, MV_FRAME_OUT_INFO_EX&    m_stImageInfo, unsigned char* m_pSaveImageBuf, int count)
{
    MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
    memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));

    if (m_pSaveImageBuf == NULL || m_stImageInfo.enPixelType == 0)
    {
        return MV_E_NODATA;
    }

    stSaveFileParam.enImageType = enSaveImageType; // ch:需要保存的图像类型 | en:Image format to save
    stSaveFileParam.enPixelType = m_stImageInfo.enPixelType;  // ch:相机对应的像素格式 | en:Camera pixel type
    stSaveFileParam.nWidth = m_stImageInfo.nWidth;         // ch:相机对应的宽 | en:Width
    stSaveFileParam.nHeight = m_stImageInfo.nHeight;          // ch:相机对应的高 | en:Height
    stSaveFileParam.nDataLen = m_stImageInfo.nFrameLen;
    stSaveFileParam.pData = m_pSaveImageBuf;
    stSaveFileParam.iMethodValue = 0;

    // ch:jpg图像质量范围为(50-99], png图像质量范围为[0-9] | en:jpg image nQuality range is (50-99], png image nQuality range is [0-9]
    if (MV_Image_Bmp == stSaveFileParam.enImageType)
    {
        //sprintf_s(stSaveFileParam.pImagePath, 256, "Image_w%d_h%d_fn%03d.bmp", stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
        sprintf_s(stSaveFileParam.pImagePath, 256, "%s/graycode_%02d.bmp",mytime, count);
    }
    else if (MV_Image_Jpeg == stSaveFileParam.enImageType)
    {
        stSaveFileParam.nQuality = 80;
        sprintf_s(stSaveFileParam.pImagePath, 256, "Image_w%d_h%d_fn%03d.jpg", stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
    }
    else if (MV_Image_Tif == stSaveFileParam.enImageType)
    {
        sprintf_s(stSaveFileParam.pImagePath, 256, "Image_w%d_h%d_fn%03d.tif", stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
    }
    else if (MV_Image_Png == stSaveFileParam.enImageType)
    {
        stSaveFileParam.nQuality = 8;
        sprintf_s(stSaveFileParam.pImagePath, 256, "Image_w%d_h%d_fn%03d.png", stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
    }

    int nRet = MV_CC_SaveImageToFile(pUser, &stSaveFileParam);

    return nRet;
}

static  unsigned int __stdcall WorkThread(void* pUser)
{
    int nRet = MV_OK;
    MV_FRAME_OUT stOutFrame = {0};
    unsigned char* m_pSaveImageBuf = nullptr;
    unsigned int m_nSaveImageBufSize = 0;
    MV_FRAME_OUT_INFO_EX    m_stImageInfo;
    int count = 0;
    while(true)
    {
        if (m_pSaveImageBuf)
        {
            free(m_pSaveImageBuf);
            m_pSaveImageBuf = NULL;
        }

        nRet = MV_CC_GetImageBuffer(pUser, &stOutFrame, 1000);
        if (nRet == MV_OK)
        {
            /*xzy: add my code here*/

            if (NULL == m_pSaveImageBuf || stOutFrame.stFrameInfo.nFrameLen > m_nSaveImageBufSize)
            {
                if (m_pSaveImageBuf)
                {
                    free(m_pSaveImageBuf);
                    m_pSaveImageBuf = NULL;
                }

                m_pSaveImageBuf = (unsigned char*)malloc(sizeof(unsigned char) * stOutFrame.stFrameInfo.nFrameLen);
                if (m_pSaveImageBuf == NULL)
                {
                    return 0;
                }
                m_nSaveImageBufSize = stOutFrame.stFrameInfo.nFrameLen;
            }
            memcpy(m_pSaveImageBuf, stOutFrame.pBufAddr, stOutFrame.stFrameInfo.nFrameLen);
            memcpy(&m_stImageInfo, &(stOutFrame.stFrameInfo), sizeof(MV_FRAME_OUT_INFO_EX));
            saveImage(pUser, MV_Image_Bmp, m_stImageInfo, m_pSaveImageBuf,count);
            /*xzy: end*/
            printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n",
                stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum);

            nRet = MV_CC_FreeImageBuffer(pUser, &stOutFrame);
            if(nRet != MV_OK)
            {
                printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
            }
        }
        else
        {
            printf("Get Image fail! nRet [0x%x]\n", nRet);
        }
        if(g_bExit)
        {
            break;
        }
        count++;
        if (count == 47)
        {
            stopShowPic();
            exit(0);
        }
        Sleep(2030);
    }

    return 0;
}

int main()
{
    getTime();
    int nRet = MV_OK;
    void* handle = NULL;

    do 
    {
        // ch:枚举设备 | en:Enum device
        MV_CC_DEVICE_INFO_LIST stDeviceList;
        memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
        nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
        if (MV_OK != nRet)
        {
            printf("Enum Devices fail! nRet [0x%x]\n", nRet);
            break;
        }

        if (stDeviceList.nDeviceNum > 0)
        {
            for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
            {
                printf("[device %d]:\n", i);
                MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
                if (NULL == pDeviceInfo)
                {
                    break;
                } 
                PrintDeviceInfo(pDeviceInfo);            
            }  
        } 
        else
        {
            printf("Find No Devices!\n");
            break;
        }
        /*
        printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1);
        unsigned int nIndex = 0;
        scanf_s("%d", &nIndex);

        if (nIndex >= stDeviceList.nDeviceNum)
        {
            printf("Input error!\n");
            break;
        }
        */
        unsigned int nIndex = 0;
        printf("press enter to start:");
        char tmp = 0;
        scanf_s("%c", &tmp);

        // ch:选择设备并创建句柄 | en:Select device and create handle
        nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet)
        {
            printf("Create Handle fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:打开设备 | en:Open device
        nRet = MV_CC_OpenDevice(handle);
        if (MV_OK != nRet)
        {
            printf("Open Device fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
        if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
        {
            int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
            if (nPacketSize > 0)
            {
                nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize);
                if(nRet != MV_OK)
                {
                    printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
                }
            }
            else
            {
                printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
            }
        }

        // ch:设置触发模式为off | en:Set trigger mode as off
        nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
        if (MV_OK != nRet)
        {
            printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
            break;
        }
        startShowPic();
        Sleep(1200);
        // ch:开始取流 | en:Start grab image
        nRet = MV_CC_StartGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        unsigned int nThreadID = 0;
        void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID );
        if (NULL == hThreadHandle)
        {
            break;
        }

        printf("Press a key to stop grabbing.\n");
        WaitForKeyPress();

        g_bExit = true;
        Sleep(1000);

        // ch:停止取流 | en:Stop grab image
        nRet = MV_CC_StopGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:关闭设备 | Close device
        nRet = MV_CC_CloseDevice(handle);
        if (MV_OK != nRet)
        {
            printf("ClosDevice fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:销毁句柄 | Destroy handle
        nRet = MV_CC_DestroyHandle(handle);
        if (MV_OK != nRet)
        {
            printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
            break;
        }
    } while (0);


    if (nRet != MV_OK)
    {
        if (handle != NULL)
        {
            MV_CC_DestroyHandle(handle);
            handle = NULL;
        }
    }

    printf("Press a key to exit.\n");
    WaitForKeyPress();

    return 0;
}
