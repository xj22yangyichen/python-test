#include "function.h"
#include "utility.h"
#include <format>
#include <utility>

namespace reverSilly::PythonInterpreter
{
	ScopeStack::Scope FunctionDefinition::accept(Arguments arg)const
	{
		ScopeStack::Scope res;
		if(arg.positional.size()>params.size())
			throw_program_error(std::format
			(
				"Too many positional parameters: expect {}, given {}",
				params.size(),arg.positional.size())
			);
		for(auto i=0u;i!=arg.positional.size();++i)
			res.insert({params[i],std::move(arg.positional[i])});
		for(auto &[k,v]:arg.named)
			if(auto[it,b]=res.insert({k,v});!b)
				throw_program_error(std::format("Multiple argument: {}",k));
		for(auto &[k,v]:defaults)
			res.insert({k,v});
		for(auto &name:params)
			if(!res.count(name))
				throw_program_error(std::format("argument not given: {}",name));
		if(res.size()>params.size())
			throw_program_error(std::format
			(
				"Too many parameters: expect {}, given {}",
				params.size(),res.size()
			));
		return res;
	}
}