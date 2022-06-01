#include "CompilationEngine.h"
#include <cstdlib>
#include <iostream>

CompilationEngine::CompilationEngine(const std::string& writeFile, JackTokenizer& pJackTokenizer)
	:
	of{ writeFile, std::ios_base::out | std::ios_base::trunc },
	jackTokenizer{pJackTokenizer},
	leftOffset{} // TODO make beatiful offset from the left side, this counter should increasing every time when we compile something nested and comeback after this
{
}

void CompilationEngine::compile()
{
	compileClass();
}

void CompilationEngine::putTokensInFileAsXML()
{
	of << "<tokens>\n";

	while (jackTokenizer.isThereAnyToken())
	{
		const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

		of << '<' << tokenTypeStr << ">" << tokenValue << "</" << tokenTypeStr << ">\n";
		++jackTokenizer;
	}

	of << "</tokens>\n";
}

void CompilationEngine::eatValue(const std::string& str)
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

	if (str == tokenValue)
	{
		of << '<' << tokenTypeStr << ">" << tokenValue << "</" << tokenTypeStr << ">\n";
		++jackTokenizer;
	}
	else
	{
		std::cerr << "Can't eat " << str << " cause it's not token value\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::eatId()
{
	eatToken(JackTokenizer::ETOKEN::IDENTIFIER);
}

void CompilationEngine::eatToken(const JackTokenizer::ETOKEN token)
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

	if (token == jackTokenizer.getCurrTokenType())
	{
		of << '<' << tokenTypeStr << ">" << tokenValue << "</" << tokenTypeStr << ">\n";
		++jackTokenizer;
	}
	else
	{
		std::cerr << "Can't eat " << jackTokenizer.getTokenString(token) << " cause it's not token type\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::eatType()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;
	const auto token = jackTokenizer.getCurrTokenType();
	
	if (isType(tokenValue, token))
	{
		of << '<' << tokenTypeStr << ">" << tokenValue << "</" << tokenTypeStr << ">\n";
		++jackTokenizer;
	}
	else
	{
		std::cerr << "Can't eat type: " << tokenValue << '\n';
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::eatOneOfThem(std::initializer_list<std::string> list)
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;


	for (const auto& str : list)
	{
		if (tokenValue == str)
		{
			of << '<' << tokenTypeStr << ">" << tokenValue << "</" << tokenTypeStr << ">\n";
			++jackTokenizer;
			return;
		}
	}

	std::cerr << "before die tokenValue: " << tokenValue << "\n";


	std::cerr << "Can't eat one of ";
	for (const auto& str : list)
	{
		std::cerr << str << " ";
	}
	std::cerr << "\n";

	std::cerr << "Terminating...\n";
	std::exit(1);
}

void CompilationEngine::eatOptionalVarNames()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;
	while (tokenValue == ",")
	{
		eatValue(tokenValue);
		eatId();

		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}
}

void CompilationEngine::eatFuncRetType()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;
	const auto token = jackTokenizer.getCurrTokenType();
	
	if (tokenValue == "void" || isType(tokenValue, token))
	{
		of << '<' << tokenTypeStr << ">" << tokenValue << "</" << tokenTypeStr << ">\n";
		++jackTokenizer;
	}
	else
	{
		std::cerr << "Can't eat type " << tokenValue << "\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::eatParamList()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	if (tokenValue != ")")
	{
		eatType();
		eatId();
		
		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}
		
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
		
		while (tokenValue != ")")
		{
			eatValue(",");
			eatType();
			eatId();

			if (!jackTokenizer.isThereAnyToken())
			{
				tokenizerEmptyOut();
			}

			std::tie(tokenValue, std::ignore) = *jackTokenizer;
		}
	}
}

void CompilationEngine::compileClass()
{
	of << "<class>\n";
	eatValue("class");
	eatId();
	eatValue("{");
	compileClassVarDec();
	compileSubroutine();
	eatValue("}");
	of << "</class>\n";
}

void CompilationEngine::compileClassVarDec()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	while (tokenValue == "static" || tokenValue == "field")
	{
		of << "<classVarDec>\n";

		eatOneOfThem({ "static", "field" });
		eatType();
		eatId();
		eatOptionalVarNames();
		eatValue(";");
		
		of << "</classVarDec>\n";

		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}
}

void CompilationEngine::compileSubroutine()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}
	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	while (tokenValue == "constructor"||
		tokenValue == "function"   ||
		tokenValue == "method"
		)
	{
		of << "<subroutineDec>\n";

		eatOneOfThem({ "constructor", "function", "method" });
		eatFuncRetType();
		eatId();
		eatValue("(");
		compileParameterList();
		eatValue(")");
		compileSubroutineBody();

		of << "</subroutineDec>\n";

		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}
}

void CompilationEngine::compileParameterList()
{
	of << "<parameterList>\n";

	eatParamList();
	
	of << "</parameterList>\n";
}

//there could be none of them, don't even show it up if it's empty
void CompilationEngine::compileVarDec()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;
		
	while (tokenValue == "var")
	{
		of << "<varDec>\n";
		eatValue("var");
		eatType();
		eatId();
		eatOptionalVarNames();
		eatValue(";");
		of << "</varDec>\n";

		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}

		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}

}

void CompilationEngine::compileSubroutineBody()
{
	of << "<subroutineBody>\n";
	
	eatValue("{");
	compileVarDec();
	compileStatements();
	eatValue("}");

	of << "</subroutineBody>\n";
}

void CompilationEngine::compileStatements()
{
	of << "<statements>\n";

	while (isThereAnyStatement())
		compileStatement();

	of << "</statements>\n";
}

