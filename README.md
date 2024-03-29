# Spreadsheet
Проект: Электронная таблица

# Использование:

Проект позволяет создавать таблицу аналогично таблице Excel. В ячейки можно вводить информацию в виде текста, числа или формулы. В формулах можно задавать ссылки на другие ячейки. Расчет формул производится со любым унарным и бинарным оператором. 
Результаты можно вывести на консоль или в файл. Выводится как текстовое представление ячеек, так и числовое. 
При вводе значения в ячейке проверяется корректность ввода. В случае ошибки - пользователю выводится сообщение. 
При расчете значений ячейки также проверяется корректность данных в ячейках, в случае ошибки вычисления - пользователю выводится сообщение с кодом ошибки.

При расчете формулы используется дерево формулы из библиотеки ANTLR.

Запустить проект:
1. Добавить соответствующие команды в файл main.cpp по приведенному образцу. 
2. Собрать проект с использовавнием CMake - сформируется файл spreadsheet.exe. Используйте подготовенные для этого и размещенные в проекте CMakeLists.txt и FindANTLR.cmake.
3. Запустить файл spreadsheet.exe с вашим main.cpp.

# Системные требования:
1. С++17
2. GCC(MinGW-w64) 11.2.0
3. Комплект разработки для Java JDK https://www.oracle.com/java/technologies/downloads/
4. Библиотека ANTLR4 https://www.antlr.org/
5. CMake version 3.8+ https://cmake.org
