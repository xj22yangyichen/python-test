#include "builtins.h"
#include <iostream>
#include <utility>

namespace reverSilly::PythonInterpreter
{
	namespace builtins
	{
		Value check_singlearg(Arguments&& arg)
		{
			static const FunctionDefinition single
			{
				.params={"x"},
				.defaults{},
				.body=nullptr
			};
			return single.accept(std::move(arg)).at("x");
		}
		template<typename T>
		Value convert(Arguments&& arg)
		{
			return rethrow([&arg]()
			{
				return Value{check_singlearg(std::move(arg)).to<T>()};
			});
		}
		Value int_(Arguments arg)
		{
			return convert<Integer>(std::move(arg));
		}
		Value float_(Arguments arg)
		{
			return convert<Float>(std::move(arg));
		}
		Value str_(Arguments arg)
		{
			return convert<String>(std::move(arg));
		}
		Value bool_(Arguments arg)
		{
			return convert<Boolean>(std::move(arg));
		}
		Value print_(Arguments arg)
		{
			String sep(" "),end("\n");
			if(auto it=arg.named.find("sep");it!=arg.named.end())
				sep=it->second.to<String>();
			if(auto it=arg.named.find("end");it!=arg.named.end())
				end=it->second.to<String>();
			bool first=true;
			for(auto &i:arg.positional)
			{
				if(!first)
					std::cout<<sep;
				std::cout<<i.to<String>();
				first=false;
			}
			std::cout<<end;
			return {};
		}
	}
	void inject_builtins(FunctionTable &s)
	{
#define X(sth) s.insert({#sth,std::function<Value(Arguments)>{builtins::sth ## _}})
		X(int);
		X(float);
		X(str);
		X(bool);
		X(print);
#undef X
	}
}