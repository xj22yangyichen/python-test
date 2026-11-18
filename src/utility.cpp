#include "utility.h"
#include <stdexcept>

namespace reverSilly::PythonInterpreter
{
	void throw_not_implemented(std::string msg)
	{
		throw std::logic_error{std::move(msg)};
	}
	void throw_program_error(std::string msg)
	{
		throw ProgramError{std::move(msg)};
	}
}