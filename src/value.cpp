#include "value.h"
#include "operators.h"
#include "utility.h"
#include <compare>
#include <string_view>
#include <typeinfo>
#include <format>

namespace reverSilly::PythonInterpreter 
{

#define X(a) case Type::a: return #a;
	std::string_view type_name(Type t)
	{
		switch(t)
		{
#define NOTHING
			PY_TYPES(NOTHING)
#undef NOTHING
		}
		throw_not_implemented(std::format("type_name not implemented for enum:{}",(int)t));
	}
#undef X

	Type common_type(Type a, Type b)
	{
		if(a==Type::Float||b==Type::Float)
			return Type::Float;
		return Type::Integer;
	}
	bool is_numeric(Type a)
	{
		return a==Type::Boolean||a==Type::Float||a==Type::Integer;
	}
	Value Value::to(Type t)const
	{
		switch(t)
		{
			case Type::None:return {to<None>()};
			case Type::Boolean:return {to<Boolean>()};
			case Type::Float:return {to<Float>()};
			case Type::Integer:return {to<Integer>()};
			case Type::String:return {to<String>()};
			case Type::Tuple:return {to<Tuple>()};
		}
		throw std::bad_cast{};
	}

	std::partial_ordering operator<=>(Value const &a,Value const &b)
	{
		if(a.is_numeric()&&b.is_numeric())
			return do_promoted<std::partial_ordering>(a,b,[](auto const &a,auto const &b){return a<=>b;},"<=>");
		if(a.type()==b.type())
		{
			if(a.is<None>())
				throw_program_error("Compare is not defined for None");
			return a.data<=>b.data;
		}
		throw_program_error(std::format("Compare is not defined for different types:{},{}",a.type_name(),b.type_name()));
	}
	bool operator==(Value const &a,Value const &b)
	{
		if(a.is_numeric()&&b.is_numeric())
			return do_promoted<bool>(a,b,[](auto const &a,auto const &b){return a==b;},"==");
		return a.data==b.data;
	}
}