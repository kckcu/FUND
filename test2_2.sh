#!/bin/bash

SOURCE_FILE="2_2.c"
EXEC_FILE="./lab2_2"

echo "=== Компиляция программы ==="
gcc -std=c99 -Wall -Wextra $SOURCE_FILE -o $EXEC_FILE
if [ $? -ne 0 ]; then
    echo "Ошибка компиляции! Тесты остановлены."
    exit 1
fi
echo "Успешно."
echo "========================================="

# Генерируем 3 разных входных файла

# Файл 1: Разные зарплаты + проверка сортировки по ID
cat << 'EOF' > input1.txt
3 John Doe 5000.50
1 Alice Smith 7000.00
2 Bob Builder 3000.00
4 John Doe 5000.50
EOF

# Файл 2: Одинаковые зарплаты, фамилии и имена (упор на сортировку по ID)
cat << 'EOF' > input2.txt
10 Jane Austen 4000.00
5 Jane Austen 4000.00
1 Jane Austen 4000.00
EOF

# Файл 3: Одинаковая зарплата, разные фамилии
cat << 'EOF' > input3.txt
2 Anna Zed 6000.00
1 Anna Adams 6000.00
3 Boris Adams 6000.00
EOF

# Файл с ошибкой: цифры в имени (чтобы проверить валидацию)
cat << 'EOF' > input_bad.txt
1 H4cker Man 5000.00
EOF

echo -e "\n[--- ТЕСТ 1: По возрастанию (-a) ---]"
echo "Входные данные (input1.txt):"
cat input1.txt
$EXEC_FILE input1.txt -a output1.txt
echo -e "\nРезультат (output1.txt):"
cat output1.txt

echo -e "\n\n[--- ТЕСТ 2: По убыванию (-d) ---]"
echo "Входные данные (input2.txt):"
cat input2.txt
$EXEC_FILE input2.txt -d output2.txt
echo -e "\nРезультат (output2.txt):"
cat output2.txt

echo -e "\n\n[--- ТЕСТ 3: Сложные дубликаты, сортировка по возрастанию (/a) ---]"
echo "Входные данные (input3.txt):"
cat input3.txt
$EXEC_FILE input3.txt /a output3.txt
echo -e "\nРезультат (output3.txt):"
cat output3.txt

echo -e "\n\n========================================="
echo "    ПРОВЕРКА ОТЛОВА ОШИБОК И ВАЛИДАЦИИ   "
echo "========================================="

echo -e "\nТест 4.1: Ошибка валидации данных (цифры в имени)"
$EXEC_FILE input_bad.txt -a output_bad.txt
echo "Код возврата: $?"

echo -e "\nТест 4.2: Ошибка флага (неизвестный флаг -x)"
$EXEC_FILE input1.txt -x output_test.txt
echo "Код возврата: $?"

echo -e "\nТест 4.3: Несуществующий входной файл"
$EXEC_FILE ghost.txt -a output_test.txt
echo "Код возврата: $?"

echo -e "\nТест 4.4: Недостаточно аргументов"
$EXEC_FILE input1.txt -a
echo "Код возврата: $?"

echo -e "\nОчистка временных файлов..."
rm input1.txt input2.txt input3.txt input_bad.txt output1.txt output2.txt output3.txt 2>/dev/null
echo "Тесты завершены!"