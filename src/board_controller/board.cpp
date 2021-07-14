#include <string>
#include <vector>

#include "board.h"
#include "board_controller.h"
#include "custom_cast.h"
#include "file_streamer.h"
#include "multicast_streamer.h"
#include "stub_streamer.h"

#include "spdlog/sinks/null_sink.h"

#define LOGGER_NAME "board_logger"

#ifdef __ANDROID__
#include "spdlog/sinks/android_sink.h"
std::shared_ptr<spdlog::logger> Board::board_logger =
    spdlog::android_logger (LOGGER_NAME, "brainflow_ndk_logger");
#else
std::shared_ptr<spdlog::logger> Board::board_logger = spdlog::stderr_logger_mt (LOGGER_NAME);
#endif

JNIEnv *Board::java_jnienv = nullptr;

int Board::set_log_level (int level)
{
    int log_level = level;
    if (level > 6)
    {
        log_level = 6;
    }
    if (level < 0)
    {
        log_level = 0;
    }
    try
    {
        Board::board_logger->set_level (spdlog::level::level_enum (log_level));
        Board::board_logger->flush_on (spdlog::level::level_enum (log_level));
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Board::set_log_file (char *log_file)
{
#ifdef __ANDROID__
    Board::board_logger->error ("For Android set_log_file is unavailable");
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
#else
    try
    {
        spdlog::level::level_enum level = Board::board_logger->level ();
        Board::board_logger = spdlog::create<spdlog::sinks::null_sink_st> (
            "null_logger"); // to dont set logger to nullptr and avoid race condition
        spdlog::drop (LOGGER_NAME);
        Board::board_logger = spdlog::basic_logger_mt (LOGGER_NAME, log_file);
        Board::board_logger->set_level (level);
        Board::board_logger->flush_on (level);
        spdlog::drop ("null_logger");
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}

int Board::prepare_for_acquisition (int buffer_size, char *streamer_params)
{
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    if (streamer)
    {
        delete streamer;
        streamer = NULL;
    }
    if (db)
    {
        delete db;
        db = NULL;
    }

    std::vector<std::string> required_fields {"num_rows", "timestamp_channel", "name"};
    for (std::string field : required_fields)
    {
        if (board_descr.find (field) == board_descr.end ())
        {
            safe_logger (spdlog::level::err,
                "Field {} is not found in brainflow_boards.h for id {}", field, board_id);
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    int res = prepare_streamer (streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    db = new DataBuffer ((int)board_descr["num_rows"], buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer with size {}", buffer_size);
        delete db;
        db = NULL;
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Board::push_package (double *package)
{
    lock.lock ();
    try
    {
        int marker_channel = board_descr["marker_channel"];
        if (marker_queue.empty ())
        {
            package[marker_channel] = 0.0;
        }
        else
        {
            double marker = marker_queue.at (0);
            package[marker_channel] = marker;
            marker_queue.pop_front ();
        }
    }
    catch (...)
    {
        safe_logger (spdlog::level::err, "Failed to get marker channel/value");
    }
    lock.unlock ();

    if (db != NULL)
    {
        db->add_data (package);
    }
    if (streamer != NULL)
    {
        streamer->stream_data (package);
    }
}

int Board::insert_marker (double value)
{
    if (std::fabs (value) < std::numeric_limits<double>::epsilon ())
    {
        safe_logger (spdlog::level::err, "0 is a default value for marker, you can not use it.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    lock.lock ();
    marker_queue.push_back (value);
    lock.unlock ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Board::free_packages ()
{
    if (db != NULL)
    {
        delete db;
        db = NULL;
    }

    if (streamer != NULL)
    {
        delete streamer;
        streamer = NULL;
    }
}

int Board::prepare_streamer (char *streamer_params)
{
    int num_rows = (int)board_descr["num_rows"];
    // to dont write smth like if (streamer) every time for all boards create dummy streamer which
    // does nothing and return an instance of this streamer if user dont specify streamer_params
    if (streamer_params == NULL)
    {
        safe_logger (spdlog::level::debug, "use stub streamer");
        streamer = new StubStreamer (num_rows);
    }
    else if (streamer_params[0] == '\0')
    {
        safe_logger (spdlog::level::debug, "use stub streamer");
        streamer = new StubStreamer (num_rows);
    }
    else
    {
        // parse string, sscanf doesnt work
        std::string streamer_params_str (streamer_params);
        size_t idx1 = streamer_params_str.find ("://");
        if (idx1 == std::string::npos)
        {
            safe_logger (
                spdlog::level::err, "format is streamer_type://streamer_dest:streamer_args");
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        std::string streamer_type = streamer_params_str.substr (0, idx1);
        size_t idx2 = streamer_params_str.find_last_of (":", std::string::npos);
        if ((idx2 == std::string::npos) || (idx1 == idx2))
        {
            safe_logger (
                spdlog::level::err, "format is streamer_type://streamer_dest:streamer_args");
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        std::string streamer_dest = streamer_params_str.substr (idx1 + 3, idx2 - idx1 - 3);
        std::string streamer_mods = streamer_params_str.substr (idx2 + 1);

        if (streamer_type == "file")
        {
            safe_logger (spdlog::level::trace, "File Streamer, file: {}, mods: {}",
                streamer_dest.c_str (), streamer_mods.c_str ());
            streamer = new FileStreamer (streamer_dest.c_str (), streamer_mods.c_str (), num_rows);
        }
        if (streamer_type == "streaming_board")
        {
            int port = 0;
            try
            {
                port = std::stoi (streamer_mods);
            }
            catch (const std::exception &e)
            {
                safe_logger (spdlog::level::err, e.what ());
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }
            safe_logger (spdlog::level::trace, "MultiCast Streamer, ip addr: {}, port: {}",
                streamer_dest.c_str (), streamer_mods.c_str ());
            streamer = new MultiCastStreamer (streamer_dest.c_str (), port, num_rows);
        }

        if (streamer == NULL)
        {
            safe_logger (
                spdlog::level::err, "unsupported streamer type {}", streamer_type.c_str ());
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }

    int res = streamer->init_streamer ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init streamer");
        delete streamer;
        streamer = NULL;
    }

    return res;
}

int Board::get_current_board_data (int num_samples, double *data_buf, int *returned_samples)
{
    if (!db)
    {
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    if ((!data_buf) || (!returned_samples))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int num_rows = (int)board_descr["num_rows"];

    double *buf = new double[num_samples * num_rows];
    int num_data_points = (int)db->get_current_data (num_samples, buf);
    reshape_data (num_data_points, buf, data_buf);
    delete[] buf;
    *returned_samples = num_data_points;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Board::get_board_data_count (int *result)
{
    if (!db)
    {
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    if (!result)
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    *result = (int)db->get_data_count ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Board::get_board_data (int data_count, double *data_buf)
{
    if (!db)
    {
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    if (!data_buf)
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int num_rows = (int)board_descr["num_rows"];
    double *buf = new double[data_count * num_rows];
    int num_data_points = (int)db->get_data (data_count, buf);
    reshape_data (num_data_points, buf, data_buf);
    delete[] buf;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Board::reshape_data (int data_count, const double *buf, double *output_buf)
{
    int num_rows = (int)board_descr["num_rows"];

    for (int i = 0; i < data_count; i++)
    {
        for (int j = 0; j < num_rows; j++)
        {
            output_buf[j * data_count + i] = buf[i * num_rows + j];
        }
    }
}
