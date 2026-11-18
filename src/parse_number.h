#pragma once
#include "value.h"
#include <string_view>

namespace reverSilly::PythonInterpreter
{
	Value parse_number(std::string_view s);
}