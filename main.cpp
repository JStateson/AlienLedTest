#include "pch.h"
#include "alienfx.h"

#include <windows.h>
#include <SetupAPI.h>
#include <hidsdi.h>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/scoped_ptr.hpp>
#include <hidclass.h>

using namespace winrt;
using namespace Windows::Foundation;

HANDLE hDeviceHandle = NULL;
std::wstring AlienfxDeviceName;
bool AlienfxNew = false;

bool FindDevice(int pVendorId, int pProductId, std::wstring& pDevicePath) {
    GUID guid;
    bool flag = false;
    pDevicePath = L"";
    HidD_GetHidGuid(&guid);
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        //std::cout << "Couldn't get guid";
        return false;
    }
    unsigned int dw = 0;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;

    unsigned int lastError = 0;
    while (!flag)
    {
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, dw, &deviceInterfaceData))
        {
            lastError = GetLastError();
            return false;
        }
        dw++;
        DWORD dwRequiredSize = 0;
        if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, NULL, 0, &dwRequiredSize, NULL)) {
            //std::cout << "Getting the needed buffer size failed";
            return false;
        }
        //std::cout << "Required size is " << dwRequiredSize << std::endl;
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            //std::cout << "Last error is not ERROR_INSUFFICIENT_BUFFER";
            return false;
        }
        boost::scoped_ptr<SP_DEVICE_INTERFACE_DETAIL_DATA> deviceInterfaceDetailData((SP_DEVICE_INTERFACE_DETAIL_DATA*)new char[dwRequiredSize]);
        deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, deviceInterfaceDetailData.get(), dwRequiredSize, NULL, NULL))
        {
            std::wstring devicePath = deviceInterfaceDetailData->DevicePath;
            HANDLE hDevice = CreateFile(devicePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (hDevice != INVALID_HANDLE_VALUE)
            {
                boost::scoped_ptr<HIDD_ATTRIBUTES> attributes(new HIDD_ATTRIBUTES);
                attributes->Size = sizeof(HIDD_ATTRIBUTES);
                if (HidD_GetAttributes(hDevice, attributes.get())) {
                    if (((attributes->VendorID == pVendorId) && (attributes->ProductID == pProductId)))
                    {
                        flag = true;
                        pDevicePath = devicePath;
                    }
                    else {
                        //std::cout << "ProductID and VendorID does not match" << std::endl;
                    }
                }
                else {
                    //std::cout << "Failed to get attributes" << std::endl;
                }
                CloseHandle(hDevice);
            }
            else
            {
                lastError = GetLastError();
                //std::cout << "Failed to open file" << std::endl;
            }
        }
    }
    return flag;
}


bool AlienfxInit() {
#ifdef ALIENFX_DEBUG
    std::cout << "FindDevice" << std::endl;
#endif
    if (FindDevice(0x187c, 0x550, AlienfxDeviceName)) {
        hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDeviceHandle == NULL)
            return false;
        return true;
    }
    if (FindDevice(0x187c, 0x526, AlienfxDeviceName)) {
#ifdef ALIENFX_DEBUG
        std::cout << "Opening new alienfx device" << std::endl;
#endif
        hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (hDeviceHandle == NULL)
            return false;
        AlienfxNew = true;
        return true;
    }
    if (FindDevice(0x187c, 0x514, AlienfxDeviceName)) {
#ifdef ALIENFX_DEBUG
        std::cout << "Opening m11x alienfx device" << std::endl;
#endif
        hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (hDeviceHandle == NULL)
            return false;
        AlienfxNew = true;
        return true;
    }
    if (FindDevice(0x187c, 0x520, AlienfxDeviceName)) {
#ifdef ALIENFX_DEBUG
        std::cout << "Opening M17x R3 alienfx device" << std::endl;
#endif
        hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (hDeviceHandle == NULL)
            return false;
        AlienfxNew = true;
        return true;
    }
    return false;
}

bool AlienfxReinit() {
    CloseHandle(hDeviceHandle);
    hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDeviceHandle == NULL)
        return false;
    return true;
}

void AlienfxDeinit() {
    if (hDeviceHandle != NULL) {
        CloseHandle(hDeviceHandle);
        hDeviceHandle = NULL;
    }
}

