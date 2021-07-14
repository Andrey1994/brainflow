#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
// windows sockets version 2 should be added before windows.h(seems like there is a version
// conflict), windows sockets are used for wifi boards
#include <windows.h>
#endif

#include <map>
#include <memory>
#include <mutex>
#include <string.h>
#include <string>
#include <utility>

#include "ant_neuro.h"
#include "board.h"
#include "board_controller.h"
#include "board_info_getter.h"
#include "brainbit.h"
#include "brainbit_bled.h"
#include "brainflow_constants.h"
#include "brainflow_input_params.h"
#include "callibri_ecg.h"
#include "callibri_eeg.h"
#include "callibri_emg.h"
#include "cyton.h"
#include "cyton_daisy.h"
#include "cyton_daisy_wifi.h"
#include "cyton_wifi.h"
#include "fascia.h"
#include "freeeeg32.h"
#include "galea.h"
#include "galea_serial.h"
#include "ganglion.h"
#include "ganglion_wifi.h"
#include "gforce_dual.h"
#include "gforce_pro.h"
#include "ironbci.h"
#include "muse_2_bled.h"
#include "muse_s_bled.h"
#include "notion_osc.h"
#include "playback_file_board.h"
#include "streaming_board.h"
#include "synthetic_board.h"
#include "unicorn_board.h"

#include "json.hpp"

using json = nlohmann::json;


std::map<std::pair<int, struct BrainFlowInputParams>, std::shared_ptr<Board>> boards;
std::mutex mutex;

std::pair<int, struct BrainFlowInputParams> get_key (
    int board_id, struct BrainFlowInputParams params);
static int check_board_session (int board_id, char *json_brainflow_input_params,
    std::pair<int, struct BrainFlowInputParams> &key, bool log_error = true);
static int string_to_brainflow_input_params (
    const char *json_brainflow_input_params, struct BrainFlowInputParams *params);


