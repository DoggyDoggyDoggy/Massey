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

fun main() {
    printHeader()
}