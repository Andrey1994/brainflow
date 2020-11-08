#include "serial.h"

// ioctl headers conflict with standard posix headers for serial port, ioctl needed for custom
// baudrate, move this function to another compilation unit from serial.cpp
#if defined _WIN32
#include <windows.h>
#elif defined __linux__
#include </usr/include/asm-generic/ioctls.h>
#include </usr/include/asm-generic/termbits.h>
#include <sys/ioctl.h>
#endif


#if defined _WIN32
int Serial::set_custom_baudrate (int baudrate)
{
    DCB dcb_serial_params = {0};
    dcb_serial_params.DCBlength = sizeof (dcb_serial_params);
    if (GetCommState (this->port_descriptor, &dcb_serial_params) == 0)
    {
        CloseHandle (this->port_descriptor);
        return SerialExitCodes::GET_PORT_STATE_ERROR;
    }

    dcb_serial_params.BaudRate = baudrate;
    if (SetCommState (this->port_descriptor, &dcb_serial_params) == 0)
    {
        CloseHandle (this->port_descriptor);
        return SerialExitCodes::SET_PORT_STATE_ERROR;
    }
    return SerialExitCodes::OK;
}

#elif defined __linux__
int Serial::set_custom_baudrate (int baudrate)
{
    struct termios2 port_settings;
    memset (&port_settings, 0, sizeof (port_settings));

    ioctl (this->port_descriptor, TCGETS2, &port_settings);
    port_settings.c_cflag &= ~CBAUD;   // Remove current BAUD rate
    port_settings.c_cflag |= BOTHER;   // Allow custom BAUD rate using int input
    port_settings.c_ispeed = baudrate; // Set the input BAUD rate
    port_settings.c_ospeed = baudrate; // Set the output BAUD rate
    int r = ioctl (this->port_descriptor, TCSETS2, &port_settings);

    if (r == 0)
    {
        return SerialExitCodes::OK;
    }
    else
    {
        return SerialExitCodes::SET_PORT_STATE_ERROR;
    }
}

#else
int Serial::set_custom_baudrate (int baudrate)
{
    // not implemented yet
    return SerialExitCodes::SET_PORT_STATE_ERROR;
}
#endif