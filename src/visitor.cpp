#include "visitor.h"
#include "ControlFlow.h"
#include "Python3Parser.h"
#include "Python3ParserBaseVisitor.h"
#include "Python3ParserVisitor.h"
#include "ScopeStack.h"
#include "builtins.h"
#include "function.h"
#include "parse_number.h"
#include "tree/TerminalNode.h"
#include "utility.h"
#include "value.h"
#include "escape.h"
#include "operators.h"
#include <any>
#include <format>
#include <ranges>
#include <utility>
#include <variant>

namespace reverSilly::PythonInterpreter
{
	Visitor::Visitor(): ::Python3ParserBaseVisitor()
	{
		inject_builtins(table);
	}

	Value Visitor::evaluate(ParseTree *p)
	{
		return std::any_cast<Value>(visit(p));
	}
	ControlFlow Visitor::execute(ParseTree *p)
	{
		return std::any_cast<ControlFlow>(visit(p));
	}
	std::any Visitor::visitChildren(ParseTree *t)
	{
		throw_not_implemented(std::format
		(
			"the visitor is not implemented on '{}'",
			typeid(*t).name()
		));
	}

	std::any Visitor::visitAtom(Python3Parser::AtomContext *ctx)
	{
		if(ctx->NUMBER())
			return parse_number(ctx->NUMBER()->getText());
		if(ctx->NAME())
		{
			return rethrow([&]
			{
				return stk.find(ctx->NAME()->getText());
			});
		}
		if(!ctx->STRING().empty())
		{
			String res;
			for(auto &i:ctx->STRING())
				res+=parse_string(i->getText(),default_escape);
			return Value{res};
		}
		if(ctx->NONE())
			return Value{};
		if(ctx->TRUE())
			return Value{Boolean{true}};
		if(ctx->FALSE())
			return Value{Boolean{false}};
		if(ctx->test())
			return evaluate(ctx->test());
		if(ctx->format_string())
			return format(ctx->format_string());
		throw_not_implemented("not implemented");
	}
	Value Visitor::format(Python3Parser::Format_stringContext *ctx)
	{
		auto f_escape=[](std::string_view &str)->std::string
		{
			if(char ch=str.front();str.starts_with("{{")||str.starts_with("}}"))
			{
				str.remove_prefix(2);
				return {ch};
			}
			return default_escape(str);
		};
		String res;
		for(auto child:ctx->children)
		{
			if(auto *tl=dynamic_cast<Python3Parser::TestlistContext*>(child))
				res+=evaluate(tl).to<String>();
			else if(auto term=dynamic_cast<antlr4::tree::TerminalNode*>(child))
			{
				if(term->getSymbol()->getType()==Python3Parser::FORMAT_STRING_LITERAL)
					res+=escape(term->getText(),f_escape);
			}
		}
		return {std::move(res)};
	}

	std::any Visitor::visitTestlist(Python3Parser::TestlistContext *ctx)
	{
		if(ctx->COMMA().empty())
			return visit(ctx->test(0));
		return Value{visitTuple(ctx)};
	}
	Tuple Visitor::visitTuple(Python3Parser::TestlistContext *ctx)
	{
		Tuple res;
		for(auto i:ctx->test())
			res.emplace_back(evaluate(i));
		return res;
	}
	std::any Visitor::visitTest(Python3Parser::TestContext *ctx)
	{
		return visit(ctx->or_test());
	}

	std::any Visitor::visitOr_test(Python3Parser::Or_testContext *ctx)
	{
		if(auto sub=ctx->and_test();sub.size()==1)
			return visit(sub.front());
		else for(auto i:sub)
			if(evaluate(i).to<Boolean>())
				return Value{true};
		return Value{false};
	}
	std::any Visitor::visitAnd_test(Python3Parser::And_testContext *ctx)
	{
		if(auto sub=ctx->not_test();sub.size()==1)
			return visit(sub.front());
		else for(auto i:sub)
			if(!evaluate(i).to<Boolean>())
				return Value{false};
		return Value{true};
	}
	std::any Visitor::visitNot_test(Python3Parser::Not_testContext *ctx)
	{
		if(!ctx->not_test())
			return visit(ctx->comparison());
		return Value{!evaluate(ctx->not_test()).to<Boolean>()};
	}

