# Подключаем библиотеку GoogleTest (требуется для сборки тестов)
find_package(GTest REQUIRED)

# Добавляем директории с заголовочными файлами GoogleTest
include_directories(${GTEST_INCLUDE_DIRS})

# Создаём исполняемый файл tests_runner из указанных исходников
add_executable(tests_runner
    main.cpp
    LoggerTest.cpp
    SocketLoggerTest.cpp
    LogQueueTest.cpp
    StatsTest.cpp
)

# Добавляем директорию с заголовочными файлами проекта для tests_runner
target_include_directories(tests_runner PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Линкуем тестовый исполняемый файл с библиотекой logger, GoogleTest и pthread (для потоков)
target_link_libraries(tests_runner PRIVATE logger GTest::GTest GTest::Main pthread)

# Устанавливаем директорию вывода исполняемого файла tests_runner
set_target_properties(tests_runner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Создаём директорию для логов тестов, если её ещё нет
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_logs)

# Определяем макрос LOG_DIR с путём к директории логов для использования в тестах
target_compile_definitions(tests_runner PRIVATE LOG_DIR="${CMAKE_BINARY_DIR}/test_logs")