bool WriteDevice(byte* pBuffer, size_t pBytesToWrite, size_t& pBytesWritten) {
    if (AlienfxNew) {
        pBuffer[0] = (byte) 0x02;
        return DeviceIoControl(hDeviceHandle, IOCTL_HID_SET_OUTPUT_REPORT, pBuffer, pBytesToWrite, NULL, 0, (DWORD*)&pBytesWritten, NULL);
    }
    return WriteFile(hDeviceHandle, pBuffer, pBytesToWrite, (DWORD*)&pBytesWritten, NULL);
}

bool ReadDevice(byte* pBuffer, size_t pBytesToRead, size_t& pBytesRead) {
    if (AlienfxNew) {
        return DeviceIoControl(hDeviceHandle, IOCTL_HID_GET_INPUT_REPORT, NULL, 0, pBuffer, pBytesToRead, (DWORD*)&pBytesRead, NULL);
    }
    return ReadFile(hDeviceHandle, pBuffer, pBytesToRead, (DWORD*)&pBytesRead, NULL);
}

void AlienfxReset(byte pOptions) {
    size_t BytesWritten;
    byte Buffer[] = { (byte) (byte) 0x00 ,(byte)0x07 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 };
    Buffer[2] = pOptions;
    WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxSetColor(byte pAction, byte pSetId, int pLeds, int pColor) {
    size_t BytesWritten;
    byte Buffer[] = { (byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 ,(byte)(byte) 0x00 };
    Buffer[1] = pAction;
    Buffer[2] = pSetId;
    Buffer[3] = (byte)(pLeds & 0xFF0000) >> 16;
    Buffer[4] = (byte)(pLeds & 0x00FF00) >> 8;
    Buffer[5] = (byte)(pLeds &  0x0000FF);
    Buffer[6] = (byte)(pColor & 0xFF0000) >> 16;
    Buffer[7] = (byte)(pColor &  0x00FF00) >> 8;
    Buffer[8] = (byte)(pColor &  0x0000FF);
    WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxEndLoopBlock() {
    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00 ,(byte)0x04 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 };
    WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxEndTransmitionAndExecute() {
    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00 ,(byte)0x05 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 };
    WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxStoreNextInstruction(byte pStorageId) {
    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00 ,(byte)0x08 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 };
    Buffer[2] = pStorageId;
    WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxEndStorageBlock() {
    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 };
    WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxSaveStorageData() {
    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00 ,(byte)0x09 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 };
    WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxSetSpeed(int pSpeed) {
    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00 ,(byte)0x0E ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 };
    Buffer[2] = (byte)(pSpeed & 0xFF00) >> 8;
    Buffer[3] = (byte)(pSpeed &  0x00FF);
    WriteDevice(Buffer, 9, BytesWritten);
}

byte AlienfxGetDeviceStatus() {
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxGetDeviceStatus";
#endif
    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00 ,(byte)0x06 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 ,(byte) 0x00 };
    WriteDevice(Buffer, 9, BytesWritten);
    if (AlienfxNew)
        Buffer[0] = (byte)0x01;
    ReadDevice(Buffer, 9, BytesWritten);
#ifdef ALIENFX_DEBUG
    for (int i = 0; i < 9; i++) {
        std::cout << " " << std::hex << (int)Buffer[i];
    }
    std::cout << std::endl;
#endif
    if (AlienfxNew) {
        if (Buffer[0] == (byte)0x01)
            return(byte)0x06;
        return Buffer[0];
    }
    else
        return Buffer[1];
}

byte AlienfxWaitForReady() {
    byte status = AlienfxGetDeviceStatus();
    for (int i = 0; i < 5 && status != (byte)ALIENFX_READY; i++) {
        if (status == (byte)ALIENFX_DEVICE_RESET)
            return status;
        Sleep(1);
        status = AlienfxGetDeviceStatus();
    }
    return status;
}

byte AlienfxWaitForBusy() {
    byte status = AlienfxGetDeviceStatus();
    for (int i = 0; i < 5 && status != (byte)ALIENFX_BUSY; i++) {
        if (status == (byte)ALIENFX_DEVICE_RESET)
            return status;
        Sleep(1);
        status = AlienfxGetDeviceStatus();
    }
    return status;
}

/******************************************************************************/
/**   UNIX specific part                                                     **/
/******************************************************************************/
#if defined __unix__  // Unix

#include <iostream>
#include <libusb.h>

libusb_device_handle* hDeviceHandle = NULL;
uint16_t hVID = 0x0;
uint16_t hPID = 0x0;
libusb_context* context;

std::string AlienfxDeviceName;

bool AlienfxNew = false;

void detach(libusb_device_handle* device)
{
    int r = libusb_kernel_driver_active(device, 0);
    if (r == 1)
        r = libusb_detach_kernel_driver(device, 0);

}

/**
 * Test for the presence of a given AlienFX device (vendor id, product id).
 */
bool FindDevice(uint16_t pVendorId, uint16_t pProductId) {
    hDeviceHandle = libusb_open_device_with_vid_pid(
        context,
        pVendorId,
        pProductId);

    if (hDeviceHandle == NULL) return false;
    else
    {
        hVID = pVendorId;
        hPID = pProductId;

        detach(hDeviceHandle);

        int res = libusb_claim_interface(hDeviceHandle, 0);
    }

    return true;
}


/**
 * AlienFX initialization.
 * Tests for a list of known devices.
 */
bool AlienfxInit() {

    libusb_init(&context);
    libusb_set_debug(context, 3);

    if (FindDevice(0x187c, 0x511))
    {
        std::cout << "Area-51 M15x detected" << std::endl;

        if (hDeviceHandle == NULL)
            return false;

        AlienfxNew = true;

        return true;
    }

    if (FindDevice(0x187c, 0x512))
    {
        std::cout << "All Powerfull M15x detected" << std::endl;

        if (hDeviceHandle == NULL)
            return false;

        AlienfxNew = true;

        return true;
    }

    if (FindDevice(0x187c, 0x514))
    {
        std::cout << "M11x detected" << std::endl;

        if (hDeviceHandle == NULL)
            return false;

        AlienfxNew = true;

        return true;
    }

    if (FindDevice(0x187c, 0x520))
    {
        std::cout << "M17x R3 detected" << std::endl;

        if (hDeviceHandle == NULL)
            return false;

        AlienfxNew = true;

        return true;
    }

    if (FindDevice(0x187c, 0x521))
    {
        std::cout << "M14x R2 detected" << std::endl;

        AlienfxNew = true;

        return true;
    }

    return false;
}

/**
 * Reinitialize the AlienFX device.
 */
bool AlienfxReinit()
{
    // close the device handle
    libusb_close(hDeviceHandle);

    // reopen the device
    hDeviceHandle = libusb_open_device_with_vid_pid(
        context,
        hVID,
        hPID);

    if (hDeviceHandle == NULL)
        return false;

    return true;
}

/**
 * Deinitialize the AlienFX device.
 */
void AlienfxDeinit()
{
    libusb_release_interface(hDeviceHandle, 0);
    libusb_attach_kernel_driver(hDeviceHandle, 0);

    libusb_close(hDeviceHandle);
    libusb_exit(context);
}

/**
 *
 * Return the number of bytes transferred.
 */
int WriteDevice(unsigned char* pData, int pDataLength)
{
#ifdef ALIENFX_DEBUG
    std::cout << "WriteDevice" << std::endl;
#endif

    int res = libusb_control_transfer(hDeviceHandle, SEND_REQUEST_TYPE,
        SEND_REQUEST, SEND_VALUE, SEND_INDEX, pData, pDataLength, 0);

    if (res < 0) std::cerr << "libusb error: " << libusb_error_name(res) << std::endl;
}

/**
 *
 * Return the number of bytes transferred.
 */
int ReadDevice(unsigned char* pData, int pDataLength)
{
#ifdef ALIENFX_DEBUG
    std::cout << "ReadDevice" << std::endl;
#endif

    int res = libusb_control_transfer(hDeviceHandle, READ_REQUEST_TYPE,
        READ_REQUEST, READ_VALUE, READ_INDEX, pData, pDataLength, 0);

    if (res < 0) std::cerr << "libusb error: " << libusb_error_name(res) << std::endl;
}

void AlienfxReset(byte pOptions)
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxReset" << std::endl;
#endif

    byte Buffer[] = { (byte) 0x00 , 0x07, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00 };
    Buffer[2] = pOptions;
    WriteDevice(Buffer, 9);
}

