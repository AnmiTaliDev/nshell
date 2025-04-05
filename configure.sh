#!/bin/bash
# NaGNU/nshell - Configuration script
# Developers: ruzen42, mkfs, AnmiTaliDev

# Получаем текущую дату и время в UTC
BUILD_DATE=$(date -u +"%Y-%m-%d %H:%M:%S")

# Конфигурационные переменные по умолчанию
PREFIX="/usr/local"
DEBUG=0
READLINE=1
COLOR=1

# Функция для вывода помощи
show_help() {
    cat << EOF
Скрипт настройки для NShell

Использование: ./configure.sh [опции]

Опции:
  --prefix=DIR           Установить префикс сборки (по умолчанию: /usr/local)
  --enable-debug        Включить отладочную информацию
  --disable-readline    Отключить поддержку readline
  --disable-color       Отключить поддержку цветного вывода
  --help               Показать эту справку и выйти

EOF
}

# Обработка аргументов командной строки
for arg in "$@"; do
    case $arg in
        --prefix=*)
            PREFIX="${arg#*=}"
            ;;
        --enable-debug)
            DEBUG=1
            ;;
        --disable-readline)
            READLINE=0
            ;;
        --disable-color)
            COLOR=0
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo "Неизвестная опция: $arg"
            echo "Используйте --help для получения справки"
            exit 1
            ;;
    esac
done

# Проверка зависимостей
echo "Проверка зависимостей..."

# Проверка компилятора
if ! command -v gcc >/dev/null 2>&1; then
    echo "Ошибка: GCC не найден"
    exit 1
fi

# Проверка readline если включен
if [ $READLINE -eq 1 ]; then
    if ! pkg-config --exists readline; then
        echo "Ошибка: библиотека readline не найдена"
        echo "Установите libreadline-dev или используйте --disable-readline"
        exit 1
    fi
fi

# Создание конфигурационного файла
cat > config.h << EOF
/* Автоматически сгенерировано configure.sh */
#ifndef CONFIG_H
#define CONFIG_H

#define BUILD_DATE "$BUILD_DATE"
#define PREFIX "$PREFIX"

$([ $DEBUG -eq 1 ] && echo "#define DEBUG")
$([ $READLINE -eq 1 ] && echo "#define HAVE_READLINE")
$([ $COLOR -eq 1 ] && echo "#define ENABLE_COLOR")

#endif /* CONFIG_H */
EOF

# Генерация Makefile из шаблона
sed -e "s|@PREFIX@|$PREFIX|g" \
    -e "s|@DEBUG@|$DEBUG|g" \
    -e "s|@READLINE@|$READLINE|g" \
    -e "s|@COLOR@|$COLOR|g" \
    Makefile.in > Makefile

echo "Конфигурация завершена:"
echo "  Префикс установки: $PREFIX"
echo "  Отладка: $([ $DEBUG -eq 1 ] && echo 'включена' || echo 'выключена')"
echo "  Readline: $([ $READLINE -eq 1 ] && echo 'включен' || echo 'выключен')"
echo "  Цветной вывод: $([ $COLOR -eq 1 ] && echo 'включен' || echo 'выключен')"
echo "  Дата сборки: $BUILD_DATE"
echo
echo "Теперь выполните 'make' для сборки проекта"

chmod +x configure.sh