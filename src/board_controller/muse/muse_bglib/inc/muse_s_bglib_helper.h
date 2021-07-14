#pragma once

#include <string>

#include "muse_bglib_helper.h"


// for now its the same as for Muse2, will add MuseS specific data later
class MuseSBGLibHelper : public MuseBGLibHelper
{
public:
    MuseSBGLibHelper (json board_descr) : MuseBGLibHelper (board_descr)
    {
    }

    std::string get_preset ()
    {
        return "p21";
    }
};
