import lexer.lex
import parser.Parser
import utils.printHeader


fun main() {
    printHeader()
    val env = mutableMapOf<String, String>()  // Symbol table
    var buffer = ""
    var prompt = "> "
    while (true) {
        print(prompt)
        val line = readLine() ?: break
        buffer += line + "\n"
        while (buffer.contains(";")) {
            val index = buffer.indexOf(";")
            var stmtText = buffer.substring(0, index).trim()
            buffer = buffer.substring(index + 1)
            if (stmtText.isEmpty()) continue
            stmtText += ";"

            try {
                val tokens = lex(stmtText)
                val parser = Parser(tokens)
            } catch (e: Exception) {
                println("Error: ${e.message}")
            }
        }
    }
}