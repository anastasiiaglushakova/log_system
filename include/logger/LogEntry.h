#pragma once  // Защита от повторного включения
              // заголовочного файла

#include <ctime>  // Для типа time_t (время в секундах с эпохи)
#include <string>  // Для использования std::string

// Структура LogEntry представляет собой отдельную запись
// лога
struct LogEntry {
  std::string message;  // Текст лог-сообщения
  time_t timestamp;  // Временная метка (время записи лога)
  std::string level;  // Уровень логирования (например,
                      // "info", "warning", "error")
};
