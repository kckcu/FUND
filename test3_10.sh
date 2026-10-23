#!/bin/bash

echo "Сборка построителя деревьев..."
gcc 3_10.c -o app_3_10

if [ $? -ne 0 ]; then
    echo "❌ Ошибка компиляции! Проверь код."
    exit 1
fi

echo "Создание входного файла с выражениями..."
cat <<EOF > input.txt
A (B (E (G, T, R (W, Z)), F (L, M)), C)
MainFolder (Docs (Taxes, Bills), Photos (2025, 2026), Games)
EOF

echo "✅ Запуск программы:"
echo "============================================"

# Запуск с аргументами: входной файл и выходной файл
./app_3_10 input.txt output.txt

echo "============================================"
echo ">>> Результат (содержимое output.txt):"
cat output.txt

echo "============================================"
echo "Удаление временных файлов..."
rm app_3_10 input.txt output.txt
echo "Готово!"