#pragma once
#include <string>
#include <string_view>
#include <type_traits>

namespace reverSilly::PythonInterpreter
{
	std::string default_escape(std::string_view& str);
	std::string escape(std::string_view str,auto F)//not quoted
	requires std::is_invocable_r_v<std::string,decltype(F),std::string_view&>
	{
		std::string res;
		while(!str.empty())
			res+=F(str);
		return res;
	}
	

	std::string parse_string(std::string_view str,auto F)//quoted string literal
	requires std::is_invocable_r_v<std::string,decltype(F),std::string_view&>
	{
		bool raw=false;
		auto front_is=[](std::string_view str,std::string_view sv)
		{
			if(str.empty())
				return false;
			return sv.find(str.front())!=std::string_view::npos;
		};
		if(front_is(str,"rRuU"))
		{
			if(front_is(str,"rR"))
				raw=true;
			str.remove_prefix(1);
		}
		if(str.starts_with("'''")||str.starts_with("\"\"\""))
			str.remove_prefix(3);
		else if(front_is(str,"\'\""))
			str.remove_prefix(1);
		if(str.ends_with("'''")||str.ends_with("\"\"\""))
			str.remove_suffix(3);
		else if(!str.empty()&&(str.back()=='\''||str.back()=='"'))
			str.remove_suffix(1);
		if(raw)
			return std::string(str);
		return escape(str,F);
	}
}