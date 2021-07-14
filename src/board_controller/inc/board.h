#pragma once

#include <cmath>
#include <deque>
#include <limits>
#include <string>

#include "board_controller.h"
#include "brainflow_boards.h"
#include "brainflow_constants.h"
#include "brainflow_input_params.h"
#include "data_buffer.h"
#include "spinlock.h"
#include "streamer.h"

#include "spdlog/spdlog.h"

#define MAX_CAPTURE_SAMPLES (86400 * 250) // should be enough for one day of capturing


class Board
{
public:
    static std::shared_ptr<spdlog::logger> board_logger;
    static JNIEnv *java_jnienv; // nullptr unless on java
    static int set_log_level (int log_level);
    static int set_log_file (char *log_file);

    virtual ~Board ()
    {
        skip_logs = true; // also should be set in inherited class destructor because it will be
                          // called before
        free_packages ();
    }

    Board (int board_id, struct BrainFlowInputParams params)
    {
        skip_logs = false;
        db = NULL;
        streamer = NULL;
        this->board_id = board_id;
        this->params = params;
        try
        {
            board_descr = brainflow_boards_json["boards"][std::to_string (board_id)];
        }
        catch (json::exception &e)
        {
            safe_logger (spdlog::level::err, e.what ());
        }
    }
    virtual int prepare_session () = 0;
    virtual int start_stream (int buffer_size, char *streamer_params) = 0;
    virtual int stop_stream () = 0;
    virtual int release_session () = 0;
    virtual int config_board (std::string config, std::string &response) = 0;

    int get_current_board_data (int num_samples, double *data_buf, int *returned_samples);
    int get_board_data_count (int *result);
    int get_board_data (int data_count, double *data_buf);
    int insert_marker (double value);

    // Board::board_logger should not be called from destructors, to ensure that there are safe log
    // methods Board::board_logger still available but should be used only outside destructors
    template <typename Arg1, typename... Args>
    // clang-format off
    void safe_logger (
        spdlog::level::level_enum log_level, const char *fmt, const Arg1 &arg1, const Args &... args)
    // clang-format on
    {
        if (!skip_logs)
        {
            Board::board_logger->log (log_level, fmt, arg1, args...);
        }
    }

    template <typename T>
    void safe_logger (spdlog::level::level_enum log_level, const T &msg)
    {
        if (!skip_logs)
        {
            Board::board_logger->log (log_level, msg);
        }
    }

    int get_board_id ()
    {
        return board_id;
    }

protected:
    DataBuffer *db;
    bool skip_logs;
    int board_id;
    struct BrainFlowInputParams params;
    Streamer *streamer;
    json board_descr;
    SpinLock lock;
    std::deque<double> marker_queue;

    int prepare_for_acquisition (int buffer_size, char *streamer_params);
    void free_packages ();
    void push_package (double *package);

private:
    int prepare_streamer (char *streamer_params);
    // reshapes data from DataBuffer format where all channels are mixed to linear buffer
    void reshape_data (int data_count, const double *buf, double *output_buf);
};
