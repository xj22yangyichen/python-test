#pragma once
#include "Python3Parser.h"
#include "ScopeStack.h"
#include "value.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace reverSilly::PythonInterpreter
{
	struct Arguments
	{
		std::vector<Value> positional;
		std::unordered_map<std::string,Value> named;
	};
	using Function=std::function<Value(Arguments)>;
	using FunctionTable=std::unordered_map<std::string,Function>;

	struct FunctionDefinition
	{
		std::vector<std::string>params;
		std::unordered_map<std::string,Value>defaults;
		Python3Parser::SuiteContext *body;
		
		ScopeStack::Scope accept(Arguments arg)const;
	};
}