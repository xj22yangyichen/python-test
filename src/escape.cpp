#include "escape.h"
#include <charconv>
#include <string_view>
#include <system_error>

namespace reverSilly::PythonInterpreter
{
	std::string default_escape(std::string_view& str)
	{
		if(unsigned ch=str.front();ch!='\\')
		{
			str.remove_prefix(1);
			return {static_cast<char>(ch)};
		}
		else
		{
			str.remove_prefix(1);
			if(!str.empty())
			{
				switch(str.front())
				{
					case 'a':ch='\a';break;
					case 'b':ch='\b';break;
					case 'f':ch='\f';break;
					case 'n':ch='\n';break;
					case 'r':ch='\r';break;
					case 't':ch='\t';break;
					case 'v':ch='\v';break;
					case '\n':
						str.remove_prefix(1);
						return "";
					case 'x':
					{
						str.remove_prefix(1);
						auto[jt,ec]=std::from_chars(str.begin(),str.end(),ch,16);
						if(ec!=std::errc{})
							return {'x'};
						str={jt-1,str.end()};
					}
					break;
					default:
					if('0'<=str.front()&&str.front()<'8')
					{
						str={std::from_chars
						(
							str.begin(),
							std::min(str.begin()+3,str.end()),
							ch,
							8
						).ptr-1,str.end()};
					}
					else ch=str.front();
				}
				str.remove_prefix(1);
			}
			return {static_cast<char>(ch)};
		} 
	}
	
}