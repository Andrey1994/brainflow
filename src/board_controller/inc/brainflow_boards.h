#pragma once

#include "json.hpp"

using json = nlohmann::json;

// clang-format off
json brainflow_boards_json = {
    {"boards", {
        {"-3",
            {{"name", "PlayBack"}
        }},
        {"-2",
            {{"name", "Streaming"}
        }},
        {"-1",
            {{"name", "Synthetic"},
            {"sampling_rate", 250},
            {"package_num_channel", 0},
            {"battery_channel", 29},
            {"timestamp_channel", 30},
            {"num_rows", 31},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"eeg_names", "Fz,C3,Cz,C4,Pz,PO7,Oz,PO8,F5,F7,F3,F1,F2,F4,F6,F8"},
            {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"accel_channels", {17, 18, 19}},
            {"gyro_channels", {20, 21, 22}},
            {"eda_channels", {23}},
            {"ppg_channels", {24, 25}},
            {"temperature_channels", {26}},
            {"resistance_channels", {27, 28}}
        }},
        {"0",
            {{"name", "Cyton"},
            {"sampling_rate", 250},
            {"package_num_channel", 0},
            {"timestamp_channel", 22},
            {"num_rows", 23},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"eeg_names", "Fp1,Fp2,C3,C4,P7,P8,O1,O2"},
            {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"accel_channels", {9, 10, 11}},
            {"analog_channels", {19, 20, 21}},
            {"other_channels", {12, 13, 14, 15, 16, 17, 18}}
        }},
        {"1",
            {{"name", "Ganglion"},
            {"sampling_rate", 200},
            {"package_num_channel", 0},
            {"timestamp_channel", 13},
            {"num_rows", 14},
            {"eeg_channels", {1, 2, 3, 4}},
            {"emg_channels", {1, 2, 3, 4}},
            {"ecg_channels", {1, 2, 3, 4}},
            {"eog_channels", {1, 2, 3, 4}},
            {"accel_channels", {5, 6, 7}},
            {"resistance_channels", {8, 9, 10, 11, 12}}
        }},
        {"2",
            {{"name", "CytonDaisy"},
            {"sampling_rate", 125},
            {"package_num_channel", 0},
            {"timestamp_channel", 30},
            {"num_rows", 31},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"eeg_names", "Fp1,Fp2,C3,C4,P7,P8,O1,O2,F7,F8,F3,F4,T7,T8,P3,P4"},
            {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"accel_channels", {17, 18, 19}},
            {"analog_channels", {27, 28, 29}},
            {"other_channels", {20, 21, 22, 23, 24, 25, 26}}
        }},
        {"3",
            {{"name", "Galea"},
            {"sampling_rate", 250},
            {"package_num_channel", 0},
            {"timestamp_channel", 22},
            {"num_rows", 23},
            {"battery_channel", 21},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 10, 15}},
            {"eeg_names", "Fz,C3,Cz,C4,Pz,PO7,Oz,PO8,F5,F7"},
            {"emg_channels", {9, 12, 14, 16}},
            {"eog_channels", {11, 13}},
            {"eda_channels", {19}},
            {"ppg_channels", {17, 18}},
            {"temperature_channels", {20}}
        }},
        {"4",
            {{"name", "GanglionWifi"},
            {"sampling_rate", 1600},
            {"package_num_channel", 0},
            {"timestamp_channel", 23},
            {"num_rows", 24},
            {"eeg_channels", {1, 2, 3, 4}},
            {"emg_channels", {1, 2, 3, 4}},
            {"ecg_channels", {1, 2, 3, 4}},
            {"eog_channels", {1, 2, 3, 4}},
            {"accel_channels", {5, 6, 7}},
            {"analog_channels", {15, 16, 17}},
            {"other_channels", {8, 9, 10, 11, 12, 13, 14}},
            {"resistance_channels", {18, 19, 20, 21, 22}}
        }},
        {"5",
            {{"name", "CytonWifi"},
            {"sampling_rate", 1000},
            {"package_num_channel", 0},
            {"timestamp_channel", 22},
            {"num_rows", 23},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"accel_channels", {9, 10, 11}},
            {"analog_channels", {19, 20, 21}},
            {"other_channels", {12, 13, 14, 15, 16, 17, 18}}
        }},
        {"6",
            {{"name", "CytonDaisyWifi"},
            {"sampling_rate", 1000},
            {"package_num_channel", 0},
            {"timestamp_channel", 30},
            {"num_rows", 31},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
            {"accel_channels", {17, 18, 19}},
            {"analog_channels", {27, 28, 29}},
            {"other_channels", {20, 21, 22, 23, 24, 25, 26}}
        }},
        {"7",
            {{"name", "BrainBit"},
            {"sampling_rate", 250},
            {"timestamp_channel", 10},
            {"package_num_channel", 0},
            {"battery_channel", 9},
            {"num_rows", 11},
            {"eeg_channels", {1, 2, 3, 4}},
            {"eeg_names", "T3,T4,O1,O2"},
            {"resistance_channels", {5, 6, 7, 8}}
        }},
        {"8",
            {{"name", "Unicorn"},
            {"sampling_rate", 250},
            {"timestamp_channel", 17},
            {"package_num_channel", 15},
            {"num_rows", 18},
            {"eeg_channels", {0, 1, 2, 3, 4, 5, 6, 7}},
            {"eeg_names", "Fz,C3,Cz,C4,Pz,PO7,Oz,PO8"},
            {"accel_channels", {8, 9, 10}},
            {"gyro_channels", {11, 12, 13}},
            {"other_channels", {16}},
            {"battery_channel", 14}
        }},
        {"9",
            {{"name", "CallibriEEG"},
            {"sampling_rate", 250},
            {"timestamp_channel", 2},
            {"package_num_channel", 0},
            {"num_rows", 3},
            {"eeg_channels", {1}}
        }},
        {"10",
            {{"name", "CallibriEMG"},
            {"sampling_rate", 1000},
            {"timestamp_channel", 2},
            {"package_num_channel", 0},
            {"num_rows", 3},
            {"emg_channels", {1}}
        }},
        {"11",
            {{"name", "CallibriECG"},
            {"sampling_rate", 125},
            {"timestamp_channel", 2},
            {"package_num_channel", 0},
            {"num_rows", 3},
            {"ecg_channels", {1}}
        }},
        {"12",
            {{"name", "Fascia"},
            {"sampling_rate", 500},
            {"package_num_channel", 0},
            {"timestamp_channel", 19},
            {"num_rows", 20},
            {"eeg_channels", {2, 3, 4, 5, 6, 7, 8, 9}},
            {"accel_channels", {10, 11, 12}},
            {"gyro_channels", {13, 14, 15}},
            {"eda_channels", {16}}, {"ppg_channels", {18}},
            {"temperature_channels", {17}},
            {"other_channels", {1}}
        }},
        {"13",
            {{"name", "NotionOSC1"},
            {"sampling_rate", 250},
            {"timestamp_channel", 10},
            {"package_num_channel", 0},
            {"num_rows", 11},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"eeg_names", "CP6,F6,C4,CP4,CP3,F5,C3,CP5"},
            {"other_channels", {9}}
        }},
        {"14",
            {{"name", "NotionOSC2"},
            {"sampling_rate", 250},
            {"timestamp_channel", 10},
            {"package_num_channel", 0},
            {"num_rows", 11},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
            {"eeg_names", "CP5,F5,C3,CP3,CP6,F6,C4,CP4"},
            {"other_channels", {9}}
        }},
        {"15",
            {{"name", "IronBCI"},
            {"sampling_rate", 250},
            {"timestamp_channel", 9},
            {"package_num_channel", 0},
            {"num_rows", 10},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}}
        }},
        {"16",
            {{"name", "GforcePro"},
            {"sampling_rate", 500},
            {"package_num_channel", 0},
            {"timestamp_channel", 9},
            {"num_rows", 10},
            {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}}
        }},
        {"17",
            {{"name", "FreeEEG32"},
            {"sampling_rate", 512},
            {"timestamp_channel", 33},
            {"package_num_channel", 0},
            {"num_rows", 34},
            {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}}
        }}
    }
}};
// clang-format on