#pragma once
#include <compare>
#include <format>
#include <string_view>
#include <type_traits>
#include <variant>
#include <string>
#include <vector>
#include <int2048.h>
#include <utility.h>

namespace reverSilly
{
	namespace PythonInterpreter
	{
		struct Value;
		using None=std::monostate;
		using Boolean=bool;
		using Float=double;
		using Integer=sjtu::int2048;
		using String=std::string;
		using Tuple=std::vector<Value>;
#define PY_TYPES(C) X(None) C X(Boolean) C X(Float) C X(Integer) C X(String) C X(Tuple)
#define COMMA ,
		enum class Type
		{
#define X(a) a
			PY_TYPES(COMMA)
#undef X
		};
		std::string_view type_name(Type t);
		template<typename T>
		constexpr std::string_view type_name()
		{
#define X(a) \
			if constexpr(std::is_same_v<T,a>)\
				return #a;
			PY_TYPES(else)
			else static_assert(false,"type not available");
#undef X
		}
		Type common_type(Type,Type);
		bool is_numeric(Type);

		struct Value
		{
#define X(a) a
			std::variant<PY_TYPES(COMMA)>data;
#undef X
#undef COMMA
			
			Type type()const
			{
				return static_cast<Type>(data.index());
			}
			std::string_view type_name()const
			{
				return PythonInterpreter::type_name(type());
			}
			template<typename T>
			bool is()const
			{
				return (bool)std::get_if<T>(&data);
			}
			bool is_numeric()const
			{
				return PythonInterpreter::is_numeric(type());
			}
			template<typename T>
			T &as()
			{
				return std::get<T>(data);
			}
			template<typename T>
			T const&as()const
			{
				return std::get<T>(data);
			}
			template<typename T>
			T to()const
			{
				return std::visit([](auto const &i)->T
				{
					if constexpr(std::is_convertible_v<decltype(i),T>)
						return static_cast<T>(i);
					else
					 	return conversion_not_defined<T>()(i);
				},data);
			}
			Value to(Type)const;
			template<typename R>
			R visit(auto ...F)const
			{
				return std::visit<R>(overloaded{F...},data);
			}
			template<typename R>
			R visit(auto ...F)
			{
				return std::visit<R>(overloaded{F...},data);
			}
			friend std::partial_ordering operator<=>(Value const&,Value const&);
			friend bool operator==(Value const&,Value const&);
			friend bool operator!=(Value const&,Value const&)=default;
			friend bool operator<(Value const&,Value const&)=default;
			friend bool operator<=(Value const&,Value const&)=default;
			friend bool operator>(Value const&,Value const&)=default;
			friend bool operator>=(Value const&,Value const&)=default;

		private:
			template<typename R>
			static auto conversion_not_defined()
			{
				return [](auto &&v)->R
				{
					throw_program_error(std::format
					(
						"cannot convert types: {}->{}",
						PythonInterpreter::type_name<std::remove_cvref_t<decltype(v)>>(),
						PythonInterpreter::type_name<std::remove_cvref_t<R>>()
					));
				};
			}
		};
		template<>
		inline String Value::to<String>()const
		{
			return visit<String>
			(
				[](None){return "None";},
				[](Boolean i){return i?"True":"False";},
				[](Float i){return std::to_string(i);},
				[](Integer const &i){return i.str();},
				[](String const &i){return i;},
				[](Tuple const &i)
				{
					String res("(");
					bool first=true;
					for(auto &j:i)
					{
						if(!first)
							res+=',';
						res+=j.to<String>();
						first=false;
					}
					res+=')';
					return res;
				}
			);
		}
		template<>
		inline Boolean Value::to<Boolean>()const
		{
			return visit<Boolean>
			(
				[](None){return false;},
				[](Boolean i){return i;},
				[](Float i){return i;},
				[](Integer const &i){return i!=0;},
				[](String const &i){return !i.empty();},
				[](Tuple const &i){return !i.empty();}
			);
		}
		template<>
		inline Float Value::to<Float>()const
		{
			return visit<Float>
			(
				[](Boolean a){return a;},
				[](Integer const &a){return a.to<Float>();},
				[](Float a){return a;},
				[](String const &a){return std::stod(a);},
				conversion_not_defined<Float>()
			);
		}
		template<>
		inline Integer Value::to<Integer>()const
		{
			return visit<Integer>
			(
				[](Boolean a){return a;},
				[](Integer const &a){return a;},
				[](Float a){return Integer(a);},//this is not so good. loss precision.
				[](String const&a){return Integer(a);},
				conversion_not_defined<Integer>()
			);
		}
	}
}