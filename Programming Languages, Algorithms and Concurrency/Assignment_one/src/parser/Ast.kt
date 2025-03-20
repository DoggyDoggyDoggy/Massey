package parser

// Interface for expressions
interface Expr {
    fun evaluate(env: MutableMap<String, String>): String
}

// Class for string literal
class Literal(val value: String) : Expr {
    override fun evaluate(env: MutableMap<String, String>) = value
}

// Class for constants: SPACE, TAB, NEWLINE
class Constant(val name: String) : Expr {
    override fun evaluate(env: MutableMap<String, String>): String = when (name) {
        "SPACE" -> " "
        "TAB" -> "\t"
        "NEWLINE" -> "\n"
        else -> ""
    }
}

// Class for identifier (variable)
class Identifier(val name: String) : Expr {
    override fun evaluate(env: MutableMap<String, String>): String =
        env[name] ?: throw Exception("Variable '$name' is not defined.")
}

// Binary expression for concatenation operation (+ operation)
class BinaryExpr(val left: Expr, val right: Expr) : Expr {
    override fun evaluate(env: MutableMap<String, String>) = left.evaluate(env) + right.evaluate(env)
}