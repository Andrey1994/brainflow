#pragma once

#include <string>
#include <thread>

#include "board.h"

#if defined __linux__ || defined _WIN32
// required to dont link ant neuro sdk
#define EEGO_SDK_BIND_DYNAMIC

#include <eemagine/sdk/amplifier.h>
#include <eemagine/sdk/stream.h>
#endif


class AntNeuroBoard : public Board
{

#if defined __linux__ || defined _WIN32
private:
    volatile bool keep_alive;
    bool initialized;
    std::thread streaming_thread;
    std::string ant_neuro_lib_path;
    eemagine::sdk::amplifier *amp;
    eemagine::sdk::stream *stream;

    void read_thread ();
#endif


public:
    AntNeuroBoard (int board_id, struct BrainFlowInputParams params);
    ~AntNeuroBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};
