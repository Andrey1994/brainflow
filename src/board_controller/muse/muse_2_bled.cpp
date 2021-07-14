#include <string>
#include <utility>

#include "get_dll_dir.h"
#include "muse_2_bled.h"

#include "brainflow_constants.h"


int Muse2BLED::num_objects = 0;


Muse2BLED::Muse2BLED (struct BrainFlowInputParams params)
    : DynLibBoard ((int)BoardIds::MUSE_2_BLED_BOARD, params)
{
    Muse2BLED::num_objects++;

    if (Muse2BLED::num_objects > 1)
    {
        is_valid = false;
    }
    else
    {
        is_valid = true;
    }
}

Muse2BLED::~Muse2BLED ()
{
    skip_logs = true;
    Muse2BLED::num_objects--;
    release_session ();
}

std::string Muse2BLED::get_lib_name ()
{
    std::string muselib_path = "";
    std::string muselib_name = "";
    char muselib_dir[1024];
    bool res = get_dll_path (muselib_dir);

#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        muselib_name = "MuseLib32.dll";
    }
    else
    {
        muselib_name = "MuseLib.dll";
    }
#endif
#ifdef __linux__
    muselib_name = "libMuseLib.so";
#endif
#ifdef __APPLE__
    muselib_name = "libMuseLib.dylib";
#endif

    if (res)
    {
        muselib_path = std::string (muselib_dir) + muselib_name;
    }
    else
    {
        muselib_path = muselib_name;
    }
    return muselib_path;
}

int Muse2BLED::prepare_session ()
{
    if (!is_valid)
    {
        safe_logger (spdlog::level::info, "only one Muse2BLED per process is allowed");
        return (int)BrainFlowExitCodes::ANOTHER_BOARD_IS_CREATED_ERROR;
    }
    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "you need to specify dongle port");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    return DynLibBoard::prepare_session ();
}