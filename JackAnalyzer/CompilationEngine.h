#pragma once

#include "JackTokenizer.h"

struct CompilationEngine
{
	CompilationEngine(const std::string& writeFile, JackTokenizer& pJackTokenizer);

	//should be called one of those functions
	void compile();
	void putTokensInFileAsXML();

private:

	void eatValue(const std::string& str);
	void eatId();
	void eatToken(const JackTokenizer::ETOKEN token);
	void eatType();
	void eatOneOfThem(std::initializer_list<std::string> list);

	void eatOptionalVarNames();

	void eatFuncRetType();
	void eatParamList();

	void compileClass(); // class first token
	void compileClassVarDec(); // static or field first token
	void compileSubroutine(); // constructor | function | method first token 
	void compileParameterList(); // ( ) could be empty inside but starts and end with parentheses
	void compileVarDec(); // start with var

	void compileSubroutineBody();

	void compileStatements(); // this is after if, while, function, method, constructor bodies

	bool isThereAnyStatement() const;
	bool isThatStatement(const std::string& str) const noexcept;
	void compileStatement();
	void subroutineCall();

	void eatOptionalArraySubscript();
	void eatOptionalExpression();
	void eatOptionalElse();
	void compileOptionalOpTerm();

	void compileDo(); // 'do' start
	void compileLet(); // 'let' start
	void compileWhile(); // 'while' start
	void compileReturn(); // 'return' start

	void compileIf(); // 'if' start
	void compileExpression(); //actually as it everytime inside of something we exactly know where to use it, so it won't be empty at all
	void compileTerm();
	void compileExpressionList();

	void tokenizerEmptyOut();

	bool isOp(const std::string& str);
	bool isType(const std::string& tokenValue, const JackTokenizer::ETOKEN tokenType) const noexcept;

	std::ofstream of;
	JackTokenizer& jackTokenizer;
	std::size_t leftOffset;

};

