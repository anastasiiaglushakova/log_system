#pragma once  // Защита от повторного включения
              // заголовочного файла

#include <netinet/in.h>  // Для работы с сетевыми структурами и протоколами (sockaddr_in и др.)
#include <unistd.h>  // Для системных вызовов POSIX (close и т.п.)

#include <mutex>  // Для мьютекса — защиты от одновременного доступа из нескольких потоков
#include <string>  // Для std::string

#include "ILogger.h"  // Интерфейс ILogger для реализации методов логгера

namespace logger {

// Класс логгера, отправляющего сообщения через TCP-сокет на
// указанный хост и порт
class SocketLogger : public ILogger {
 public:
  // Конструктор: устанавливает соединение с хостом и
  // портом, задаёт уровень логирования по умолчанию
  SocketLogger(const std::string &host, int port,
               LogLevel defaultLevel);

  // Деструктор: закрывает сокет
  ~SocketLogger();

  // Отправляет лог-сообщение по TCP в формате "[время]
  // [уровень] сообщение" Сообщение отправляется только при
  // валидном соединении и если уровень >= установленного
  void log(const std::string &message,
           LogLevel level) override;

  // Устанавливает текущий уровень логирования
  void setLogLevel(LogLevel level) override;

  // Возвращает текущий уровень логирования
  LogLevel getLogLevel() const override;

 private:
  int sock_;  // Дескриптор TCP-сокета
  LogLevel logLevel_;  // Текущий уровень логирования
  mutable std::mutex
    mutex_;  // Мьютекс для потокобезопасности доступа к
             // сокету и уровню
};

}  // namespace logger
