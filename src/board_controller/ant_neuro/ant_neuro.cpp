#include "ant_neuro.h"

// implementation for linux and windows
#if defined(_WIN32) || defined(__linux__) && !defined(__ANDROID__)

#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <chrono>

#include "get_dll_dir.h"
#include "timestamp.h"

#include "eemagine/sdk/factory.h"
#include "eemagine/sdk/wrapper.h"

using namespace eemagine::sdk;


AntNeuroBoard::AntNeuroBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
    // get full path of and neuro library with assumption that this lib is in the same folder
    char ant_neuro_lib_dir[1024];
    bool res = get_dll_path (ant_neuro_lib_dir);
    std::string lib_name = "";
    if (sizeof (void *) == 4)
    {
#ifdef _WIN32
        lib_name = "eego-SDK32.dll";
#else
        lib_name = "libeego-SDK32.so";
#endif
    }
    else
    {
#ifdef _WIN32
        lib_name = "eego-SDK.dll";
#else
        lib_name = "libeego-SDK.so";
#endif
    }
    if (res)
    {
        ant_neuro_lib_path = std::string (ant_neuro_lib_dir) + lib_name;
    }
    else
    {
        ant_neuro_lib_path = lib_name;
    }
    safe_logger (spdlog::level::debug, "use dyn lib: {}", ant_neuro_lib_path.c_str ());

    keep_alive = false;
    initialized = false;
    amp = NULL;
    stream = NULL;
}

AntNeuroBoard::~AntNeuroBoard ()
{
    skip_logs = true;
    release_session ();
}

int AntNeuroBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    try
    {
        factory fact (ant_neuro_lib_path);
        amp = fact.getAmplifier ();
    }
    catch (const exceptions::notFound &e)
    {
        safe_logger (spdlog::level::err, "No devices found, {}", e.what ());
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    catch (...)
    {
        safe_logger (
            spdlog::level::err, "Failed to create factory from {}", ant_neuro_lib_path.c_str ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroBoard::start_stream (int buffer_size, char *streamer_params)
{
    if (stream != NULL)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    if (amp == NULL)
    {
        safe_logger (spdlog::level::err, "Amplifier is not created");
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    try
    {
        int sampling_rate = board_descr["sampling_rate"];
        stream = amp->OpenEegStream (
            sampling_rate); // todo do we need other args? If yes pass them via config_board
    }
    catch (...)
    {
        safe_logger (spdlog::level::err, "Failed to start acquisition.");
        return (int)BrainFlowExitCodes::STREAM_THREAD_ERROR;
    }
    if (stream == NULL)
    {
        safe_logger (spdlog::level::err, "Failed to start acquisition.");
        return (int)BrainFlowExitCodes::STREAM_THREAD_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroBoard::stop_stream ()
{
    if (stream != NULL)
    {
        keep_alive = false;
        streaming_thread.join ();
        delete stream;
        stream = NULL;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int AntNeuroBoard::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
        initialized = false;
    }
    if (amp != NULL)
    {
        delete amp;
        amp = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void AntNeuroBoard::read_thread ()
{
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    std::vector<int> eeg_channels = board_descr["eeg_channels"];

    while (keep_alive)
    {
        try
        {
            buffer buf = stream->getData ();
            int buf_channels = buf.getChannelCount ();
            for (int i = 0; i < (int)buf.getSampleCount (); i++)
            {
                // two channels reserved for trigger and sample counter
                for (int j = 0; j < std::min ((int)eeg_channels.size (), (int)(buf_channels - 2));
                     j++)
                {
                    package[eeg_channels[j]] = buf.getSample (j, i);
                }
                package[board_descr["package_num_channel"].get<int> ()] =
                    buf.getSample (buf_channels - 1, i);
                package[board_descr["other_channels"][0].get<int> ()] =
                    buf.getSample (buf_channels - 2, i); // place trigger into other channels
                package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
                push_package (package);
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
        }
        catch (...)
        {
            safe_logger (spdlog::level::err, "exception in data thread.");
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
        }
    }
    delete[] package;
}

int AntNeuroBoard::config_board (std::string config, std::string &response)
{
    // todo do we need some options?
    safe_logger (spdlog::level::debug, "config_board is not supported for Ant Neuro.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

// stub for macos
#else
AntNeuroBoard::AntNeuroBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
}

AntNeuroBoard::~AntNeuroBoard ()
{
}

int AntNeuroBoard::prepare_session ()
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroBoard::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroBoard::release_session ()
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroBoard::stop_stream ()
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroBoard::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}
#endif
