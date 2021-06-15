#include <algorithm>
#include <string.h>
#include <string>

#include "muse_bglib_helper.h"
#include "muse_constants.h"
#include "muse_types.h"
#include "uart.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define UART_TIMEOUT 1000


int MuseBGLibHelper::initialize (struct BrainFlowInputParams params)
{
    if (!initialized)
    {
        input_params = params;
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        initialized = true;
        int buffer_size = get_buffer_size ();
        db = new DataBuffer (buffer_size, 1000);
        constexpr int packages_in_transaction = 12;
        current_buf.resize (packages_in_transaction);
        new_eeg_data.resize (4); // 4 eeg channels
        for (int i = 0; i < packages_in_transaction; i++)
        {
            current_buf[i].resize (buffer_size);
            std::fill (current_buf[i].begin (), current_buf[i].end (), 0.0);
            std::fill (new_eeg_data.begin (), new_eeg_data.end (), false);
        }
        new_eeg_data.reserve (4);
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseBGLibHelper::open_device ()
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (uart_open (input_params.serial_port.c_str ()))
    {
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    int res = reset_ble_dev ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    state = (int)DeviceState::OPEN_CALLED;
    ble_cmd_gap_discover (gap_discover_observation);
    res = wait_for_callback ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    ble_cmd_gap_end_procedure ();
    return open_ble_dev ();
}

int MuseBGLibHelper::stop_stream ()
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (!should_stop_stream)
    {
        should_stop_stream = true;
        read_characteristic_thread.join ();
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    // for sanity check
    for (int i = 0; i < 5; i++)
    {
        const char *stop_cmd = "h";
        res = config_device (stop_cmd);
    }
    return res;
}

int MuseBGLibHelper::start_stream ()
{
    // from silicanlabs forum - write 0x00001 to enable notifications
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    uint8 configuration[] = {0x01, 0x00};
    for (uint16 ccid : ccids)
    {
        state = (int)DeviceState::WRITE_TO_CLIENT_CHAR;
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        ble_cmd_attclient_attribute_write (connection, ccid, 2, &configuration);
        ble_cmd_attclient_execute_write (connection, 1);
        int res = wait_for_callback ();
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
    }
    int res = config_device (get_preset ().c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    res = config_device ("d");
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        should_stop_stream = false;
        read_characteristic_thread = std::thread ([this] { this->thread_worker (); });
    }

    return res;
}

int MuseBGLibHelper::close_device ()
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    state = (int)DeviceState::CLOSE_CALLED;
    stop_stream ();
    uart_close ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseBGLibHelper::get_data (void *param)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    state = (int)DeviceState::GET_DATA_CALLED;
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    size_t num_points = db->get_data (1, (double *)param);
    if (num_points != 1)
    {
        res = (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    return res;
}

int MuseBGLibHelper::release ()
{
    close_device ();
    exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    connection = -1;
    muse_handle_start = 0;
    muse_handle_end = 0;
    state = (int)DeviceState::NONE;
    should_stop_stream = true;
    initialized = false;
    control_char_handle = 0;
    ccids.clear ();
    characteristics.clear ();
    if (db)
    {
        delete db;
        db = NULL;
    }
    new_eeg_data.clear ();
    for (int i = 0; i < current_buf.size (); i++)
    {
        current_buf[i].clear ();
    }
    current_buf.clear ();

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseBGLibHelper::config_device (const char *config)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    uint8 command[16];
    int len = (int)strlen ((char *)config);
    command[0] = (uint8)len + 1;
    for (int i = 0; i < len; i++)
    {
        command[i + 1] = int (config[i]);
    }
    command[len + 1] = 10;
    if (control_char_handle == 0)
    {
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    ble_cmd_attclient_write_command (connection, control_char_handle, len + 2, command);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

/////////////////
/// Callbacks ///
/////////////////

void MuseBGLibHelper::ble_evt_connection_status (const struct ble_msg_connection_status_evt_t *msg)
{
    // New connection
    if (msg->flags & connection_connected)
    {
        connection = msg->connection;
        // this method is called from ble_evt_connection_disconnected need to set exit code only
        // when we call this method from open_ble_device
        if (state == (int)DeviceState::INITIAL_CONNECTION)
        {
            exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

void MuseBGLibHelper::ble_evt_connection_disconnected (
    const struct ble_msg_connection_disconnected_evt_t *msg)
{
    // atempt to reconnect
    // changing values here leads to package loss, dont touch it
    ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_random, 10, 76, 100, 0);
}

// ble_evt_attclient_group_found and ble_evt_attclient_procedure_completed are called after the same
// command(ble_cmd_attclient_read_by_group_type)
void MuseBGLibHelper::ble_evt_attclient_group_found (
    const struct ble_msg_attclient_group_found_evt_t *msg)
{
    if (msg->uuid.len == 0)
    {
        return;
    }
    uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
    if (uuid == MUSE_SERVICE_UUID)
    {
        muse_handle_start = msg->start;
        muse_handle_end = msg->end;
    }
}

void MuseBGLibHelper::ble_evt_attclient_procedure_completed (
    const struct ble_msg_attclient_procedure_completed_evt_t *msg)
{
    if ((state == (int)DeviceState::WRITE_TO_CLIENT_CHAR) ||
        (state == (int)DeviceState::CONFIG_CALLED))
    {
        if (msg->result == 0)
        {
            exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
    if (state == (int)DeviceState::OPEN_CALLED)
    {
        if ((muse_handle_start) && (muse_handle_end))
        {
            ble_cmd_attclient_find_information (msg->connection, muse_handle_start,
                muse_handle_end); // triggers
                                  // ble_evt_attclient_find_information_found
        }
    }
}

void MuseBGLibHelper::ble_evt_attclient_find_information_found (
    const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
    if (state == (int)DeviceState::OPEN_CALLED)
    {
        if (msg->uuid.len == 2)
        {
            uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
            if (uuid == CLIENT_CHARACTERISTIC_UUID)
            {
                ccids.insert (msg->chrhandle);
            }
        }

        if (msg->uuid.len == 16)
        {
            uint16 uuid_int = (msg->uuid.data[1] << 8) | msg->uuid.data[0];
            std::cout << (int)uuid_int << std::endl;
            char str[37] = {};
            sprintf (str, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                msg->uuid.data[15], msg->uuid.data[14], msg->uuid.data[13], msg->uuid.data[12],
                msg->uuid.data[11], msg->uuid.data[10], msg->uuid.data[9], msg->uuid.data[8],
                msg->uuid.data[7], msg->uuid.data[6], msg->uuid.data[5], msg->uuid.data[4],
                msg->uuid.data[3], msg->uuid.data[2], msg->uuid.data[1], msg->uuid.data[0]);
            std::string uuid (str);
            if (strcmp (str, MUSE_GATT_ATTR_STREAM_TOGGLE) == 0)
            {
                control_char_handle = msg->chrhandle;
            }
            if (strcmp (str, MUSE_GATT_ATTR_TP9) == 0)
            {
                characteristics[msg->chrhandle] = uuid;
            }
            if (strcmp (str, MUSE_GATT_ATTR_AF7) == 0)
            {
                characteristics[msg->chrhandle] = uuid;
            }
            if (strcmp (str, MUSE_GATT_ATTR_AF8) == 0)
            {
                characteristics[msg->chrhandle] = uuid;
            }
            if (strcmp (str, MUSE_GATT_ATTR_TP10) == 0)
            {
                characteristics[msg->chrhandle] = uuid;
            }
            if (strcmp (str, MUSE_GATT_ATTR_ACCELEROMETER) == 0)
            {
                characteristics[msg->chrhandle] = uuid;
            }
            std::cout << uuid << std::endl;
        }
        if ((characteristics.size () == 5) && (state == (int)DeviceState::OPEN_CALLED) &&
            (ccids.size () == 10))
        {
            exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

void MuseBGLibHelper::ble_evt_attclient_attribute_value (
    const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
    if ((int)msg->value.len != 20)
    {
        return;
    }
    std::string uuid = "";
    if (characteristics.find (msg->atthandle) != characteristics.end ())
    {
        uuid = characteristics[msg->atthandle];
    }
    else
    {
        return;
    }

    if (uuid == MUSE_GATT_ATTR_ACCELEROMETER)
    {
        for (int axis = 0; axis < 3; axis++)
        {
            double accel_val1 = msg->value.data[2 + axis * 6] * 256 + msg->value.data[3 + axis * 6];
            double accel_val2 = msg->value.data[4 + axis * 6] * 256 + msg->value.data[5 + axis * 6];
            double accel_val3 = msg->value.data[6 + axis * 6] * 256 + msg->value.data[7 + axis * 6];
            for (int counter = 0; counter < current_buf.size (); counter++)
            {
                if (counter < 4)
                {
                    current_buf[counter][axis + 5] = accel_val1 * MUSE_ACCELEROMETER_SCALE_FACTOR;
                }
                else if (counter < 8)
                {
                    current_buf[counter][axis + 5] = accel_val2 * MUSE_ACCELEROMETER_SCALE_FACTOR;
                }
                else
                {
                    current_buf[counter][axis + 5] = accel_val3 * MUSE_ACCELEROMETER_SCALE_FACTOR;
                }
            }
        }
    }
    else
    {
        int pos = -1;
        if (uuid == MUSE_GATT_ATTR_TP9)
        {
            pos = 1;
            new_eeg_data[0] = true;
        }
        if (uuid == MUSE_GATT_ATTR_AF7)
        {
            pos = 2;
            new_eeg_data[1] = true;
        }
        if (uuid == MUSE_GATT_ATTR_AF8)
        {
            pos = 3;
            new_eeg_data[2] = true;
        }
        if (uuid == MUSE_GATT_ATTR_TP10)
        {
            pos = 4;
            new_eeg_data[3] = true;
        }

        if (pos < 0)
        {
            return;
        }

        unsigned int package_num = msg->value.data[0] * 256 + msg->value.data[1];
        for (int i = 2, counter = 0; i < msg->value.len; i += 3, counter += 2)
        {
            double val1 = msg->value.data[i] << 4 | msg->value.data[i + 1] >> 4;
            double val2 = (msg->value.data[i + 1] & 0xF) << 8 | msg->value.data[i + 2];
            val1 = (val1 - 0x800) * 125.0 / 256.0;
            val2 = (val2 - 0x800) * 125.0 / 256.0;
            current_buf[counter][pos] = val1;
            current_buf[counter + 1][pos] = val2;
            current_buf[counter][0] = package_num;
            current_buf[counter + 1][0] = package_num;
        }

        if (std::find (new_eeg_data.begin (), new_eeg_data.end (), false) == new_eeg_data.end ())
        {
            for (int i = 0; i < current_buf.size (); i++)
            {
                db->add_data (&current_buf[i][0]);
            }
            std::fill (new_eeg_data.begin (), new_eeg_data.end (), false);
        }
    }
}

void MuseBGLibHelper::ble_evt_gap_scan_response (const struct ble_msg_gap_scan_response_evt_t *msg)
{
    char name[512];
    bool name_found_in_response = false;
    for (int i = 0; i < msg->data.len;)
    {
        int8 len = msg->data.data[i++];
        if (!len)
        {
            continue;
        }
        if (i + len > msg->data.len)
        {
            break; // not enough data
        }
        uint8 type = msg->data.data[i++];
        if (type == 0x09) // no idea what is 0x09
        {
            name_found_in_response = true;
            memcpy (name, msg->data.data + i, len - 1);
            name[len - 1] = '\0';
        }

        i += len - 1;
    }

    bool is_valid = true;
    if (name_found_in_response)
    {
        if (!input_params.serial_number.empty ())
        {
            if (strcmp (name, input_params.serial_number.c_str ()) != 0)
            {
                is_valid = false;
            }
        }
        else
        {
            if (strstr (name, "Muse") == NULL)
            {
                is_valid = false;
            }
        }
        if (is_valid)
        {
            memcpy ((void *)connect_addr.addr, msg->sender.addr, sizeof (bd_addr));
            exit_code = (int)BrainFlowExitCodes::STATUS_OK;
        }
    }
}

///////////////
/// Helpers ///
///////////////

void MuseBGLibHelper::thread_worker ()
{
    while (!should_stop_stream)
    {
        read_message (UART_TIMEOUT);
    }
}

int MuseBGLibHelper::read_message (int timeout_ms)
{
    unsigned char *data = NULL;
    struct ble_header hdr;
    int r;

    r = uart_rx (sizeof (hdr), (unsigned char *)&hdr, timeout_ms);
    if (!r)
    {
        return -1; // timeout
    }
    else if (r < 0)
    {
        exit_code = (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
        return 1; // fails to read
    }
    if (hdr.lolen)
    {
        data = new unsigned char[hdr.lolen];
        r = uart_rx (hdr.lolen, data, UART_TIMEOUT);
        if (r <= 0)
        {
            exit_code = (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
            delete[] data;
            return 1; // fails to read
        }
    }

    const struct ble_msg *msg = ble_get_msg_hdr (hdr);
    if (msg)
    {
        msg->handler (data);
    }
    delete[] data;
    return 0;
}

int MuseBGLibHelper::open_ble_dev ()
{
    exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    // send command to connect
    state = (int)DeviceState::INITIAL_CONNECTION;
    ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_public, 10, 76, 100, 0);
    int res = wait_for_callback ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    state = (int)DeviceState::OPEN_CALLED;
    exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    uint8 primary_service_uuid[] = {0x00, 0x28};

    ble_cmd_attclient_read_by_group_type (
        connection, FIRST_HANDLE, LAST_HANDLE, 2, primary_service_uuid);

    res = wait_for_callback ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    // from siliconlabs forum - write 0x00001 to enable notifications
    // copypasted in start_stream method but lets keep it in 2 places
    uint8 configuration[] = {0x01, 0x00};
    for (uint16 ccid : ccids)
    {
        state = (int)DeviceState::WRITE_TO_CLIENT_CHAR;
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        ble_cmd_attclient_attribute_write (connection, ccid, 2, &configuration);
        ble_cmd_attclient_execute_write (connection, 1);
        wait_for_callback ();
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseBGLibHelper::wait_for_callback ()
{
    auto start_time = std::chrono::high_resolution_clock::now ();
    int run_time = 0;
    while ((run_time < input_params.timeout) &&
        (exit_code == (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR))
    {
        if (read_message (UART_TIMEOUT) > 0)
        {
            break;
        }
        auto end_time = std::chrono::high_resolution_clock::now ();
        run_time =
            (int)std::chrono::duration_cast<std::chrono::seconds> (end_time - start_time).count ();
    }
    return exit_code;
}

int MuseBGLibHelper::reset_ble_dev ()
{
    // Reset dongle to get it into known state
    ble_cmd_system_reset (0);
    uart_close ();
    int i;
    for (i = 0; i < 5; i++)
    {
#ifdef _WIN32
        Sleep (500);
#else
        usleep (500000);
#endif
        if (!uart_open (input_params.serial_port.c_str ()))
        {
            break;
        }
    }
    if (i == 5)
    {
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}