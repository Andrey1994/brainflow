#include "unicorn_board.h"

// implementation for linux and windows
#if defined __linux__ || defined _WIN32

#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#endif

#include "get_dll_dir.h"
#include "timestamp.h"
#include "unicorn_board.h"


constexpr int UnicornBoard::package_size;


UnicornBoard::UnicornBoard (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::UNICORN_BOARD, params)
{
    // get full path of libunicorn.so with assumption that this lib is in the same folder
    char unicornlib_dir[1024];
    device_handle = 0;
    bool res = get_dll_path (unicornlib_dir);
    std::string unicornlib_path = "";
#ifdef _WIN32
    std::string lib_name = "Unicorn.dll";
#elif defined(__arm__)
    std::string lib_name = "libunicorn_raspberry.so";
#else
    std::string lib_name = "libunicorn.so";
#endif
    if (res)
    {
        unicornlib_path = std::string (unicornlib_dir) + lib_name;
    }
    else
    {
        unicornlib_path = lib_name;
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", unicornlib_path.c_str ());
    dll_loader = new DLLLoader (unicornlib_path.c_str ());

    is_streaming = false;
    keep_alive = false;
    initialized = false;
    func_get_data = NULL;
}

UnicornBoard::~UnicornBoard ()
{
    skip_logs = true;
    release_session ();
}

int UnicornBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::debug, "Library is loaded");

    func_get_data = (int (*) (UNICORN_HANDLE, uint32_t, float *, uint32_t))dll_loader->get_address (
        "UNICORN_GetData");
    if (func_get_data == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for UNICORN_GetData");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = call_open ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int UnicornBoard::start_stream (int buffer_size, char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    res = call_start ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int UnicornBoard::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        return call_stop ();
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int UnicornBoard::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
        initialized = false;
    }
    if (dll_loader != NULL)
    {
        func_get_data = NULL;
        call_close ();
        dll_loader->free_library ();
        delete dll_loader;
        dll_loader = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void UnicornBoard::read_thread ()
{
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    float temp_buffer[UnicornBoard::package_size];
    std::vector<int> eeg_channels = board_descr["eeg_channels"];

    while (keep_alive)
    {
        // unicorn uses similar idea as in brainflow - return single array with different kinds of
        // data and provide API(defines in this case) to mark this data
        func_get_data (device_handle, 1, temp_buffer, UnicornBoard::package_size * sizeof (float));
        package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
        // eeg data
        package[eeg_channels[0]] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX];
        package[eeg_channels[1]] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 1];
        package[eeg_channels[2]] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 2];
        package[eeg_channels[3]] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 3];
        package[eeg_channels[4]] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 4];
        package[eeg_channels[5]] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 5];
        package[eeg_channels[6]] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 6];
        package[eeg_channels[7]] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 7];
        // accel data
        package[board_descr["accel_channels"][0].get<int> ()] =
            (double)temp_buffer[UNICORN_ACCELEROMETER_CONFIG_INDEX];
        package[board_descr["accel_channels"][1].get<int> ()] =
            (double)temp_buffer[UNICORN_ACCELEROMETER_CONFIG_INDEX + 1];
        package[board_descr["accel_channels"][2].get<int> ()] =
            (double)temp_buffer[UNICORN_ACCELEROMETER_CONFIG_INDEX + 2];
        // gyro data
        package[board_descr["gyro_channels"][0].get<int> ()] =
            (double)temp_buffer[UNICORN_GYROSCOPE_CONFIG_INDEX];
        package[board_descr["gyro_channels"][1].get<int> ()] =
            (double)temp_buffer[UNICORN_GYROSCOPE_CONFIG_INDEX + 1];
        package[board_descr["gyro_channels"][2].get<int> ()] =
            (double)temp_buffer[UNICORN_GYROSCOPE_CONFIG_INDEX + 2];
        // battery data
        package[board_descr["battery_channel"].get<int> ()] =
            (double)temp_buffer[UNICORN_BATTERY_CONFIG_INDEX];
        // counter / package num
        package[board_descr["package_num_channel"].get<int> ()] =
            (double)temp_buffer[UNICORN_COUNTER_CONFIG_INDEX];
        // validation config index? place it to other channels
        package[board_descr["other_channels"][0].get<int> ()] =
            (double)temp_buffer[UNICORN_VALIDATION_CONFIG_INDEX];
        push_package (package);
    }
    delete[] package;
}

