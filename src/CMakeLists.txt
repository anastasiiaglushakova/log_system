# Создаёт библиотеку "logger" из исходных файлов Logger.cpp и SocketLogger.cpp
add_library(logger
    Logger.cpp
    SocketLogger.cpp
)

# Добавляет директорию с заголовочными файлами в область видимости библиотеки
# PUBLIC означает, что эта директория будет видна и для всех, кто линкуется с этой библиотекой
target_include_directories(logger PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
