#include <chrono>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "playback_file_board.h"
#include "timestamp.h"

#define SET_LOOPBACK_TRUE "loopback_true"
#define SET_LOOPBACK_FALSE "loopback_false"
#define NEW_TIMESTAMPS "new_timestamps"
#define OLD_TIMESTAMPS "old_timestamps"


PlaybackFileBoard::PlaybackFileBoard (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::PLAYBACK_FILE_BOARD,
          params) // its a hack - set board_id for playback board here temporary and override it
                  // with master board id in prepare_session, board_id is protected and there is no
                  // api to get it so its ok
{
    keep_alive = false;
    loopback = false;
    is_streaming = false;
    initialized = false;
    use_new_timestamps = true;
    this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

PlaybackFileBoard::~PlaybackFileBoard ()
{
    skip_logs = true;
    release_session ();
}

int PlaybackFileBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if ((params.file.empty ()) || (params.other_info.empty ()))
    {
        safe_logger (spdlog::level::err, "playback file or master board id not provided");
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
        safe_logger (
            spdlog::level::err, "Write board id of board which recorded data to other_info field");
        safe_logger (spdlog::level::err, e.what ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // check that file exist in prepare_session
    FILE *fp;
    fp = fopen (params.file.c_str (), "r");
    if (fp == NULL)
    {
        safe_logger (spdlog::level::err, "invalid file path");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    fclose (fp);

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int PlaybackFileBoard::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::trace, "start stream");
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
    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 2 * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in 2sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
}

int PlaybackFileBoard::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int PlaybackFileBoard::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
        initialized = false;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void PlaybackFileBoard::read_thread ()
{
    FILE *fp;
    fp = fopen (params.file.c_str (), "r");
    if (fp == NULL)
    {
        safe_logger (spdlog::level::err, "failed to open file in thread");
        return;
    }
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    char buf[4096];
    double last_timestamp = -1.0;
    bool new_timestamps = use_new_timestamps; // to prevent changing during streaming
    int timestamp_channel = board_descr["timestamp_channel"];
    double accumulated_time_delta = 0.0;

    while (keep_alive)
    {
        auto start = std::chrono::high_resolution_clock::now ();
        char *res = fgets (buf, sizeof (buf), fp);
        if ((loopback) && (res == NULL))
        {
            fseek (fp, 0, SEEK_SET); // go to beginning'
            last_timestamp = -1.0;
            continue;
        }
        if ((!loopback) && (res == NULL))
        {
// busy wait instead exit
#ifdef _WIN32
            Sleep (1);
#else
            usleep (1000);
#endif
            continue;
        }
        // res not NULL
        std::string tsv_string (buf);
        std::stringstream ss (tsv_string);
        std::vector<std::string> splitted;
        std::string tmp;
        char sep = '\t';
        if (tsv_string.find ('\t') == std::string::npos)
        {
            sep = ',';
        }
        while (std::getline (ss, tmp, sep))
        {
            if (tmp != "\n")
            {
                splitted.push_back (tmp);
            }
        }
        if (splitted.size () != num_rows)
        {
            safe_logger (spdlog::level::err,
                "invalid string in file, check provided board id. String size {}, expected size {}",
                splitted.size (), num_rows);
            continue;
        }
        for (int i = 0; i < num_rows; i++)
        {
            package[i] = std::stod (splitted[i]);
        }
        // notify main thread
        if (this->state != (int)BrainFlowExitCodes::STATUS_OK)
        {
            {
                std::lock_guard<std::mutex> lk (this->m);
                this->state = (int)BrainFlowExitCodes::STATUS_OK;
            }
            this->cv.notify_one ();
        }
        auto stop = std::chrono::high_resolution_clock::now ();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count ();

        if (last_timestamp > 0)
        {
            double time_wait = package[timestamp_channel] - last_timestamp; // in seconds
            accumulated_time_delta += duration;
            if (accumulated_time_delta > 1000.0)
            {
                time_wait = time_wait - (int)(accumulated_time_delta / 1000.0);
                accumulated_time_delta -= 1000.0;
            }
            if (time_wait > 0.001)
            {
#ifdef _WIN32
                Sleep ((int)(time_wait * 1000));
#else
                usleep ((int)(time_wait * 1000000));
#endif
            }
        }
        last_timestamp = package[timestamp_channel];

        if (new_timestamps)
        {
            package[timestamp_channel] = get_timestamp ();
        }
        push_package (package);
    }
    fclose (fp);
    delete[] package;
}

int PlaybackFileBoard::config_board (std::string config, std::string &response)
{
    if (strcmp (config.c_str (), SET_LOOPBACK_TRUE) == 0)
    {
        loopback = true;
    }
    else if (strcmp (config.c_str (), SET_LOOPBACK_FALSE) == 0)
    {
        loopback = false;
    }
    else if (strcmp (config.c_str (), NEW_TIMESTAMPS) == 0)
    {
        use_new_timestamps = true;
    }
    else if (strcmp (config.c_str (), OLD_TIMESTAMPS) == 0)
    {
        use_new_timestamps = false;
    }
    else
    {
        safe_logger (spdlog::level::warn, "invalid config string {}", config);
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}
