#pragma once
#include "value.h"
#include "utility.h"
#include <format>
#include <stdexcept>

namespace reverSilly::PythonInterpreter
{
	template<typename R>
	R do_promoted(Value const &a,Value const &b,auto F,std::string_view op_name="arithmetic")
	{
		if(!a.is_numeric()||!b.is_numeric())
			throw_program_error(std::format
		("arithmetic operation not defined for types: {}{}{}",
			a.type_name(),op_name,b.type_name()));
		Type t=common_type(a.type(),b.type());
		if(t==Type::Integer)
			return R{F(a.to<Integer>(),b.to<Integer>())};
		if(t==Type::Float)
			return R{F(a.to<Float>(),b.to<Float>())};
		throw_not_implemented(std::format("{} not implemented for {}",op_name,type_name(t)));
	}

	Value operator+(Value const&,Value const&);
	Value operator-(Value const&,Value const&);
	Value operator*(Value const&,Value const&);
	Value operator/(Value const&,Value const&);
	Value floor_division(Value const&,Value const&);
	Value operator%(Value const&,Value const&);
	Value operator+(Value const&);
	Value operator-(Value const&);

	Tuple operator+(Tuple const&,Tuple const&);
	Tuple operator-(Tuple const&,Tuple const&);
	Tuple operator*(Tuple const&,Tuple const&);
	Tuple operator/(Tuple const&,Tuple const&);
	Tuple floor_division(Tuple const&,Tuple const&);
	Tuple operator%(Tuple const&,Tuple const&);
	Tuple operator+(Tuple const&);
	Tuple operator-(Tuple const&);

	Tuple zip_transform(Tuple const &a,Tuple const &b,auto &&F)
	{
		if(a.size()!=b.size())
			throw std::invalid_argument
		{std::format("tuple operation length not match: {} vs {}",a.size(),b.size())};
		auto it=a.begin(),jt=b.begin();
		Tuple res;
		res.reserve(a.size());
		for(;it!=a.end()&&jt!=b.end();++it,++jt)
			res.push_back(F(*it,*jt));
		return res;
	}
}