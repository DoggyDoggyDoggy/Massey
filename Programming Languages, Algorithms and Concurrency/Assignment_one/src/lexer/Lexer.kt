package lexer

// Data for the token: type, value, and position for error messages
data class Token(val type: String, val value: String, val pos: Int)

val tokenSpecs: List<Pair<String, String>> = listOf(
    "PRINTLENGTH" to "printlength",
    "PRINTWORDS" to "printwords",
    "PRINTWORDCOUNT" to "printwordcount",
    "APPEND" to "append",
    "SET" to "set",
    "REVERSE" to "reverse",
    "LIST" to "list",
    "EXIT" to "exit",
    "PRINT" to "print",
    "SKIP" to "[ \\t\\n]+",
    "CONSTANT" to "\\b(?:SPACE|TAB|NEWLINE)\\b",
    "ID" to "[a-zA-Z][a-zA-Z0-9]*",
    "LITERAL" to "\"(\\\\.|[^\"\\\\])*\"",
    "PLUS" to "\\+",
    "END" to ";",
    "MISMATCH" to "."
)

// Lexical analysis function, returns a list of tokens from the input string
fun lex(input: String): List<Token> {
    val tokens = mutableListOf<Token>()
    var pos = 0
    while (pos < input.length) {
        var matchFound = false
        for ((type, pattern) in tokenSpecs) {
            val regex = if (type == "LITERAL")
                Regex("^$pattern", RegexOption.DOT_MATCHES_ALL)
            else
                Regex("^$pattern")
            val matchResult = regex.find(input.substring(pos))
            if (matchResult != null) {
                matchFound = true
                val value = matchResult.value
                when (type) {
                    "SKIP" -> { }
                    "MISMATCH" -> throw Exception("Unrecognized character '$value' at position $pos")
                    else -> tokens.add(Token(type, value, pos))
                }
                pos += value.length
                break
            }
        }
        if (!matchFound) {
            throw Exception("Unrecognized character at position $pos")
        }
    }
    return tokens
}