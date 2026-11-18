#pragma once
#include <stdexcept>

namespace reverSilly::PythonInterpreter
{
	struct ProgramError:std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};
	template<class...T>
	struct overloaded:T...{ using T::operator()...; };
	[[noreturn]]void throw_not_implemented(std::string msg);
	[[noreturn]]void throw_program_error(std::string msg);

	decltype(auto) rethrow(auto&& F)
	{
		try{ return F(); }
		catch(std::exception &e)
		{ throw ProgramError(e.what()); }
	}
}