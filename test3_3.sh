#!/bin/bash

gcc 3_3.c -o app_3_3

if [ $? -ne 0 ]; then
    echo "❌ Ошибка компиляции!"
    exit 1
fi

echo "✅ Успешно собрано. Запускаем сжатие:"
echo "============================================"
echo "beep boop beer bear boar" > test_text.txt

# Правильный запуск с префиксом ./
./app_3_3 test_text.txt

echo "============================================"
echo ">>> Тест пустого файла:"
touch empty.txt

# Правильный запуск с префиксом ./
./app_3_3 empty.txt

# Убираем мусор
rm app_3_3 test_text.txt empty.txt