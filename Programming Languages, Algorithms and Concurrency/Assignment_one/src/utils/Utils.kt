package utils

fun printHeader() {
    println("----------------------------------------")
    println("159.341 Assignment 1 Semester 1 2025")
    println("Submitted by: Denys Pedan, 23011350")
    println("----------------------------------------")
}

// A function for extracting words from a string using a given pattern.
fun extractWords(text: String): List<String> {
    val regex = Regex("[A-Za-z0-9]+(?:['-][A-Za-z0-9]+)*")
    return regex.findAll(text).map { it.value }.toList()
}

//Function to handle a string literal: removes outer quotes and handles escaping
fun unescapeLiteral(literal: String): String {
    val inner = literal.substring(1, literal.length - 1)
    return inner.replace(Regex("""\\(.)""")) { matchResult ->
        matchResult.groupValues[1]
    }
}

// Function that finds the index of the character ';' that is outside a string literal
fun findStatementEnd(buffer: String): Int {
    var inLiteral = false
    var escape = false
    for (i in buffer.indices) {
        val c = buffer[i]
        if (inLiteral) {
            if (escape) {
                escape = false
            } else if (c == '\\') {
                escape = true
            } else if (c == '"') {
                inLiteral = false
            }
        } else {
            if (c == '"') {
                inLiteral = true
            } else if (c == ';') {
                return i
            }
        }
    }
    return -1
}