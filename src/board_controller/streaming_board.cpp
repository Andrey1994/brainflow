#include <string.h>

#include "board_info_getter.h"
#include "multicast_streamer.h"
#include "streaming_board.h"

#ifndef _WIN32
#include <errno.h>
#endif


StreamingBoard::StreamingBoard (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::STREAMING_BOARD,
          params) // its a hack - set board_id for streaming board here temporary and override it
                  // with master board id in prepare_session, board_id is protected and there is no
                  // api to get it so its ok
{
    client = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
}

StreamingBoard::~StreamingBoard ()
{
    skip_logs = true;
    release_session ();
}

int StreamingBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if ((params.ip_address.empty ()) || (params.other_info.empty ()) || (params.ip_port == 0))
    {
        safe_logger (spdlog::level::err,
            "write multicast group ip to ip_address field, ip port to ip_port field and original "
            "board id to other info");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        board_id = std::stoi (params.other_info);
        board_descr = brainflow_boards_json["boards"][std::to_string (board_id)];
    }
    catch (json::exception &e)
    {
        safe_logger (spdlog::level::err, "invalid json");
        safe_logger (spdlog::level::err, e.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    catch (const std::exception &e)
    {
        safe_logger (spdlog::level::err,
            "Write board id for the board which streams data to other_info field");
        safe_logger (spdlog::level::err, e.what ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    client = new MultiCastClient (params.ip_address.c_str (), params.ip_port);
    int res = client->init ();
    if (res != (int)MultiCastReturnCodes::STATUS_OK)
    {
#ifdef _WIN32
        safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
        safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
        safe_logger (spdlog::level::err, "failed to init socket: {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int StreamingBoard::config_board (std::string config, std::string &response)
{
    // dont allow streaming boards to change config for master board
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int StreamingBoard::start_stream (int buffer_size, char *streamer_params)
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

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int StreamingBoard::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int StreamingBoard::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
        if (client)
        {
            delete client;
            client = NULL;
        }
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void StreamingBoard::read_thread ()
{
    // format for incomming package is determined by original board
    int num_rows = board_descr["num_rows"];
    int num_packages = MultiCastStreamer::get_packages_in_chunk ();
    int transaction_len = num_rows * num_packages;
    int bytes_per_recv = sizeof (double) * transaction_len;
    double *transaction = new double[transaction_len];
    for (int i = 0; i < num_rows; i++)
    {
        transaction[i] = 0.0;
    }

    while (keep_alive)
    {
        int res = client->recv (transaction, bytes_per_recv);
        if (res != bytes_per_recv)
        {
            safe_logger (
                spdlog::level::trace, "unable to read {} bytes, read {}", bytes_per_recv, res);
            continue;
        }
        for (int i = 0; i < num_packages; i++)
        {
            push_package (transaction + i * num_rows);
        }
    }
    delete[] transaction;
}
