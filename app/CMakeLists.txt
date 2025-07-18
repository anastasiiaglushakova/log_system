# Создаёт исполняемый файл "app" из исходника main.cpp
add_executable(app main.cpp)

# Подключает библиотеку logger к исполняемому файлу "app"
# Используется приватное связывание (только для этого таргета)
target_link_libraries(app PRIVATE logger)

# Добавляет директорию с заголовками в область видимости таргета "app"
# ${PROJECT_SOURCE_DIR}/include — путь к общей папке с .h-файлами
target_include_directories(app PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Устанавливает выходную директорию для исполняемого файла "app"
# Файл будет размещён в подкаталоге bin внутри директории сборки
set_target_properties(app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)