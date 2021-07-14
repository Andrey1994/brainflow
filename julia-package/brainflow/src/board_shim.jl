import JSON
export BrainFlowInputParams

@enum BoardIds begin

    PLAYBACK_FILE_BOARD = -3
    STREAMING_BOARD = -2
    SYNTHETIC_BOARD = -1
    CYTON_BOARD = 0
    GANGLION_BOARD = 1
    CYTON_DAISY_BOARD = 2
    GALEA_BOARD = 3
    GANGLION_WIFI_BOARD = 4
    CYTON_WIFI_BOARD = 5
    CYTON_DAISY_WIFI_BOARD = 6
    BRAINBIT_BOARD = 7
    UNICORN_BOARD = 8
    CALLIBRI_EEG_BOARD = 9
    CALLIBRI_EMG_BOARD = 10
    CALLIBRI_ECG_BOARD = 11
    FASCIA_BOARD = 12
    NOTION_1_BOARD = 13
    NOTION_2_BOARD = 14
    IRONBCI_BOARD = 15
    GFORCE_PRO_BOARD = 16
    FREEEEG32_BOARD = 17
    BRAINBIT_BLED_BOARD = 18
    GFORCE_DUAL_BOARD = 19
    GALEA_SERIAL_BOARD = 20
    MUSE_S_BLED_BOARD = 21
    MUSE_2_BLED_BOARD = 22
    CROWN_BOARD = 23
    ANT_NEURO_EE_411_BOARD = 24

end

BoardIdType = Union{BoardIds, Integer}

@enum IpProtocolType begin

    NONE = 0
    UDP = 1
    TCP = 2

end

@Base.kwdef mutable struct BrainFlowInputParams
    serial_port::String = ""
    mac_address::String = ""
    ip_address::String = ""
    ip_port::Int32 = 0
    ip_protocol::Int32 = Integer(NONE)
    other_info::String = ""
    timeout::Int32 = 0
    serial_number::String = ""
    file::String = ""
end

