#pragma once
#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include "Python3Parser.h"
#include "ScopeStack.h"
#include "function.h"
#include "value.h"
#include "ControlFlow.h"
#include "Python3ParserBaseVisitor.h"

namespace reverSilly::PythonInterpreter
{
	class Visitor:public ::Python3ParserBaseVisitor 
	{
		FunctionTable table;
		ScopeStack stk;
		// TODO: override all methods of Python3ParserBaseVisitor
		using Value=reverSilly::PythonInterpreter::Value;
		using ParseTree=antlr4::tree::ParseTree;

		Value evaluate(ParseTree*);
		ControlFlow execute(ParseTree*);
		std::any visitChildren(ParseTree *t)override;
		std::any visitAtom(Python3Parser::AtomContext*)override;
		std::any visitTestlist(Python3Parser::TestlistContext*)override;
		std::any visitTest(Python3Parser::TestContext*)override;
		std::any visitOr_test(Python3Parser::Or_testContext*)override;
		std::any visitAnd_test(Python3Parser::And_testContext*)override;
		std::any visitNot_test(Python3Parser::Not_testContext*)override;
		std::any visitComparison(Python3Parser::ComparisonContext*)override;
		std::any visitArith_expr(Python3Parser::Arith_exprContext*)override;
		std::any visitTerm(Python3Parser::TermContext*)override;
		std::any visitFactor(Python3Parser::FactorContext*)override;
		std::any visitAtom_expr(Python3Parser::Atom_exprContext*)override;
		std::any visitExpr_stmt(Python3Parser::Expr_stmtContext*)override;
		std::any visitStmt(Python3Parser::StmtContext*)override;
		std::any visitSimple_stmt(Python3Parser::Simple_stmtContext*)override;
		std::any visitSmall_stmt(Python3Parser::Small_stmtContext*)override;
		std::any visitFile_input(Python3Parser::File_inputContext*)override;
		std::any visitBreak_stmt(Python3Parser::Break_stmtContext*)override;
		std::any visitContinue_stmt(Python3Parser::Continue_stmtContext*)override;
		std::any visitReturn_stmt(Python3Parser::Return_stmtContext*)override;
		std::any visitFlow_stmt(Python3Parser::Flow_stmtContext*)override;
		std::any visitCompound_stmt(Python3Parser::Compound_stmtContext*)override;
		std::any visitSuite(Python3Parser::SuiteContext*)override;
		std::any visitIf_stmt(Python3Parser::If_stmtContext*)override;
		std::any visitWhile_stmt(Python3Parser::While_stmtContext*)override;
		std::any visitFuncdef(Python3Parser::FuncdefContext*)override;

		Value format(Python3Parser::Format_stringContext*);
		void assign(Python3Parser::TestlistContext*,Tuple);
		Arguments parse_arguments(Python3Parser::ArglistContext*);
		void parse_parameter(FunctionDefinition&,Python3Parser::ParametersContext*);
		Tuple visitTuple(Python3Parser::TestlistContext*);
	public:
		Visitor();
	};
}


#endif//PYTHON_INTERPRETER_EVALVISITOR_H
