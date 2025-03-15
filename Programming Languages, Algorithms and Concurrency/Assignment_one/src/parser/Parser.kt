package parser

import lexer.Token

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

    fun parseStatement() {
        val token = current() ?: throw Exception("No input data")
        return when (token.type) {
            "APPEND" -> {

            }

            "SET" -> {

            }

            "PRINT" -> {

            }

            "PRINTLENGTH" -> {

            }

            "PRINTWORDS" -> {

            }

            "PRINTWORDCOUNT" -> {

            }

            "LIST" -> {

            }

            "REVERSE" -> {

            }

            "EXIT" -> {

            }

            else -> throw Exception("Unknown instruction starting with token ${token.type}")
        }
    }
}