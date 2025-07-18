#include <gtest/gtest.h>

#include "logger/SocketLogger.h"

using namespace logger;

// Тестируем установку и получение уровня логирования
TEST(SocketLoggerTest, SetGetLevel) {
  // Создаем SocketLogger с уровнем Info
  SocketLogger slogger("127.0.0.1", 5000, LogLevel::Info);
  // Проверяем, что уровень логирования корректно
  // возвращается
  EXPECT_EQ(slogger.getLogLevel(), LogLevel::Info);

  // Меняем уровень логирования на Warning
  slogger.setLogLevel(LogLevel::Warning);
  // Проверяем, что уровень обновился
  EXPECT_EQ(slogger.getLogLevel(), LogLevel::Warning);
}

// Тест подключения - здесь сложно тестировать без реального
// сервера, но можно проверить, что сокет создаётся (если
// доступно). В реальном проекте можно замокать или
// запустить тестовый сервер.
TEST(SocketLoggerTest, Connection) {
  // Создаем SocketLogger, пытаемся подключиться к серверу
  // по localhost:5000
  SocketLogger slogger("127.0.0.1", 5000, LogLevel::Info);
  // Вызываем метод log, чтобы проверить, что логгер не
  // ломается
  slogger.log("Test message", LogLevel::Info);
  // Подтверждаем, что тест прошел без ошибок
  SUCCEED();
}
