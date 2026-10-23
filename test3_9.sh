#!/bin/bash

echo "Сборка статистического анализатора..."
gcc 3_9.c -o app_3_9

if [ $? -ne 0 ]; then
    echo "❌ Ошибка компиляции!"
    exit 1
fi

echo "Создаем подопытный текстовый файл..."
cat <<EOF > test_text.txt
hello world, hello C.
programming is fun. world is big.
hello again, world.
EOF

echo "✅ Запуск программы (авто-ввод):"
echo "============================================"

# Запускаем программу. 
# Аргументы: 1) Файл 2) Сепаратор запятая 3) Сепаратор точка
./app_3_9 test_text.txt "," "." <<EOF
1
hello
2
3
3
4
5
6
7
EOF

echo "============================================"
echo "Удаление временных файлов..."
rm app_3_9 test_text.txt tree_backup.txt
echo "Готово!"