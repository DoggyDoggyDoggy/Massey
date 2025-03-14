package lexer

// Data for the token: type, value, and position for error messages
data class Token(val type: String, val value: String, val pos: Int)