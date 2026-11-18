#include "ScopeStack.h"
#include <stdexcept>
#include <utility>

namespace reverSilly::PythonInterpreter
{
	void ScopeStack::store(std::string const &name,Value v)
	{
		scopes.back().insert_or_assign(name,std::move(v));
	}
	void ScopeStack::assign(std::string name,Value v)
	{
		if(auto p=try_find(name);p)
			*p=std::move(v);
		else
			store(name,v);
	}
	Value const &ScopeStack::find(std::string const &name)const
	{
		if(auto p=try_find(name);p)
			return *p;
		throw std::out_of_range{"error finding variable:"+name};
	}
	Value &ScopeStack::find(std::string const &name)
	{
		return const_cast<Value&>(const_cast<ScopeStack const*>(this)->find(name));
	}
	Value const *ScopeStack::try_find(std::string const &name)const
	{
		if(auto it=scopes.back().find(name);it!=scopes.back().end())
			return &it->second;
		if(auto it=scopes.front().find(name);it!=scopes.front().end())
			return &it->second;
		return nullptr;
	}
	Value *ScopeStack::try_find(std::string const &name)
	{
		return const_cast<Value*>(const_cast<ScopeStack const*>(this)->try_find(name));
	}
	void ScopeStack::push(Scope s)
	{
		scopes.push_back(std::move(s));
	}
	void ScopeStack::push()	
	{
		scopes.emplace_back();
	}
	void ScopeStack::pop()
	{
		scopes.pop_back();
	}
}