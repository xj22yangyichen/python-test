#pragma once
#include "value.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace reverSilly::PythonInterpreter
{
	struct ScopeStack
	{
		using Scope=std::unordered_map<std::string,Value>;
		std::vector<Scope>scopes;
		ScopeStack():scopes(1){}
		void store(std::string const &name,Value v);//in current stack
		void assign(std::string name,Value v);//search,else add in current stack
		Value const &find(std::string const &name)const;//current+global
		Value &find(std::string const &name);
		Value const *try_find(std::string const &name)const;
		Value *try_find(std::string const &name);
		void push();
		void push(Scope);
		void pop();
	};

	struct ScopeGuard
	{
		ScopeStack &p;
		ScopeGuard(ScopeStack &p):p{p}
		{
			p.push();
		}
		ScopeGuard(ScopeStack &p,ScopeStack::Scope s):p{p}
		{
			p.push(std::move(s));
		}
		~ScopeGuard()
		{
			p.pop();
		}
	};
}