void AlienfxSetColor(byte pAction, byte pSetId, int pLeds, int pColor)
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxSetColor" << std::endl;
#endif

    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00 };
    Buffer[1] = pAction;
    Buffer[2] = pSetId;
    Buffer[3] = (pLeds & 0xFF0000) >> 16;
    Buffer[4] = (pLeds & (byte) 0x00FF00) >> 8;
    Buffer[5] = (pLeds & (byte) 0x0000FF);
    Buffer[6] = (pColor & 0xFF0000) >> 16;
    Buffer[7] = (pColor & (byte) 0x00FF00) >> 8;
    Buffer[8] = (pColor & (byte) 0x0000FF);
    WriteDevice(Buffer, 9);
}

void AlienfxEndLoopBlock()
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxEndLoopBlock" << std::endl;
#endif

    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00, 0x04, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00 };
    WriteDevice(Buffer, 9);
}

void AlienfxEndTransmitionAndExecute()
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxEndTransmitionAndExecute" << std::endl;
#endif

    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00, 0x05, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00 };
    WriteDevice(Buffer, 9);
}

void AlienfxStoreNextInstruction(byte pStorageId)
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxStoreNextInstruction" << std::endl;
#endif

    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00, 0x08, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00 };
    Buffer[2] = pStorageId;
    WriteDevice(Buffer, 9);
}

