#!/bin/bash

SOURCE_FILE="4.c"
EXEC_FILE="4"
INPUT_FILE="input.txt"

echo "Компиляция файла $SOURCE_FILE..."
gcc $SOURCE_FILE -o $EXEC_FILE

if [ $? -ne 0 ]; then
    echo "Ошибка компиляции! Тесты остановлены."
    exit 1
fi
echo "Компиляция прошла успешно."

# Создаем тестовый файл
cat << 'EOF' > $INPUT_FILE
Hello World 123!
Привет мир 456?
Test_line... 789.
EOF

echo "========================================="
echo "СОЗДАН ВХОДНОЙ ФАЙЛ ($INPUT_FILE):"
cat $INPUT_FILE
echo "========================================="

echo -e "\n[--- ТЕСТ 1: Флаг -d (Удаление цифр, авто-имя) ---]"
./$EXEC_FILE -d $INPUT_FILE
echo "Результат в файле out_$INPUT_FILE:"
cat out_$INPUT_FILE

echo -e "\n\n[--- ТЕСТ 2: Флаг -nd (Удаление цифр, свое имя) ---]"
./$EXEC_FILE -nd $INPUT_FILE res_d.txt
echo "Результат в файле res_d.txt:"
cat res_d.txt

echo -e "\n\n[--- ТЕСТ 3: Флаг -i (Счетчик латинских букв, авто-имя) ---]"
./$EXEC_FILE -i $INPUT_FILE
echo "Результат в файле out_$INPUT_FILE:"
cat out_$INPUT_FILE

echo -e "\n\n[--- ТЕСТ 4: Флаг -ni (Счетчик латинских букв, свое имя) ---]"
./$EXEC_FILE -ni $INPUT_FILE res_i.txt
echo "Результат в файле res_i.txt:"
cat res_i.txt

echo -e "\n\n[--- ТЕСТ 5: Флаг -s (Счетчик спецсимволов, авто-имя) ---]"
# Считает все, кроме латиницы, цифр, пробелов и переносов строк. 
# (Кириллица тоже считается как спецсимвол в рамках этой логики)
./$EXEC_FILE -s $INPUT_FILE
echo "Результат в файле out_$INPUT_FILE:"
cat out_$INPUT_FILE

echo -e "\n\n[--- ТЕСТ 6: Флаг -na (ASCII HEX, свое имя) ---]"
./$EXEC_FILE -na $INPUT_FILE res_a.txt
echo "Результат в файле res_a.txt:"
cat res_a.txt

echo -e "\n\n========================================="
echo "Очистка временных файлов..."
rm $INPUT_FILE out_$INPUT_FILE res_d.txt res_i.txt res_a.txt
echo "Все тесты успешно завершены!"