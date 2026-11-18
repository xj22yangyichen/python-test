#pragma once
#include "function.h"
#include "value.h"

namespace reverSilly::PythonInterpreter
{
	namespace builtins
	{
		Value int_(Arguments arg);
		Value float_(Arguments arg);
		Value str_(Arguments arg);
		Value bool_(Arguments arg);
		Value print_(Arguments arg);
	}
	void inject_builtins(FunctionTable &s);
}