void AlienfxEndStorageBlock()
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxEndStorageBlock" << std::endl;
#endif

    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00 };
    WriteDevice(Buffer, 9);
}

void AlienfxSaveStorageData()
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxSaveStorageData" << std::endl;
#endif

    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00, 0x09, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00 };
    WriteDevice(Buffer, 9);
}

void AlienfxSetSpeed(int pSpeed)
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxSetSpeed" << std::endl;
#endif

    size_t BytesWritten;
    byte Buffer[] = { (byte) 0x00, 0x0E, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00 };
    Buffer[2] = (pSpeed & 0xFF00) >> 8;
    Buffer[3] = (pSpeed & (byte) 0x00FF);
    WriteDevice(Buffer, 9);
}

byte AlienfxGetDeviceStatus()
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxGetDeviceStatus" << std::endl;
#endif

    size_t BytesWritten;

    byte Buffer[] = { (byte) 0x00, 0x06, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00 };

    WriteDevice(Buffer, 9);

    if (AlienfxNew)
        Buffer[0] = 0x01;

    ReadDevice(Buffer, 9);

#ifdef ALIENFX_DEBUG
    for (int i = 0; i < 9; i++)
        std::cout << " " << std::hex << (int)Buffer[i];

    std::cout << std::endl;
#endif

    if (AlienfxNew)
    {
        if (Buffer[0] == 0x01)
            return 0x06;
        return Buffer[0];
    }
    else return Buffer[1];
}

byte AlienfxWaitForReady()
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxWaitForReady" << std::endl;
#endif

    byte status = AlienfxGetDeviceStatus();

    for (int i = 0; i < 5 && status != ALIENFX_READY; i++)
    {
        if (status == ALIENFX_DEVICE_RESET)
            return status;
        Sleep(1);
        status = AlienfxGetDeviceStatus();
    }

    return status;
}

byte AlienfxWaitForBusy()
{
#ifdef ALIENFX_DEBUG
    std::cout << "AlienfxWaitForBusy" << std::endl;
#endif

    byte status = AlienfxGetDeviceStatus();

    for (int i = 0; i < 5 && status != ALIENFX_BUSY; i++)
    {
        if (status == ALIENFX_DEVICE_RESET)
            return status;
        Sleep(1);
        status = AlienfxGetDeviceStatus();
    }

    return status;
}


#endif // End of OS specific functions


/*
int main()
{
    init_apartment();
    Uri uri(L"http://aka.ms/cppwinrt");
    printf("Hello, %ls!\n", uri.AbsoluteUri().c_str());
}
*/

using namespace std;

#define WHITE_COLOR  0xFFFFFF
#define BLACK_COLOR  0x000000
#define BLUE_COLOR   0x00FFFF
#define ON_COLOR     BLUE_COLOR
#define OFF_COLOR    BLACK_COLOR

