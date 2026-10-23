#!/bin/bash

# Название исходника и исполняемого файла
SOURCE="3_1.c"
EXEC="./3_1.out"

# Цвета для вывода
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# Компиляция программы со стандартом C99[cite: 10]
echo "Компиляция $SOURCE..."
gcc -std=c99 -Wall -Wextra -Werror $SOURCE -o $EXEC

if [ $? -ne 0 ]; then
    echo -e "${RED}Ошибка компиляции!${NC}"
    exit 1
fi

echo -e "${GREEN}Компиляция успешно завершена.${NC}\n"

# Функция для тестирования
# $1 - Аргумент 1 (число)
# $2 - Аргумент 2 (степень r)
# $3 - Ожидаемый вывод
# $4 - Ожидаемый код возврата
run_test() {
    local num=$1
    local r=$2
    local expected_out=$3
    local expected_ret=$4

    echo -n "Тест: convert($num, r=$r) ... "
    
    # Запуск программы с перенаправлением ошибок в стандартный вывод
    actual_out=$($EXEC $num $r 2>&1)
    actual_ret=$?

    if [ "$actual_ret" -eq "$expected_ret" ] && [[ "$actual_out" == *"$expected_out"* ]]; then
        echo -e "${GREEN}ПРОЙДЕН${NC} (Результат: $actual_out)"
    else
        echo -e "${RED}ПРОВАЛ${NC}"
        echo "   Ожидалось: вывод содержит '$expected_out', статус $expected_ret"
        echo "   Получено : вывод '$actual_out', статус $actual_ret"
    fi
}

# --- Позитивные тесты ---
# 10 в двоичную (r=1) -> 1010
run_test "10" "1" "1010" 0

# 10 в четверичную (r=2) -> 22
run_test "10" "2" "22" 0

# 255 в шестнадцатеричную (r=4) -> FF
run_test "255" "4" "FF" 0

# 31 в 32-ричную (r=5) -> V
run_test "31" "5" "V" 0

# 32 в 32-ричную (r=5) -> 10
run_test "32" "5" "10" 0

# Отрицательное число: -10 в двоичную (r=1) -> -1010
run_test "-10" "1" "-1010" 0

# Ноль: 0 в 8-ричную (r=3) -> 0
run_test "0" "3" "0" 0

# --- Негативные тесты ---
# Ошибка значения r (r > 5)[cite: 11] (статус 1 - ERR_INVALID_R)
run_test "10" "6" "Ошибка: значение r" 1

# Ошибка значения r (r < 1)[cite: 11] (статус 1 - ERR_INVALID_R)
run_test "10" "0" "Ошибка: значение r" 1

# Некорректные параметры (статус 3 - ERR_INVALID_ARGS)[cite: 10]
run_test "abc" "2" "Ошибка: неверный формат" 3

# Недостаточно аргументов (статус 3 - ERR_INVALID_ARGS)[cite: 10]
echo -n "Тест: запуск без аргументов ... "
actual_out=$($EXEC 2>&1)
actual_ret=$?
if [ "$actual_ret" -eq 3 ]; then
    echo -e "${GREEN}ПРОЙДЕН${NC}"
else
    echo -e "${RED}ПРОВАЛ${NC}"
fi

echo -e "\nУдаление скомпилированного файла..."
rm $EXEC