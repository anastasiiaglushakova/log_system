#include <gtest/gtest.h>

#include <fstream>
#include <string>

#include "logger/Logger.h"

using namespace logger;

// Тест для проверки установки и получения уровня
// логирования
TEST(LoggerTest, SetGetLevel) {
  // Формируем путь к файлу лога с использованием макроса
  // LOG_DIR
  std::string filepath
    = std::string(LOG_DIR) + "/test_setget.log";

  // Создаём логгер с уровнем Warning
  Logger logger(filepath, LogLevel::Warning);
  // Проверяем, что уровень установлен правильно
  EXPECT_EQ(logger.getLogLevel(), LogLevel::Warning);

  // Меняем уровень на Error
  logger.setLogLevel(LogLevel::Error);
  // Проверяем, что уровень обновился корректно
  EXPECT_EQ(logger.getLogLevel(), LogLevel::Error);
}

// Тест для проверки записи сообщений с разными уровнями
// логирования
TEST(LoggerTest, LoggingLevels) {
  // Формируем имя файла для логирования
  std::string filename
    = std::string(LOG_DIR) + "/test_levels.log";

  // Создаём логгер с уровнем Warning
  Logger logger(filename, LogLevel::Warning);

  // Логируем сообщение уровня Info (не должно попасть в
  // файл)
  logger.log("Info message", LogLevel::Info);
  // Логируем сообщение уровня Warning (должно попасть в
  // файл)
  logger.log("Warning message", LogLevel::Warning);
  // Логируем сообщение уровня Error (должно попасть в файл)
  logger.log("Error message", LogLevel::Error);

  // Открываем файл для чтения содержимого
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open());

  // Считываем всё содержимое файла в строку
  std::string content(
    (std::istreambuf_iterator<char>(file)),
    std::istreambuf_iterator<char>());
  file.close();

  // Проверяем, что в содержимом файла есть сообщения
  // уровней Warning и Error
  EXPECT_NE(content.find("Warning message"),
            std::string::npos);
  EXPECT_NE(content.find("Error message"),
            std::string::npos);
  // Проверяем, что сообщение уровня Info отсутствует в
  // файле
  EXPECT_EQ(content.find("Info message"),
            std::string::npos);
}
