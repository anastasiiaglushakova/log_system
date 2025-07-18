#pragma once  // Защита от повторного включения
              // заголовочного файла

#include <chrono>  // Для работы с временными метками
#include <ctime>  // Для преобразования времени
#include <fstream>  // Для записи в файл
#include <mutex>  // Для синхронизации доступа к лог-файлу
#include <string>  // Для std::string

#include "ILogger.h"   // Интерфейс логгера
#include "LogLevel.h"  // Перечисление уровней логирования

namespace logger {

// Структура, представляющая лог-сообщение с текстом и
// уровнем
struct LogMessage {
  std::string text;  // Текст сообщения
  LogLevel level;  // Уровень логирования
};

// Реализация логгера, записывающего сообщения в файл
class Logger : public ILogger {
 public:
  // Конструктор: принимает имя файла и уровень логирования
  // по умолчанию
  explicit Logger(const std::string &filename,
                  LogLevel level = LogLevel::Info);

  // Деструктор: закрывает файл
  ~Logger();

  // Записывает сообщение в лог, если уровень >= текущего
  void log(const std::string &message,
           LogLevel level) override;

  // Устанавливает текущий уровень логирования
  void setLogLevel(LogLevel level) override;

  // Возвращает текущий уровень логирования
  LogLevel getLogLevel() const override;

 private:
  // Возвращает текущую дату и время в строковом формате
  // "YYYY-MM-DD HH:MM:SS"
  std::string getCurrentTime() const;

  // Преобразует уровень логирования в строку
  static std::string logLevelToString(LogLevel level);

  std::ofstream logFile_;  // Поток для записи в лог-файл
  LogLevel currentLevel_;  // Текущий уровень логирования
  mutable std::mutex
    logMutex_;  // Мьютекс для потокобезопасной записи
};

}  // namespace logger
