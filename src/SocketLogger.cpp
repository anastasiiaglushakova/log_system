#include "logger/SocketLogger.h"

#include <arpa/inet.h>  // Для функций работы с IP (inet_pton)
#include <unistd.h>  // Для системных вызовов close, shutdown

#include <chrono>  // Для работы со временем
#include <cstring>  // Для memset и др.
#include <ctime>  // Для преобразования времени
#include <iomanip>   // Для std::put_time
#include <iostream>  // Для perror
#include <sstream>   // Для std::ostringstream

namespace logger {

// Конструктор: создаёт TCP-сокет и подключается к
// указанному хосту и порту
SocketLogger::SocketLogger(const std::string &host,
                           int port, LogLevel defaultLevel)
    : logLevel_(defaultLevel) {
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ < 0) {
    perror("socket");  // Вывод ошибки при создании сокета
    return;
  }

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;  // Используем IPv4
  serverAddr.sin_port = htons(
    static_cast<uint16_t>(port));  // Устанавливаем порт (в сетевом порядке байт)
  inet_pton(
    AF_INET, host.c_str(),
    &serverAddr
       .sin_addr);  // Преобразуем IP-адрес из строки

  // Пытаемся подключиться к серверу
  if (connect(sock_, (sockaddr *)&serverAddr,
              sizeof(serverAddr))
      < 0) {
    perror("connect");  // Ошибка подключения
    close(sock_);  // Закрываем сокет при ошибке
    sock_ = -1;  // Помечаем сокет как невалидный
  }
}

// Деструктор: закрывает сокет, если он был открыт
SocketLogger::~SocketLogger() {
  if (sock_ >= 0) {
    close(sock_);
  }
}

// Метод для отправки лог-сообщения по сокету
void SocketLogger::log(const std::string &message,
                       LogLevel level) {
  std::lock_guard<std::mutex> lock(
    mutex_);  // Потокобезопасность
  if (level > logLevel_ || sock_ < 0)
    return;  // Игнорируем, если уровень ниже или сокет не
             // валиден

  std::ostringstream oss;

  // Получаем текущее время для таймстампа
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);

  // Формируем строку времени в формате "YYYY-MM-DD
  // HH:MM:SS"
  oss << "["
      << std::put_time(std::localtime(&now_c), "%F %T")
      << "] ";

  // Добавляем строковое представление уровня логирования
  switch (level) {
    case LogLevel::Error:
      oss << "[ERROR] ";
      break;
    case LogLevel::Warning:
      oss << "[WARNING] ";
      break;
    case LogLevel::Info:
      oss << "[INFO] ";
      break;
  }

  oss << message
      << "\n";  // Добавляем текст сообщения и символ новой
                // строки для разделения сообщений

  std::string out = oss.str();
  size_t totalSent = 0;
  size_t toSend = out.size();

  // Цикл, отправляющий все данные по сокету (send может
  // отправить не все сразу)
  while (totalSent < toSend) {
    ssize_t sent = send(sock_, out.data() + totalSent,
                        toSend - totalSent, 0);
    if (sent == -1) {
      perror("send");  // Вывод ошибки при отправке
      break;
    }
    totalSent += sent;
  }
}

// Установка текущего уровня логирования с защитой мьютексом
void SocketLogger::setLogLevel(LogLevel level) {
  std::lock_guard<std::mutex> lock(mutex_);
  logLevel_ = level;
}

// Получение текущего уровня логирования с защитой мьютексом
LogLevel SocketLogger::getLogLevel() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return logLevel_;
}

}  // namespace logger