int main()
{
    ofstream LogFile;
    string desc;
    char input;
    int iTemp;

    cout << "This program comes without ANY warranty that it is working and/or "
        << "will not damage your hardware or lead to any data loss. It was not "
        << "designed to cause any damage but it might happen.\n\n"
        << "WARNING: Your current AlienFX profile will be erased." << endl;

    input = 0;
    while (input != 'y' && input != 'n') {
        cout << endl << "continue? (y/n)" << endl;
        cin.get(input);
        cin.ignore(100, '\n');
    }
    if (input == 'n')
        return 0;

    LogFile.open("results.log");
    LogFile.width(6);
    LogFile.fill('0');

    cout << "Searching for AlienFX device..." << endl;

    if (!AlienfxInit()) {
        cout << "AlienFX device not found. Program terminated" << endl;

#ifdef _WIN32
        system("pause");
#endif

        return 0;
    }
    else
        cout << "AlienFX device found and connected." << endl << endl;

    if (AlienfxNew) {
        LogFile << "New Alienfx Device" << endl;
    }
    else {
        LogFile << "Old Alienfx Device" << endl;
    }

    cout << "Please enter the name of the Alienware PC/laptop you are using" << endl;
    getline(cin, desc);
    LogFile << "Model description: " << desc << endl;
    cout << endl << "Please make shure that if you have a laptop your touchpad is enabled" << endl;

#ifdef _WIN32
    system("pause");
#endif

    cout << endl << "Disabling all LEDs..." << endl;

    // Disable all the LEDs of the AlienFX device
    AlienfxWaitForBusy();
    iTemp = ALIENFX_SLEEP_LIGHTS; //ALIENFX_ALL_OFF;
    AlienfxReset((byte) iTemp);
    Sleep(2);
    AlienfxWaitForReady();
    AlienfxSetColor((byte)ALIENFX_STAY, (byte)1, 0xFFFFFF, 0);
    AlienfxEndLoopBlock();
    AlienfxEndTransmitionAndExecute();
    Sleep(100);

    input = 0;
    while (input != 'y' && input != 'n') {
        cout << endl << "Are all LEDs off? (y/n)" << endl;
        cin.get(input);
        cin.ignore(100, '\n');
    }
    if (input == 'n') {
        cout << endl << "Please name the LEDs which are still on" << endl;
        getline(cin, desc);
        LogFile << "LEDs still on: " << desc << endl;
    }
    cout << endl << "Now all possible LEDs will be tested" << endl;

    int leds = 0x000001;

    int ids[] = {
      0x0001,
      0x0002,
      0x0008,
      0x0004,
      0x0010,
      0x0020,
      0x0040,
      0x0080,
      0x0100,
      0x0200,
      0x1c00,
      0x2000,
      0x4000,
      0x6000,
      0x6000,
      0x010000,
      0x0f9fff };

    for (int i = 0; i < 17; i++) {
        leds = ids[i];
        cout << endl << "Testing LED " << i << endl;

        // make the current LED blink
        AlienfxWaitForBusy();
        AlienfxReset((byte)ALIENFX_ALL_ON);
        Sleep(2);
        AlienfxWaitForReady();
        AlienfxSetColor((byte)ALIENFX_BLINK, (byte)1, leds, ON_COLOR);
        AlienfxEndLoopBlock();
        AlienfxEndTransmitionAndExecute();
        Sleep(100);

        input = 0;

        while (input != 'y' && input != 'n') {
            cout << endl << "Did something change (y/n)" << endl;
            cin.get(input);
            cin.ignore(100, '\n');
        }

        if (input == 'y') {
            cout << endl << "What did change?" << endl;
            getline(cin, desc);
            LogFile << "Led 0x" << hex << leds << dec << ": " << desc << endl;
        }

        // turn the current LED off
        AlienfxWaitForBusy();
        AlienfxReset((byte)ALIENFX_ALL_ON);
        Sleep(2);
        AlienfxWaitForReady();
        AlienfxSetColor((byte)ALIENFX_BLINK, (byte)1, leds, OFF_COLOR);
        AlienfxEndLoopBlock();
        AlienfxEndTransmitionAndExecute();
        Sleep(100);

        //leds = leds << 1;
    }
    LogFile.close();
    cout << "Testing complete. Please send the 'results.log' file to webmaster@benjamin-thaut.de" << endl;

#ifdef _WIN32
    system("pause");
#endif

    return 0;
}
