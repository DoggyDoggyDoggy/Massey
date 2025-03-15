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
    "CONSTANT" to "SPACE|TAB|NEWLINE",
    "ID" to "[a-zA-Z][a-zA-Z0-9]*",
    "LITERAL" to "\"(\\\\.|[^\"\\\\])*\"",
    "PLUS" to "\\+",
    "END" to ";",
    "SKIP" to "[ \\t\\n]+",
    "MISMATCH" to "."
)

// Lexical analysis function, returns a list of tokens from the input string
fun lex(input: String): List<Token> {
    val tokens = mutableListOf<Token>()
    var pos = 0
    while (pos < input.length) {
        var matchFound = false
        for ((type, pattern) in tokenSpecs) {
            val regex = Regex("^$pattern")
            val matchResult = regex.find(input.substring(pos))
            if (matchResult != null) {
                matchFound = true
                val value = matchResult.value
                when (type) {
                    "SKIP" -> { /* Skip space */ }
                    "MISMATCH" -> throw Exception("Unrecognized character '$value' in position $pos")
                    else -> tokens.add(Token(type, value, pos))
                }
                pos += value.length
                break
            }
        }
        if (!matchFound) {
            throw Exception("Unrecognized character in position $pos")
        }
    }
    return tokens
}