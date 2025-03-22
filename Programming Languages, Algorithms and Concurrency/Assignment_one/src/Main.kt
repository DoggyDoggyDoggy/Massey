import lexer.lex
import parser.Parser
import utils.findStatementEnd
import utils.printHeader


fun main() {
    printHeader()
    val env = mutableMapOf<String, String>()  // Symbol table: variable name -> string value
    var buffer = ""
    var prompt = "> "
    while (true) {
        print(prompt)
        val line = readLine() ?: break
        buffer += line + "\n"
        // While there is a ';' character in the buffer that is outside the string literal, we process the instruction
        while (true) {
            val index = findStatementEnd(buffer)
            if (index == -1) break
            var stmtText = buffer.substring(0, index).trim()
            buffer = buffer.substring(index + 1)
            if (stmtText.isEmpty()) continue
            stmtText += ";" // add ';' back for correct parsing
            try {
                val tokens = lex(stmtText)
                val parser = Parser(tokens)
                val stmt = parser.parseStatement()
                if (parser.pos != tokens.size) {
                    throw Exception("Extra tokens after correct instruction.")
                }
                stmt.execute(env)
            } catch (e: Exception) {
                println("Error: ${e.message}")
                // If an error occurs, we skip the remaining input until the next instruction
            }
        }
        prompt = "> "
    }
}
