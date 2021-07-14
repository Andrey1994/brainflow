#include <fstream>
#include <set>
#include <string.h>
#include <string>
#include <vector>

#include "board.h"
#include "board_info_getter.h"
#include "brainflow_boards.h"
#include "brainflow_constants.h"


static int get_single_value (
    int board_id, const char *param_name, int *value, bool use_logger = true);
static int get_string_value (
    int board_id, const char *param_name, char *string, int *len, bool use_logger = true);
static int get_array_value (
    int board_id, const char *param_name, int *output_array, int *len, bool use_logger = true);


int get_board_descr (int board_id, char *board_descr, int *len)
{
    try
    {
        std::string res = brainflow_boards_json["boards"][std::to_string (board_id)].dump ();
        strcpy (board_descr, res.c_str ());
        *len = (int)strlen (res.c_str ());
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        Board::board_logger->error (e.what ());
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
}

int get_sampling_rate (int board_id, int *sampling_rate)
{
    return get_single_value (board_id, "sampling_rate", sampling_rate);
}

int get_package_num_channel (int board_id, int *package_num_channel)
{
    return get_single_value (board_id, "package_num_channel", package_num_channel);
}

int get_marker_channel (int board_id, int *marker_channel)
{
    return get_single_value (board_id, "marker_channel", marker_channel);
}

int get_battery_channel (int board_id, int *battery_channel)
{
    return get_single_value (board_id, "battery_channel", battery_channel);
}

int get_num_rows (int board_id, int *num_rows)
{
    return get_single_value (board_id, "num_rows", num_rows);
}

int get_timestamp_channel (int board_id, int *timestamp_channel)
{
    return get_single_value (board_id, "timestamp_channel", timestamp_channel);
}

int get_eeg_names (int board_id, char *eeg_names, int *len)
{
    return get_string_value (board_id, "eeg_names", eeg_names, len);
}

int get_device_name (int board_id, char *name, int *len)
{
    return get_string_value (board_id, "name", name, len);
}

int get_eeg_channels (int board_id, int *eeg_channels, int *len)
{
    return get_array_value (board_id, "eeg_channels", eeg_channels, len);
}

int get_emg_channels (int board_id, int *emg_channels, int *len)
{
    return get_array_value (board_id, "emg_channels", emg_channels, len);
}

int get_ecg_channels (int board_id, int *ecg_channels, int *len)
{
    return get_array_value (board_id, "ecg_channels", ecg_channels, len);
}

int get_eog_channels (int board_id, int *eog_channels, int *len)
{
    return get_array_value (board_id, "eog_channels", eog_channels, len);
}

int get_eda_channels (int board_id, int *eda_channels, int *len)
{
    return get_array_value (board_id, "eda_channels", eda_channels, len);
}

int get_ppg_channels (int board_id, int *ppg_channels, int *len)
{
    return get_array_value (board_id, "ppg_channels", ppg_channels, len);
}

int get_accel_channels (int board_id, int *accel_channels, int *len)
{
    return get_array_value (board_id, "accel_channels", accel_channels, len);
}

int get_analog_channels (int board_id, int *analog_channels, int *len)
{
    return get_array_value (board_id, "analog_channels", analog_channels, len);
}

int get_gyro_channels (int board_id, int *gyro_channels, int *len)
{
    return get_array_value (board_id, "gyro_channels", gyro_channels, len);
}

int get_other_channels (int board_id, int *other_channels, int *len)
{
    return get_array_value (board_id, "other_channels", other_channels, len);
}

int get_temperature_channels (int board_id, int *temperature_channels, int *len)
{
    return get_array_value (board_id, "temperature_channels", temperature_channels, len);
}

int get_resistance_channels (int board_id, int *resistance_channels, int *len)
{
    return get_array_value (board_id, "resistance_channels", resistance_channels, len);
}

int get_exg_channels (int board_id, int *exg_channels, int *len)
{
    std::set<int> unique_channels;
    const char *data_types[4] = {"eeg_channels", "emg_channels", "ecg_channels", "eog_channels"};
    for (int i = 0; i < 4; i++)
    {
        int channels[4096] = {0};
        int channels_len = 0;
        int res = get_array_value (board_id, data_types[i], channels, &channels_len, false);
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            for (int i = 0; i < channels_len; i++)
            {
                unique_channels.insert (channels[i]);
            }
        }
    }
    int counter = 0;
    for (std::set<int>::iterator it = unique_channels.begin (); it != unique_channels.end (); it++)
    {
        exg_channels[counter] = *it;
        counter++;
    }
    *len = counter;
    if (counter == 0)
    {
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

static int get_single_value (int board_id, const char *param_name, int *value, bool use_logger)
{
    try
    {
        int val = (int)brainflow_boards_json["boards"][std::to_string (board_id)][param_name];
        *value = val;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        if (use_logger)
        {
            Board::board_logger->error (e.what ());
        }
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
}

static int get_array_value (
    int board_id, const char *param_name, int *output_array, int *len, bool use_logger)
{
    try
    {
        std::vector<int> values =
            brainflow_boards_json["boards"][std::to_string (board_id)][param_name];
        if (!values.empty ())
        {
            memcpy (output_array, &values[0], sizeof (int) * values.size ());
        }
        *len = (int)values.size ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        if (use_logger)
        {
            Board::board_logger->error (e.what ());
        }
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
}

static int get_string_value (
    int board_id, const char *param_name, char *string, int *len, bool use_logger)
{
    try
    {
        std::string val = brainflow_boards_json["boards"][std::to_string (board_id)][param_name];
        strcpy (string, val.c_str ());
        *len = (int)strlen (val.c_str ());
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        if (use_logger)
        {
            Board::board_logger->error (e.what ());
        }
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
}
