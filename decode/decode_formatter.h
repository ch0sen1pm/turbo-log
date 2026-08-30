#pragma once

#include <string>

#include "effective_msg.pb.h"

class DecodeFormatter {
public:
    void SetPattern(const std::string& pattern);

    void Format(const EffectiveMsg& msg, std::string& dest);
};