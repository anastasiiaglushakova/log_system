cmake_minimum_required(VERSION 3.14)  # Минимально необходимая версия CMake для сборки проекта
project(LoggerSystem LANGUAGES CXX)   # Объявляем проект LoggerSystem, использующий язык C++

set(CMAKE_CXX_STANDARD 17)            # Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Требуем строгое соответствие стандарту C++17
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Включаем компиляцию с позиционно-независимым кодом (PIC), полезно для создания shared library

option(BUILD_SHARED_LIBS "Build shared libraries" ON)  # Опция для сборки библиотеки как shared (по умолчанию включена)

# Устанавливаем универсальные флаги компилятора
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic -O2 \
    -fstack-protector-strong -Wshadow -Wconversion -Wfloat-equal")

# Добавляем подпроекты (поддиректории) с исходниками и приложениями
add_subdirectory(src)    # Исходники библиотеки логирования
add_subdirectory(app)    # Приложение логгера или клиент
add_subdirectory(stats)  # Приложение статистики по логам
add_subdirectory(tests)  # Тесты проекта