int prepare_session (int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    Board::board_logger->info ("incoming json: {}", json_brainflow_input_params);
    struct BrainFlowInputParams params;
    int res = string_to_brainflow_input_params (json_brainflow_input_params, &params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    std::pair<int, struct BrainFlowInputParams> key = get_key (board_id, params);
    if (boards.find (key) != boards.end ())
    {
        Board::board_logger->error (
            "Board with id {} and the same config already exists", board_id);
        return (int)BrainFlowExitCodes::ANOTHER_BOARD_IS_CREATED_ERROR;
    }

    std::shared_ptr<Board> board = NULL;
    switch (static_cast<BoardIds> (board_id))
    {
        case BoardIds::PLAYBACK_FILE_BOARD:
            board = std::shared_ptr<Board> (new PlaybackFileBoard (params));
            break;
        case BoardIds::STREAMING_BOARD:
            board = std::shared_ptr<Board> (new StreamingBoard (params));
            break;
        case BoardIds::SYNTHETIC_BOARD:
            board = std::shared_ptr<Board> (new SyntheticBoard (params));
            break;
        case BoardIds::CYTON_BOARD:
            board = std::shared_ptr<Board> (new Cyton (params));
            break;
        case BoardIds::GANGLION_BOARD:
            board = std::shared_ptr<Board> (new Ganglion (params));
            break;
        case BoardIds::CYTON_DAISY_BOARD:
            board = std::shared_ptr<Board> (new CytonDaisy (params));
            break;
        case BoardIds::GALEA_BOARD:
            board = std::shared_ptr<Board> (new Galea (params));
            break;
        case BoardIds::GANGLION_WIFI_BOARD:
            board = std::shared_ptr<Board> (new GanglionWifi (params));
            break;
        case BoardIds::CYTON_WIFI_BOARD:
            board = std::shared_ptr<Board> (new CytonWifi (params));
            break;
        case BoardIds::CYTON_DAISY_WIFI_BOARD:
            board = std::shared_ptr<Board> (new CytonDaisyWifi (params));
            break;
        case BoardIds::BRAINBIT_BOARD:
            board = std::shared_ptr<Board> (new BrainBit (params));
            break;
        case BoardIds::UNICORN_BOARD:
            board = std::shared_ptr<Board> (new UnicornBoard (params));
            break;
        case BoardIds::CALLIBRI_EEG_BOARD:
            board = std::shared_ptr<Board> (new CallibriEEG (params));
            break;
        case BoardIds::CALLIBRI_EMG_BOARD:
            board = std::shared_ptr<Board> (new CallibriEMG (params));
            break;
        case BoardIds::CALLIBRI_ECG_BOARD:
            board = std::shared_ptr<Board> (new CallibriECG (params));
            break;
        case BoardIds::FASCIA_BOARD:
            board = std::shared_ptr<Board> (new Fascia (params));
            break;
        // notion 1, notion 2 and crown have the same class
        // the only difference are get_eeg_names and sampling_rate
        case BoardIds::NOTION_1_BOARD:
            board = std::shared_ptr<Board> (new NotionOSC (params));
            break;
        case BoardIds::NOTION_2_BOARD:
            board = std::shared_ptr<Board> (new NotionOSC (params));
            break;
        case BoardIds::CROWN_BOARD:
            board = std::shared_ptr<Board> (new NotionOSC (params));
            break;
        case BoardIds::IRONBCI_BOARD:
            board = std::shared_ptr<Board> (new IronBCI (params));
            break;
        case BoardIds::GFORCE_PRO_BOARD:
            board = std::shared_ptr<Board> (new GforcePro (params));
            break;
        case BoardIds::FREEEEG32_BOARD:
            board = std::shared_ptr<Board> (new FreeEEG32 (params));
            break;
        case BoardIds::BRAINBIT_BLED_BOARD:
            board = std::shared_ptr<Board> (new BrainBitBLED (params));
            break;
        case BoardIds::GFORCE_DUAL_BOARD:
            board = std::shared_ptr<Board> (new GforceDual (params));
            break;
        case BoardIds::GALEA_SERIAL_BOARD:
            board = std::shared_ptr<Board> (new GaleaSerial (params));
            break;
        case BoardIds::MUSE_S_BLED_BOARD:
            board = std::shared_ptr<Board> (new MuseSBLED (params));
            break;
        case BoardIds::MUSE_2_BLED_BOARD:
            board = std::shared_ptr<Board> (new Muse2BLED (params));
            break;
        case BoardIds::ANT_NEURO_EE_411_BOARD:
            board = std::shared_ptr<Board> (
                new AntNeuroBoard ((int)BoardIds::ANT_NEURO_EE_411_BOARD, params));
            break;
        // todo add more ant neuro boards, they will share the same class(AntNeuroBoard) but will
        // have different board ids, just need to fill brainflow_boards.cpp
        default:
            return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
    Board::board_logger->trace ("Board object created {}", board->get_board_id ());
    res = board->prepare_session ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        board = NULL;
    }
    else
    {
        boards[key] = board;
    }
    return res;
}

int is_prepared (int *prepared, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        *prepared = 1;
    }
    if (res == (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR)
    {
        *prepared = 0;
        res = (int)BrainFlowExitCodes::STATUS_OK;
    }
    return res;
}

int start_stream (
    int buffer_size, char *streamer_params, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->start_stream (buffer_size, streamer_params);
}

int stop_stream (int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->stop_stream ();
}

int insert_marker (double value, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->insert_marker (value);
}

int release_session (int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    res = board_it->second->release_session ();
    boards.erase (board_it);
    return res;
}

int get_current_board_data (int num_samples, double *data_buf, int *returned_samples, int board_id,
    char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->get_current_board_data (num_samples, data_buf, returned_samples);
}

int get_board_data_count (int *result, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->get_board_data_count (result);
}

int get_board_data (
    int data_count, double *data_buf, int board_id, char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    return board_it->second->get_board_data (data_count, data_buf);
}

int set_log_level (int log_level)
{
    std::lock_guard<std::mutex> lock (mutex);
    return Board::set_log_level (log_level);
}

int log_message (int log_level, char *log_message)
{
    // its a method for loggging from high level api dont add it to Board class since it should not
    // be used internally
    std::lock_guard<std::mutex> lock (mutex);
    if (log_level < 0)
    {
        Board::board_logger->warn ("log level should be >= 0");
        log_level = 0;
    }
    else if (log_level > 6)
    {
        Board::board_logger->warn ("log level should be <= 6");
        log_level = 6;
    }

    Board::board_logger->log (spdlog::level::level_enum (log_level), "{}", log_message);

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int set_log_file (char *log_file)
{
    std::lock_guard<std::mutex> lock (mutex);
    return Board::set_log_file (log_file);
}

int java_set_jnienv (JNIEnv *java_jnienv)
{
    Board::java_jnienv = java_jnienv;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int config_board (char *config, char *response, int *response_len, int board_id,
    char *json_brainflow_input_params)
{
    std::lock_guard<std::mutex> lock (mutex);
    if ((config == NULL) || (response == NULL) || (response_len == NULL))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    std::pair<int, struct BrainFlowInputParams> key;
    int res = check_board_session (board_id, json_brainflow_input_params, key, false);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto board_it = boards.find (key);
    std::string conf = config;
    std::string resp = "";
    res = board_it->second->config_board (conf, resp);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        *response_len = (int)resp.length ();
        strcpy (response, resp.c_str ());
    }
    return res;
}

/////////////////////////////////////////////////
//////////////////// helpers ////////////////////
/////////////////////////////////////////////////

std::pair<int, struct BrainFlowInputParams> get_key (
    int board_id, struct BrainFlowInputParams params)
{
    std::pair<int, struct BrainFlowInputParams> key = std::make_pair (board_id, params);
    return key;
}

int check_board_session (int board_id, char *json_brainflow_input_params,
    std::pair<int, struct BrainFlowInputParams> &key, bool log_error)
{
    struct BrainFlowInputParams params;
    int res = string_to_brainflow_input_params (json_brainflow_input_params, &params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    key = get_key (board_id, params);

    if (boards.find (key) == boards.end ())
    {
        if (log_error)
        {
            Board::board_logger->error (
                "Board with id {} and port provided config is not created", key.first);
        }
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int string_to_brainflow_input_params (
    const char *json_brainflow_input_params, struct BrainFlowInputParams *params)
{
    // input string -> json -> struct BrainFlowInputParams
    try
    {
        json config = json::parse (std::string (json_brainflow_input_params));
        params->serial_port = config["serial_port"];
        params->ip_protocol = config["ip_protocol"];
        params->ip_port = config["ip_port"];
        params->other_info = config["other_info"];
        params->mac_address = config["mac_address"];
        params->ip_address = config["ip_address"];
        params->timeout = config["timeout"];
        params->serial_number = config["serial_number"];
        params->file = config["file"];
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        Board::board_logger->error ("invalid input json, {}", e.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}
