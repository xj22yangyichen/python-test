#pragma once
#include "value.h"
#include <variant>

namespace reverSilly::PythonInterpreter
{
	struct ControlFlow
	{
		struct Normal{};
		struct Break{};
		struct Continue{};
		struct Return
		{
			Value value;
		};
		std::variant<Normal,Break,Continue,Return>data;
	};
}