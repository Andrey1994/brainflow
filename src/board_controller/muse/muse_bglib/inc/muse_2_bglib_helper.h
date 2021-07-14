#pragma once

#include <string>

#include "muse_bglib_helper.h"


class Muse2BGLibHelper : public MuseBGLibHelper
{
public:
    Muse2BGLibHelper (json board_descr) : MuseBGLibHelper (board_descr)
    {
    }

    std::string get_preset ()
    {
        return "p21";
    }
};