bool CompilationEngine::isThereAnyStatement() const
{
	bool r = false;

	if (jackTokenizer.isThereAnyToken())
	{
		std::string tokenValue;
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
		r = isThatStatement(tokenValue);
	}

	return r;
}

bool CompilationEngine::isThatStatement(const std::string& str) const noexcept
{
	return str == "if" || str == "do" || str == "while" || str == "return" || str == "let";
}

void CompilationEngine::compileStatement()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	if (tokenValue == "if")
	{
		compileIf();
	}
	else if (tokenValue == "do")
	{
		compileDo();
	}
	else if (tokenValue == "while")
	{
		compileWhile();
	}
	else if (tokenValue == "let")
	{
		compileLet();
	}
	else if (tokenValue == "return")
	{
		compileReturn();
	}
	else
	{
		std::cerr << "Unrecognized statement: " << tokenValue << '\n';
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::subroutineCall()
{
	eatId();

	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;
	
	if (tokenValue == "(")
	{
		eatValue("(");
		compileExpressionList();
		eatValue(")");
	}
	else if (tokenValue == ".")
	{
		eatValue(".");
		eatId();
		eatValue("(");
		compileExpressionList();
		eatValue(")");
	}

}

void CompilationEngine::eatOptionalArraySubscript()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

	if (tokenValue == "[")
	{
		eatValue(tokenValue);
		compileExpression();
		eatValue("]");
	}
}

void CompilationEngine::eatOptionalExpression()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

	if (tokenValue != ";")
	{
		compileExpression();
	}
}

void CompilationEngine::eatOptionalElse()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}
	
	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

	if (tokenValue == "else")
	{
		eatValue(tokenValue);
		eatValue("{");
		compileStatements();
		eatValue("}");
	}
}

void CompilationEngine::compileOptionalOpTerm()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	while(isOp(tokenValue))
	{
		eatValue(tokenValue);
		compileTerm();
		
		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}
		
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}
}


void CompilationEngine::compileDo()
{
	of << "<doStatement>\n";
	eatValue("do");
	subroutineCall();
	eatValue(";");
	of << "</doStatement>\n";
}

void CompilationEngine::compileLet()
{
	of << "<letStatement>\n";
	eatValue("let");
	eatId();
	eatOptionalArraySubscript();
	eatValue("=");
	compileExpression();
	eatValue(";");
	of << "</letStatement>\n";
}

void CompilationEngine::compileWhile()
{
	of << "<whileStatement>\n";
	eatValue("while");
	eatValue("(");
	compileExpression();
	eatValue(")");
	eatValue("{");
	compileStatements();
	eatValue("}");
	of << "</whileStatement>\n";
}

void CompilationEngine::compileReturn()
{
	of << "<returnStatement>\n";
	eatValue("return");
	eatOptionalExpression();
	eatValue(";");
	of << "</returnStatement>\n";
}

void CompilationEngine::compileIf()
{
	of << "<ifStatement>\n";
	eatValue("if");
	eatValue("(");
	compileExpression();
	eatValue(")");
	eatValue("{");
	compileStatements();
	eatValue("}");
	eatOptionalElse();
	of << "</ifStatement>\n";
}

void CompilationEngine::compileExpression()
{
	of << "<expression>\n";
	compileTerm();
	compileOptionalOpTerm();
	of << "</expression>\n";
}

void CompilationEngine::compileTerm()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	of << "<term>\n";
	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;
	const auto token = jackTokenizer.getCurrTokenType();

	if (token == JackTokenizer::ETOKEN::IDENTIFIER)
	{
		eatId();

		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}

		const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

		if (tokenValue == "[")
		{
			eatValue("[");
			compileExpression();
			eatValue("]");
		} 
		else if (tokenValue == "(")
		{
			eatValue("(");
			compileExpressionList();
			eatValue(")");
		}
		else if (tokenValue == ".")
		{
			eatValue(".");
			eatId();
			eatValue("(");
			compileExpressionList();
			eatValue(")");
		}
	}
	else if (token == JackTokenizer::ETOKEN::INTEGER_CONSTANT || token == JackTokenizer::ETOKEN::STRING_CONSTANT||
		tokenValue == "true" || tokenValue == "false" || tokenValue == "null" || tokenValue == "this")
	{
		of << '<' << tokenTypeStr << ">" << tokenValue << "</" << tokenTypeStr << ">\n";
		++jackTokenizer;
	}
	else if (tokenValue == "(")
	{
		eatValue("(");
		compileExpression();
		eatValue(")");
	}
	else if (tokenValue == "-" || tokenValue == "~")
	{
		eatValue(tokenValue);
		compileTerm();
	}
	else
	{
		std::cerr << "Not valid token: " << tokenValue << " for term\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}

	of << "</term>\n";
}

void CompilationEngine::compileExpressionList()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;


	of << "<expressionList>\n";
		
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}

	std::tie(tokenValue, std::ignore) = *jackTokenizer;
	while (tokenValue != ")")
	{
		if(tokenValue == ",")
		eatValue(",");

		compileExpression();
			
		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}

		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}

	of << "</expressionList>\n";
}

void CompilationEngine::tokenizerEmptyOut()
{
	std::cerr << "Can't eat, cause tokenizer empty\n";
	std::cerr << "Terminating...\n";
	std::exit(1);
}

bool CompilationEngine::isOp(const std::string& str)
{	
	return str == "+" || str == "-" || str == "*"
		|| str == "/" || str == "&amp;" || str == "|"
		|| str == "&lt;" || str == "&gt;" || str == "=";
}

bool CompilationEngine::isType(const std::string& tokenValue, const JackTokenizer::ETOKEN tokenType) const noexcept
{
	return tokenValue == "int" || tokenValue == "char"
		|| tokenValue == "boolean" 
		|| tokenType == JackTokenizer::ETOKEN::IDENTIFIER;
}