	std::any Visitor::visitComparison(Python3Parser::ComparisonContext *ctx)
	{
		auto expr=ctx->arith_expr();
		auto ops=ctx->comp_op();
		if(ops.empty())
			return visit(expr.front());
		auto it=expr.begin();
		auto jt=ops.begin();
		Value last{evaluate(*(it++))};
		for(;it!=expr.end()&&jt!=ops.end();++it,++jt)
		{
			Value cur=evaluate(*it);
			auto op=*jt;
			if(op->LESS_THAN()&&!(last<cur))
				return Value{false};
			if(op->GREATER_THAN()&&!(last>cur))
				return Value{false};
			if(op->EQUALS()&&last!=cur)
				return Value{false};
			if(op->GT_EQ()&&!(last>=cur))
				return Value{false};
			if(op->LT_EQ()&&!(last<=cur))
				return Value{false};
			if(op->NOT_EQ_2()&&last==cur)
				return Value{false};
			last=std::move(cur);
		}
		return Value{true};
	}

	std::any Visitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx)
	{
		auto terms=ctx->term();
		auto ops=ctx->addorsub_op();
		auto it=terms.begin();
		auto jt=ops.begin();
		Value acc{evaluate(*(it++))};
		for(;it!=terms.end()&&jt!=ops.end();++it,++jt)
		{
			auto op=*jt;
			if(op->ADD())
				acc=acc+evaluate(*it);
			if(op->MINUS())
				acc=acc-evaluate(*it);
		}
		return acc;
	}

	std::any Visitor::visitTerm(Python3Parser::TermContext *ctx)
	{
		auto factors=ctx->factor();
		auto ops=ctx->muldivmod_op();
		auto it=factors.begin();
		auto jt=ops.begin();
		Value product{evaluate(*(it++))};
		for(;it!=factors.end()&&jt!=ops.end();++it,++jt)
		{
			auto op=*jt;
			Value cur{evaluate(*it)};
			if(op->STAR())
				product=product*cur;
			if(op->DIV())
				product=product/cur;
			if(op->IDIV())
				product=floor_division(product,cur);
			if(op->MOD())
				product=product%cur;
		}
		return product;
	}

	std::any Visitor::visitFactor(Python3Parser::FactorContext *ctx)
	{
		if(ctx->atom_expr())
			return visit(ctx->atom_expr());
		if(auto fac=ctx->factor();ctx->ADD())
			return +evaluate(fac);
		else if(ctx->MINUS())
			return -evaluate(fac);
		throw_not_implemented("neither +/- ");
	}

	std::any Visitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx)
	{
		if(!ctx->trailer())
			return visit(ctx->atom());
		if(!ctx->atom()->NAME())
			throw_program_error(std::format("function call not by name: {}",ctx->atom()->getText()));
		auto const&fun=rethrow([this,ctx]()->auto&
		{
			return table.at(ctx->atom()->NAME()->getText());
		});
		return fun(parse_arguments(ctx->trailer()->arglist()));
	}
	Arguments Visitor::parse_arguments(Python3Parser::ArglistContext *ctx)
	{
		Arguments args;
		if(ctx) for(auto i:ctx->argument())
		{
			if(i->ASSIGN())
			{
				std::string name=i->test(0)->getText();
				if(auto[_,b]=args.named.insert({name,evaluate(i->test(1))});!b)
					throw_program_error(std::format("duplicated named arguments: {}",name));
			}
			else
				args.positional.push_back(evaluate(i->test().front()));
		}
		return args;
	}

	std::any Visitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx)
	{
		if(auto aug=ctx->augassign();aug)
		{
			Tuple lhs=visitTuple(ctx->testlist(0));
			Tuple rhs=visitTuple(ctx->testlist(1));
			if(lhs.size()!=rhs.size())
				throw_program_error("augassign");
			if(aug->ADD_ASSIGN())
				lhs=lhs+rhs;
			if(aug->DIV_ASSIGN())
				lhs=lhs/rhs;
			if(aug->IDIV_ASSIGN())
				lhs=floor_division(lhs,rhs);
			if(aug->MOD_ASSIGN())
				lhs=lhs%rhs;
			if(aug->MULT_ASSIGN())
				lhs=lhs*rhs;
			if(aug->SUB_ASSIGN())
				lhs=lhs-rhs;
			assign(ctx->testlist(0),std::move(lhs));
		}
		else if(!ctx->ASSIGN().empty())
		{
			auto test=ctx->testlist();
			auto it=test.rbegin();
			Value rhs{evaluate(*(it++))};
			if(!rhs.is<Tuple>())
				rhs={Tuple{rhs}};
			for(;it!=test.rend();++it)
				assign(*it,rhs.as<Tuple>());
		}
		else evaluate(ctx->testlist(0));
		//why function return values are considered tuple in assign, but not in 
		return ControlFlow{};
	}
	void Visitor::assign(Python3Parser::TestlistContext* names,Tuple tup)
	{
		auto test=names->test();
		if(tup.size()!=test.size())
			throw_program_error(std::format("unpack size not match: expected {}, got {}",tup.size(),test.size()));
		else
		{
			auto it=tup.begin();
			auto jt=test.begin();
			for(;it!=tup.end()&&jt!=test.end();++it,++jt)
				stk.assign((*jt)->getText(),*it);
		}
	}

	std::any Visitor::visitStmt(Python3Parser::StmtContext *ctx)
	{
		if(auto simple=ctx->simple_stmt())
			return visit(simple);
		if(auto compound=ctx->compound_stmt())
			return visit(compound);
		throw_not_implemented("not implemented statement");
	}
	std::any Visitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx)
	{
		return visit(ctx->small_stmt());
	}
	std::any Visitor::visitSmall_stmt(Python3Parser::Small_stmtContext *ctx)
	{
		if(auto expr=ctx->expr_stmt())
			return visit(expr);
		if(auto flow=ctx->flow_stmt())
			return visit(flow);
		throw_not_implemented("not implemented Small_stmt");
	}

	std::any Visitor::visitFile_input(Python3Parser::File_inputContext *ctx)
	{
		for(auto i:ctx->stmt())
			visit(i);
		return ControlFlow{};
	}

	std::any Visitor::visitBreak_stmt(Python3Parser::Break_stmtContext *ctx)
	{
		return ControlFlow{ControlFlow::Break{}};
	}
	std::any Visitor::visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx)
	{
		return ControlFlow{ControlFlow::Continue{}};
	}
	std::any Visitor::visitReturn_stmt(Python3Parser::Return_stmtContext *ctx)
	{
		if(auto tl=ctx->testlist())
			return ControlFlow{ControlFlow::Return{evaluate(tl)}};
		else
			return ControlFlow{ControlFlow::Return{}};
	}

	std::any Visitor::visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx)
	{
		return visit(ctx->children.front());
	}
	std::any Visitor::visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx)
	{
		return visit(ctx->children.front());
	}

	std::any Visitor::visitSuite(Python3Parser::SuiteContext *ctx)
	{
		if(auto simple=ctx->simple_stmt())
			return visit(simple);
		for(auto i:ctx->stmt())
		{
			if(auto res=execute(i);!std::get_if<ControlFlow::Normal>(&res.data))
				return res;
		}
		return ControlFlow{ControlFlow::Normal{}};
	}

	std::any Visitor::visitIf_stmt(Python3Parser::If_stmtContext *ctx)
	{
		auto test=ctx->test();
		auto suite=ctx->suite();
		auto it=test.begin();
		auto jt=suite.begin();
		for(;it!=test.end()&&jt!=suite.end();++it,++jt)
		{
			if(evaluate(*it).to<Boolean>())
				return execute(*jt);
		}
		if(jt!=suite.end())
			return execute(*jt);
		return ControlFlow{ControlFlow::Normal{}};
	}
	std::any Visitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx)
	{
		auto test=ctx->test();
		auto suite=ctx->suite();
		while(evaluate(test).to<Boolean>())
		{
			if(auto res=execute(suite);std::get_if<ControlFlow::Break>(&res.data))
				return ControlFlow{ControlFlow::Normal{}};
			else if(std::get_if<ControlFlow::Return>(&res.data))
				return res;
		}
		return ControlFlow{ControlFlow::Normal{}};
	}

	std::any Visitor::visitFuncdef(Python3Parser::FuncdefContext *ctx)
	{
		std::string name=ctx->NAME()->getText();
		FunctionDefinition def{.body=ctx->suite()};
		parse_parameter(def,ctx->parameters());
		auto func=[def=std::move(def),this](Arguments arg)->Value
		{
			ScopeGuard guard(stk,def.accept(std::move(arg)));
			auto res=execute(def.body);
			if(auto i=std::get_if<ControlFlow::Return>(&res.data))
				return std::move(i->value);
			else if(auto i=std::get_if<ControlFlow::Normal>(&res.data))
				return {};
			else throw_program_error("error break/continue at function level");
		};
		table.insert_or_assign(name,std::move(func));
		return ControlFlow{ControlFlow::Normal{}};
	}
	void Visitor::parse_parameter
		(FunctionDefinition &def,Python3Parser::ParametersContext *ctx)
	{
		if(auto params=ctx->typedargslist())
		{
			std::string lastparam;
			for(auto i:params->children)
			{
				if(auto name=dynamic_cast<Python3Parser::TfpdefContext*>(i))
				{
					lastparam=name->NAME()->getText();
					def.params.push_back(lastparam);
				}
				if(auto test=dynamic_cast<Python3Parser::TestContext*>(i))
				{
					auto[_,b]=def.defaults.insert({lastparam,evaluate(test)});
					if(!b)
						throw_program_error(std::format("multiple defaults for parameter {}",lastparam));
				}
			}
		}
	}
}