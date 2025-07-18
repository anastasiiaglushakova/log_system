#pragma once  // Гарантирует, что заголовочный файл будет
              // включён только один раз при компиляции

#include <string>  // Для использования std::string

#include "LogLevel.h"  // Определение перечисления LogLevel

namespace logger {

// Интерфейс логгера, определяющий базовый контракт для всех
// типов логгеров
class ILogger {
 public:
  virtual ~ILogger()
    = default;  // Виртуальный деструктор для корректного
                // удаления через указатель на базовый класс

  // Чисто виртуальная функция логирования сообщения с
  // указанным уровнем
  virtual void log(const std::string &message,
                   LogLevel level)
    = 0;

  // Устанавливает текущий уровень логирования
  virtual void setLogLevel(LogLevel level) = 0;

  // Возвращает текущий установленный уровень логирования
  virtual LogLevel getLogLevel() const = 0;
};

}  // namespace logger
