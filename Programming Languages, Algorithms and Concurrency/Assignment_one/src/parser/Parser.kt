package parser

import lexer.Token
import statements.*
import utils.unescapeLiteral

class Parser(val tokens: List<Token>) {
    var pos = 0

    fun current(): Token? = if (pos < tokens.size) tokens[pos] else null

    fun eat(expectedType: String): Token {
        val token = current() ?: throw Exception("Expected token $expectedType, but end of input reached.")
        if (token.type != expectedType)
            throw Exception("Expected token $expectedType, and ${token.type} (${token.value}) is received at ${token.pos}")
        pos++
        return token
    }

    fun parseStatement() : Statement {
        val token = current() ?: throw Exception("No input data")
        return when (token.type) {
            "APPEND" -> {
                eat("APPEND")
                val idToken = eat("ID")
                val expr = parseExpression()
                eat("END")
                AppendStatement(idToken.value, expr)
            }

            "SET" -> {
                eat("SET")
                val idToken = eat("ID")
                val expr = parseExpression()
                eat("END")
                SetStatement(idToken.value, expr)
            }

            "PRINT" -> {
                eat("PRINT")
                val expr = parseExpression()
                eat("END")
                PrintStatement(expr)
            }

            "PRINTLENGTH" -> {
                eat("PRINTLENGTH")
                val expr = parseExpression()
                eat("END")
                PrintLengthStatement(expr)
            }

            "PRINTWORDS" -> {
                eat("PRINTWORDS")
                val expr = parseExpression()
                eat("END")
                PrintWordsStatement(expr)
            }

            "PRINTWORDCOUNT" -> {
                eat("PRINTWORDCOUNT")
                val expr = parseExpression()
                eat("END")
                PrintWordCountStatement(expr)
            }

            "LIST" -> {
                eat("LIST")
                eat("END")
                ListStatement()
            }

            "REVERSE" -> {
                eat("REVERSE")
                val idToken = eat("ID")
                eat("END")
                ReverseStatement(idToken.value)
            }

            "EXIT" -> {
                eat("EXIT")
                eat("END")
                ExitStatement()
            }

            else -> throw Exception("Unknown instruction starting with token ${token.type}")
        }
    }

    //Parsing an expression: expression := value { plus value }
    fun parseExpression(): Expr {
        var left = parseValue()
        while (current()?.type == "PLUS") {
            eat("PLUS")
            val right = parseValue()
            left = BinaryExpr(left, right)
        }
        return left
    }

    // Parse the value: value := id | constant | literal
    fun parseValue(): Expr {
        val token = current() ?: throw Exception("Unexpected end of input in expression.")
        return when (token.type) {
            "ID" -> {
                eat("ID")
                Identifier(token.value)
            }
            "CONSTANT" -> {
                eat("CONSTANT")
                Constant(token.value)
            }
            "LITERAL" -> {
                eat("LITERAL")
                Literal(unescapeLiteral(token.value))
            }
            else -> throw Exception("Unexpected token in expression: ${token.type} (${token.value})")
        }
    }
}