@brainflow_rethrow function get_sampling_rate(board_id::BoardIdType)
    val = Vector{Cint}(undef, 1)
    ccall((:get_sampling_rate, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    value = val[1]
    return value
end

@brainflow_rethrow function get_num_rows(board_id::BoardIdType)
    val = Vector{Cint}(undef, 1)
    ccall((:get_num_rows, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    # here dont need to add 1, last element included
    value = val[1]
    return value
end

@brainflow_rethrow function get_eeg_names(board_id::BoardIdType)
    names_string = Vector{Cuchar}(undef, 4096)
    len = Vector{Cint}(undef, 1)
    ccall((:get_eeg_names, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}, Ptr{Cint}), Int32(board_id), names_string, len)
    sub_string = String(names_string)[1:len[1]]
    value = split(sub_string, ',')
    return value
end

@brainflow_rethrow function get_board_descr(board_id::BoardIdType)
    names_string = Vector{Cuchar}(undef, 16000)
    len = Vector{Cint}(undef, 1)
    ccall((:get_board_descr, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}, Ptr{Cint}), Int32(board_id), names_string, len)
    sub_string = String(names_string)[1:len[1]]
    value = JSON.parse(sub_string)
    return value
end

@brainflow_rethrow function get_device_name(board_id::BoardIdType)
    names_string = Vector{Cuchar}(undef, 4096)
    len = Vector{Cint}(undef, 1)
    ccall((:get_device_name, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}, Ptr{Cint}), Int32(board_id), names_string, len)
    sub_string = String(names_string)[1:len[1]]
    return sub_string
end

single_channel_function_names = (
    :get_timestamp_channel,
    :get_package_num_channel,
    :get_battery_channel,
    :get_marker_channel,
)

# generating the channels functions
for func_name = single_channel_function_names
    cglobal_expr = Meta.parse("cglobal((:$func_name, BOARD_CONTROLLER_INTERFACE))")
    @eval @brainflow_rethrow function $func_name(board_id::BoardIdType)
        channel = Vector{Cint}(undef, 1)
        ccall($cglobal_expr, Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
        # julia counts from 1
        @inbounds value = channel[1] + 1
        return value
    end
end

channel_function_names = (
    :get_eeg_channels,
    :get_exg_channels,
    :get_emg_channels,
    :get_ecg_channels,
    :get_eog_channels,
    :get_eda_channels,
    :get_ppg_channels,
    :get_accel_channels,
    :get_analog_channels,
    :get_gyro_channels,
    :get_other_channels,
    :get_temperature_channels,
    :get_resistance_channels,
)

# generating the channels functions
for func_name = channel_function_names
    cglobal_expr = Meta.parse("cglobal((:$func_name, BOARD_CONTROLLER_INTERFACE))")
    @eval @brainflow_rethrow function $func_name(board_id::BoardIdType)
        channels = Vector{Cint}(undef, 512)
        len = Vector{Cint}(undef, 1)
        ccall($cglobal_expr, Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
        # julia counts from 1
        @inbounds value = channels[1:len[1]] .+ 1
        return value
    end
end

struct BoardShim

    master_board_id::Int32
    board_id::Int32
    input_json::String

    function BoardShim(id::Integer, params::BrainFlowInputParams)
        master_id = id
        if id == Integer(STREAMING_BOARD) || id == Integer(PLAYBACK_FILE_BOARD)
            try
                master_id = parse(Int, params.other_info)
            catch
                throw(BrainFlowError("you need to provide master board id to other_info field of BrainFlowInputParams", Integer(INVALID_ARGUMENTS_ERROR)))
            end
        end
        new(master_id, id, JSON.json(params))
    end

end
BoardShim(id::BoardIds, params::BrainFlowInputParams) = BoardShim(Integer(id), params)
BoardShim(id) = BoardShim(id, BrainFlowInputParams())

@brainflow_rethrow function prepare_session(board_shim::BoardShim)
    ccall((:prepare_session, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
end

@brainflow_rethrow function start_stream(board_shim::BoardShim, num_samples::Int, streamer_params::String)
    ccall((:start_stream, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}, Cint, Ptr{UInt8}), num_samples, streamer_params,
            board_shim.board_id, board_shim.input_json)
end

function start_stream(board_shim::BoardShim)
    start_stream(board_shim, 450000, "")
end

@brainflow_rethrow function is_prepared(board_shim::BoardShim)
    val = Vector{Cint}(undef, 1)
    ccall((:is_prepared, BOARD_CONTROLLER_INTERFACE), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    value = Bool(val[1])
    return value
end

@brainflow_rethrow function get_board_data_count(board_shim::BoardShim)
    val = Vector{Cint}(undef, 1)
    ccall((:get_board_data_count, BOARD_CONTROLLER_INTERFACE), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    value = val[1]
    return value
end

@brainflow_rethrow function insert_marker(value::Float64, board_shim::BoardShim)
    ccall((:insert_marker, BOARD_CONTROLLER_INTERFACE), Cint, (Float64, Cint, Ptr{UInt8}), value, board_shim.board_id, board_shim.input_json)
end

@brainflow_rethrow function stop_stream(board_shim::BoardShim)
    ccall((:stop_stream, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
end


@brainflow_rethrow function release_session(board_shim::BoardShim)
    ccall((:release_session, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
end

@brainflow_rethrow function config_board(config::String, board_shim::BoardShim)
    resp_string = Vector{Cuchar}(undef, 4096)
    len = Vector{Cint}(undef, 1)
    ccall((:config_board, BOARD_CONTROLLER_INTERFACE), Cint, (Ptr{UInt8}, Ptr{UInt8}, Ptr{Cint}, Cint, Ptr{UInt8}),
            config, resp_string, len, board_shim.board_id, board_shim.input_json)
    sub_string = String(resp_string)[1:len[1]]
    return sub_string
end

@brainflow_rethrow function get_board_data(board_shim::BoardShim)
    data_size = get_board_data_count(board_shim)
    num_rows = get_num_rows(board_shim.master_board_id)
    val = Vector{Float64}(undef, num_rows * data_size)
    ccall((:get_board_data, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Float64}, Cint, Ptr{UInt8}), 
            data_size, val, board_shim.board_id, board_shim.input_json)
    value = transpose(reshape(val, (data_size, num_rows)))
    return value
end

@brainflow_rethrow function get_current_board_data(num_samples::Integer, board_shim::BoardShim)
    data_size = Vector{Cint}(undef, 1)
    num_rows = get_num_rows(board_shim.master_board_id)
    val = Vector{Float64}(undef, num_rows * num_samples)
    ccall((:get_current_board_data, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Float64}, Ptr{Cint}, Cint, Ptr{UInt8}), 
            num_samples, val, data_size, board_shim.board_id, board_shim.input_json)
    value = transpose(reshape(val[1:data_size[1] * num_rows], (data_size[1], num_rows)))
    return value
end