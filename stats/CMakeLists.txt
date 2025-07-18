# Создаёт исполняемый файл "log_stats" из исходника main.cpp
add_executable(log_stats main.cpp)

# Устанавливает директорию вывода для исполняемого файла (bin внутри build)
set_target_properties(log_stats PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Добавляет директорию с заголовочными файлами в область видимости только для этого таргета
target_include_directories(log_stats PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
