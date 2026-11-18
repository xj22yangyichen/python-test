#include "operators.h"
#include "int2048.h"
#include "utility.h"
#include "value.h"
#include <cmath>
#include <format>
#include <type_traits>

namespace reverSilly::PythonInterpreter
{
	Value operator+(Value const &a,Value const &b)
	{
		if(a.is<Tuple>()&&b.is<Tuple>())
			return {a.as<Tuple>()+b.as<Tuple>()};
		if(a.is<String>()&&b.is<String>())
			return {a.as<String>()+b.as<String>()};
		return do_promoted<Value>(a,b,[](auto &&a,auto &&b){return a+b;},"+");
	}
	Value operator-(Value const &a,Value const &b)
	{
		if(a.is<Tuple>()&&b.is<Tuple>())
			return {a.as<Tuple>()-b.as<Tuple>()};
		return do_promoted<Value>(a,b,[](auto &&a,auto &&b){return a-b;},"-");
	}
	std::string repeat(std::string s,int2048::Integer const &n)
	{
		std::string res;
		for(u32 i=0,len=n.len();i<len;(++i)<len?(s+=s,0):0)
			if(n.get_bit(i))
				res+=s;
		return res;
	}
	bool bool_or_int(Type t)
	{
		return t==Type::Boolean||t==Type::Integer;
	};
	Value operator*(Value const &a,Value const &b)
	{
		auto str=[](Value const &a,Value const &b)->Value
		{
			Integer bb=b.to<Integer>();
			if(bb<0)
				return {String("")};
			return {repeat(a.as<String>(),bb.data())};
		};
		if(a.is<Tuple>()&&b.is<Tuple>())
			return {a.as<Tuple>()*b.as<Tuple>()};
		if(a.type()==Type::String&&bool_or_int(b.type()))
			return str(a,b);
		if(bool_or_int(a.type())&&b.type()==Type::String)
			return str(b,a);
		return do_promoted<Value>(a,b,[](auto &&a,auto &&b){return a*b;},"*");
	}
	Float check_fzero(Float f)
	{
		if(f==0)
			throw_program_error("divided by zero");
		return f;
	}
	Value operator/(Value const &a,Value const &b)
	{
		if(a.is<Tuple>()&&b.is<Tuple>())
			return {a.as<Tuple>()/b.as<Tuple>()};
		if(!a.is_numeric()||!b.is_numeric())
			throw_program_error(std::format("not defined: {}/{}",a.type_name(),b.type_name()));
		return rethrow([&]{return Value{a.to<Float>()/check_fzero(b.to<Float>())};});
	}
	Value floor_division(const Value &a,const Value &b)
	{
		if(a.is<Tuple>()&&b.is<Tuple>())
			return {floor_division(a.as<Tuple>(),b.as<Tuple>())};
		return do_promoted<Value>(a,b,overloaded{[](Integer const &a,Integer const &b)
		{
			return rethrow([&]
			{
				return a/b;
			});
		},[](Float a,Float b)
		{
			return std::floor(a/check_fzero(b));
		}},"//");
	}
	Value operator%(const Value &a,const Value &b)
	{
		if(a.is<Tuple>()&&b.is<Tuple>())
			return {a.as<Tuple>()%b.as<Tuple>()};
		auto fmod=[](Float a,Float b)
		{
			check_fzero(b);
			Float r=std::fmod(a, b);
			if (r!=0&&((r<0)!=(b<0)))
				r+=b;
			return r;
		};
		return do_promoted<Value>(a,b,overloaded{[](Integer const &a,Integer const &b)
		{
			return rethrow([&]
			{
				return a%b;
			});
		},[fmod](Float a,Float b)
		{
			return fmod(a,b);
		}},"%");
	}
	Value operator+(const Value &a)
	{
		if(a.is<Tuple>())
			return {+a.as<Tuple>()};
		if(a.type()==Type::Boolean)
			return {a.to<Integer>()};
		if(a.is_numeric())
			return a;
		throw_program_error(std::format("unary operator+ not defined for type: {}",a.type_name()));
	}
	Value operator-(const Value &a)
	{
		if(a.is<Tuple>())
			return {-a.as<Tuple>()};
		if(a.type()==Type::Boolean)
			return {-a.to<Integer>()};
		return a.visit<Value>
		(
			[](Integer const &a)->Value{return {-a};},
			[](Float a)->Value{return {-a};},
			[](auto &&v)->Value
			{
				throw_program_error(std::format
				(
					"unary operator- not defined for type: {}",
					type_name<std::remove_cvref_t<decltype(v)>>()
				));
			}
		);
	}

	Tuple operator+(Tuple const &a,Tuple const &b)
	{
		return zip_transform(a,b,[](auto &a,auto &b){return a+b;});
	}
	Tuple operator-(Tuple const &a,Tuple const &b)
	{
		return zip_transform(a,b,[](auto &a,auto &b){return a-b;});
	}
	Tuple operator*(Tuple const &a,Tuple const &b)
	{
		return zip_transform(a,b,[](auto &a,auto &b){return a*b;});
	}
	Tuple operator/(Tuple const &a,Tuple const &b)
	{
		return zip_transform(a,b,[](auto &a,auto &b){return a/b;});
	}
	Tuple floor_division(Tuple const &a,Tuple const &b)
	{
		return zip_transform(a,b,[](auto &a,auto &b){return floor_division(a,b);});
	}
	Tuple operator%(Tuple const &a,Tuple const &b)
	{
		return zip_transform(a,b,[](auto &a,auto &b){return a%b;});
	}
	Tuple operator+(Tuple const &a)
	{
		return zip_transform(a,a,[](auto &a,auto &b){return +a;});
	}
	Tuple operator-(Tuple const &a)
	{
		return zip_transform(a,a,[](auto &a,auto &b){return -a;});
	}
}