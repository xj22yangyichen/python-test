#include "parse_number.h"
#include "utility.h"
#include "value.h"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <string>

namespace reverSilly::PythonInterpreter
{
	Value parse_number(std::string_view s)
	{
		using str=std::string;
		int base=10;
		if(s.starts_with("0x")||s.starts_with("0X"))
			base=16;
		if(s.starts_with("0o")||s.starts_with("0O"))
			base=8;
		if(s.starts_with("0b")||s.starts_with("0B"))
			base=2;
		if(base!=10)
			return {Integer(str{s.substr(2)},base)};
		if(std::all_of(s.begin(),s.end(),static_cast<int(*)(int)>(std::isdigit)))
			return {Integer(str{s})};
		Float val;
		auto[p,ec]=std::from_chars(s.begin(),s.end(),val);
		if(p!=s.end()||ec!=std::errc{})
			throw_program_error("number literal not recognized:"+str{s});
		return {val};
	}
}