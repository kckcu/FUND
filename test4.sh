#!/bin/bash

# Названия файлов
SOURCE_FILE="lab2.c"
EXEC_FILE="lab2"
INPUT_FILE="input.txt"

echo "Компиляция файла $SOURCE_FILE..."
gcc $SOURCE_FILE -o $EXEC_FILE

if [ $? -ne 0 ]; then
    echo "Ошибка компиляции! Тесты остановлены."
    exit 1
fi
echo "Компиляция прошла успешно."
echo "========================================="

# Создаем тестовый текстовый файл с разными типами символов
cat << 'EOF' > $INPUT_FILE
Hello World 123!
Привет мир 456?
Test_line... 789.
EOF

echo "СОЗДАН ВХОДНОЙ ФАЙЛ ($INPUT_FILE):"
cat $INPUT_FILE
echo "========================================="

echo -e "\n[--- БЛОК 1: ПРОВЕРКА ЗАЩИТЫ И ОШИБОК ПОЛЬЗОВАТЕЛЯ ---]"

echo -e "\nТест 1.1: Одинаковые имена файлов (Ожидается ошибка защиты данных)"
echo "Команда: ./$EXEC_FILE -nd $INPUT_FILE $INPUT_FILE"
./$EXEC_FILE -nd $INPUT_FILE $INPUT_FILE

echo -e "\nТест 1.2: Глубокая проверка (Один файл, но разные пути: 'input.txt' и './input.txt')"
echo "Команда: ./$EXEC_FILE -nd $INPUT_FILE ./$INPUT_FILE"
./$EXEC_FILE -nd $INPUT_FILE ./$INPUT_FILE

echo -e "\nТест 1.3: Флаг 'n' передан, но выходной файл забыли указать"
echo "Команда: ./$EXEC_FILE -nd $INPUT_FILE"
./$EXEC_FILE -nd $INPUT_FILE

echo -e "\nТест 1.4: Указан несуществующий флаг (-nx)"
echo "Команда: ./$EXEC_FILE -nx $INPUT_FILE output.txt"
./$EXEC_FILE -nx $INPUT_FILE output.txt

echo -e "\nТест 1.5: Файл для чтения не существует"
echo "Команда: ./$EXEC_FILE -d ghost_file.txt"
./$EXEC_FILE -d ghost_file.txt


echo -e "\n\n[--- БЛОК 2: ПРОВЕРКА КОРРЕКТНОЙ РАБОТЫ АЛГОРИТМОВ ---]"

echo -e "\nТест 2.1: Флаг -d (Удаление цифр, авто-имя)"
./$EXEC_FILE -d $INPUT_FILE
echo "Результат (out_$INPUT_FILE):"
cat out_$INPUT_FILE

echo -e "\n\nТест 2.2: Флаг -ni (Счетчик латинских букв, ручное имя 'res_i.txt')"
./$EXEC_FILE -ni $INPUT_FILE res_i.txt
echo "Результат (res_i.txt):"
cat res_i.txt

echo -e "\n\nТест 2.3: Флаг -s (Счетчик спецсимволов, авто-имя)"
./$EXEC_FILE -s $INPUT_FILE
echo "Результат (out_$INPUT_FILE):"
cat out_$INPUT_FILE

echo -e "\n\nТест 2.4: Флаг -na (ASCII HEX, ручное имя 'res_a.txt')"
./$EXEC_FILE -na $INPUT_FILE res_a.txt
echo "Результат (res_a.txt):"
cat res_a.txt

echo -e "\n\n========================================="
echo "Очистка временных файлов..."
rm $INPUT_FILE out_$INPUT_FILE res_i.txt res_a.txt 2>/dev/null
echo "Тесты завершены!"