int UnicornBoard::config_board (std::string config, std::string &response)
{
    // todo if there will be requests for it.
    // Unicorn API provides int Unicorn_SetConfiguration method
    safe_logger (spdlog::level::debug, "config_board is not supported for Unicorn.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::call_open ()
{
    // find devices
    int (*func_get_available) (UNICORN_DEVICE_SERIAL *, uint32_t *, BOOL) =
        (int (*) (UNICORN_DEVICE_SERIAL *, uint32_t *, BOOL))dll_loader->get_address (
            "UNICORN_GetAvailableDevices");
    if (func_get_available == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for UNICORN_GetAvailableDevices");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    unsigned int available_device_count = 0;
    int ec = func_get_available (NULL, &available_device_count, TRUE);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_GetAvailableDevices {}", ec);
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    if (available_device_count < 1)
    {
        safe_logger (spdlog::level::err, "Unicorn not found");
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    UNICORN_DEVICE_SERIAL *available_devices = new UNICORN_DEVICE_SERIAL[available_device_count];
    ec = func_get_available (available_devices, &available_device_count, TRUE);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_GetAvailableDevices {}", ec);
        delete[] available_devices;
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    // search for device
    unsigned int device_num = 0;
    if (params.serial_number.empty ())
    {
        safe_logger (spdlog::level::warn,
            "Use device with id {}. To select another one provide id to serial_number field.",
            available_devices[device_num]);
    }
    else
    {
        for (device_num = 0; device_num < available_device_count; device_num++)
        {
            if (strcmp (available_devices[device_num], params.serial_number.c_str ()) == 0)
            {
                break;
            }
        }
        if (device_num == available_device_count)
        {
            safe_logger (
                spdlog::level::err, "device with id {} not found", params.serial_number.c_str ());
            delete[] available_devices;
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    // open device
    int (*func_open) (UNICORN_DEVICE_SERIAL, UNICORN_HANDLE *) = (int (*) (
        UNICORN_DEVICE_SERIAL, UNICORN_HANDLE *))dll_loader->get_address ("UNICORN_OpenDevice");
    if (func_open == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for UNICORN_OpenDevice");
        delete[] available_devices;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    ec = func_open (available_devices[device_num], &device_handle);
    if ((ec != UNICORN_ERROR_SUCCESS) || (device_handle == 0))
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_OpenDevice {}", ec);
        delete[] available_devices;
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    delete[] available_devices;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int UnicornBoard::call_start ()
{
    int (*func_start_streaming) (UNICORN_HANDLE, BOOL) =
        (int (*) (UNICORN_HANDLE, BOOL))dll_loader->get_address ("UNICORN_StartAcquisition");
    if (func_start_streaming == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for UNICORN_StartAcquisition");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int ec = func_start_streaming (device_handle, FALSE);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_StartAcquisition {}", ec);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int UnicornBoard::call_stop ()
{
    int (*func_stop_streaming) (UNICORN_HANDLE) =
        (int (*) (UNICORN_HANDLE))dll_loader->get_address ("UNICORN_StopAcquisition");
    if (func_stop_streaming == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for UNICORN_StopAcquisition");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int ec = func_stop_streaming (device_handle);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_StopAcquisition {}", ec);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int UnicornBoard::call_close ()
{
    int (*func_close) (UNICORN_HANDLE *) =
        (int (*) (UNICORN_HANDLE *))dll_loader->get_address ("UNICORN_CloseDevice");
    if (func_close == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for UNICORN_CloseDevice");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int ec = func_close (&device_handle);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_CloseDevice {}", ec);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}


// stub for macos
#else
UnicornBoard::UnicornBoard (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::UNICORN_BOARD, params)
{
}

UnicornBoard::~UnicornBoard ()
{
}

int UnicornBoard::prepare_session ()
{
    safe_logger (spdlog::level::err, "UnicornBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::err, "UnicornBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::release_session ()
{
    safe_logger (spdlog::level::err, "UnicornBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::stop_stream ()
{
    safe_logger (spdlog::level::err, "UnicornBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::err, "UnicornBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}
#endif
