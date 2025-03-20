package statements

import parser.Expr
import utils.extractWords
import kotlin.system.exitProcess

// Interface for operators (instructions)
interface Statement {
    fun execute(env: MutableMap<String, String>)
}

// Append statement: Appends the result of an expression to the contents of a variable
class AppendStatement(val id: String, val expr: Expr) : Statement {
    override fun execute(env: MutableMap<String, String>) {
        if (!env.containsKey(id))
            throw Exception("Variable '$id' is not defined.")
        env[id] = env[id]!! + expr.evaluate(env)
    }
}

// Set statement: sets the value of a variable to the result of an expression
class SetStatement(val id: String, val expr: Expr) : Statement {
    override fun execute(env: MutableMap<String, String>) {
        env[id] = expr.evaluate(env)
    }
}

// Print statement: prints the value of an expression
class PrintStatement(val expr: Expr) : Statement {
    override fun execute(env: MutableMap<String, String>) {
        println(expr.evaluate(env))
    }
}

// Printlength statement: prints the length of a string
class PrintLengthStatement(val expr: Expr) : Statement {
    override fun execute(env: MutableMap<String, String>) {
        val result = expr.evaluate(env)
        println("Length is: ${result.length}")
    }
}

// Printwords statement: prints individual words of a string
class PrintWordsStatement(val expr: Expr) : Statement {
    override fun execute(env: MutableMap<String, String>) {
        val result = expr.evaluate(env)
        val words = extractWords(result)
        println("Words are:")
        for (word in words) {
            println(word)
        }
    }
}

// Printwordcount statement: prints the number of words
class PrintWordCountStatement(val expr: Expr) : Statement {
    override fun execute(env: MutableMap<String, String>) {
        val result = expr.evaluate(env)
        val words = extractWords(result)
        println("Wordcount is: ${words.size}")
    }
}

// List statement: prints a list of variables and their values
class ListStatement : Statement {
    override fun execute(env: MutableMap<String, String>) {
        println("Identifier list (${env.size}):")
        for ((key, value) in env) {
            println("$key: \"$value\"")
        }
    }
}

// Reverse instruction: reverses the order of words in a variable string
class ReverseStatement(val id: String) : Statement {
    override fun execute(env: MutableMap<String, String>) {
        if (!env.containsKey(id))
            throw Exception("Variable '$id' is not defined.")
        val words = extractWords(env[id]!!)
        env[id] = words.reversed().joinToString(" ")
    }
}

// Exit instruction: terminates the interpreter
class ExitStatement : Statement {
    override fun execute(env: MutableMap<String, String>) {
        exitProcess(0)
    }
}