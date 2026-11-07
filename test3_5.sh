#!/bin/bash

SOURCE="3_5.c"
EXEC="./3_5.out"
DB_FILE="test_db.txt"
TRACE_FILE="test_trace.txt"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo "Компиляция $SOURCE..."
gcc -std=c99 -Wall -Wextra $SOURCE -o $EXEC

if [ $? -ne 0 ]; then
    echo -e "${RED}Ошибка компиляции!${NC}"
    exit 1
fi
echo -e "${GREEN}Компиляция успешно завершена.${NC}\n"

# Создаем тестовую базу данных студентов
# Включаем одну "битую" запись (с цифрами в имени), чтобы проверить валидацию
cat << 'EOF' > $DB_FILE
101 Alice Smith GroupA 5 5 5 5 5
102 Bob Jones GroupB 4 4 4 4 4
103 Charlie Brown GroupA 3 3 3 3 3
104 Invalid123 Name GroupC 5 5 5 5 5
EOF

# Функция для тестирования
# $1 - Название теста
# $2 - Аргументы командной строки
# $3 - Интерактивный ввод (stdin)
# $4 - Ожидаемый вывод (stdout)
# $5 - Ожидаемый код возврата
run_test() {
    local test_name="$1"
    local args="$2"
    local input_data="$3"
    local expected_out="$4"
    local expected_ret="$5"

    echo -n "Тест: $test_name ... "
    
    # Очищаем файл трассировки перед каждым тестом (если он существует)
    rm -f $TRACE_FILE

    # Запускаем программу, передавая ввод и аргументы
    actual_out=$(echo -e "$input_data" | $EXEC $args 2>&1)
    actual_ret=$?

    if [ "$actual_ret" -eq "$expected_ret" ] && [[ "$actual_out" == *"$expected_out"* ]]; then
        echo -e "${GREEN}ПРОЙДЕН${NC}"
    else
        echo -e "${RED}ПРОВАЛ${NC}"
        echo "   Аргументы: $args"
        echo "   Ввод: $(echo -e "$input_data" | tr '\n' ' ')"
        echo "   Ожидалось в выводе: '$expected_out', статус $expected_ret"
        echo "   Получено: статус $actual_ret, вывод:"
        echo "$actual_out"
    fi
}

# --- Тесты обработки ошибок ---

run_test "Неверное количество аргументов" \
"" \
"" \
"Использование:" 1

run_test "Отсутствующий файл базы" \
"not_exist.txt $TRACE_FILE" \
"5" \
"Ошибка загрузки файла базы." 1

# --- Функциональные тесты ---

run_test "Вывод всех студентов (и проверка пропуска невалидного Invalid123)" \
"$DB_FILE $TRACE_FILE" \
"4\n5\n" \
"Charlie Brown" 0

run_test "Поиск по ID (успешный)" \
"$DB_FILE $TRACE_FILE" \
"1\n101\n5\n" \
"Alice Smith" 0

run_test "Поиск по ID (несуществующий)" \
"$DB_FILE $TRACE_FILE" \
"1\n999\n5\n" \
"Студент не найден" 0

run_test "Поиск по фамилии" \
"$DB_FILE $TRACE_FILE" \
"6\nJones\n5\n" \
"Bob Jones" 0

run_test "Сортировка по ID" \
"$DB_FILE $TRACE_FILE" \
"3\n1\n5\n" \
"База отсортирована" 0

# --- Проверка работы с файлом трассировки ---

echo -n "Тест: Проверка записи отличников в trace ... "
rm -f $TRACE_FILE
# Команда 2: Показать отличников в trace, затем 5: Выход
echo -e "2\n5\n" | $EXEC $DB_FILE $TRACE_FILE > /dev/null 2>&1

if grep -q "Smith Alice" "$TRACE_FILE"; then
    echo -e "${GREEN}ПРОЙДЕН${NC}"
else
    echo -e "${RED}ПРОВАЛ${NC}"
    echo "Ожидалось, что 'Smith Alice' будет в файле $TRACE_FILE"
    cat $TRACE_FILE
fi

# Уборка
echo -e "\nУдаление тестовых файлов..."
rm -f $EXEC $DB_FILE $TRACE_FILE