#pragma once  // Защита от повторного включения
              // заголовочного файла

#include <condition_variable>  // Для синхронизации потоков
#include <mutex>  // Для блокировки доступа к очереди
#include <optional>  // Для безопасного возвращения пустого значения
#include <queue>  // Для хранения лог-сообщений

#include "Logger.h"  // Для определения LogMessage

namespace logger {

// Класс потокобезопасной очереди сообщений для передачи
// логов между потоками. Поддерживает push(), pop() с
// блокировкой, и закрытие очереди через close().
class LogQueue {
 public:
  // Добавляет сообщение в очередь и уведомляет ожидающий
  // поток
  void push(const LogMessage &msg) {
    std::lock_guard<std::mutex> lock(m_);
    q_.push(msg);
    cv_.notify_one();  // Пробуждает один поток, ожидающий
                       // сообщение
  }

  // Извлекает сообщение из очереди (блокирует, если очередь
  // пуста) Возвращает std::nullopt, если очередь закрыта и
  // пуста
  std::optional<LogMessage> pop() {
    std::unique_lock<std::mutex> lock(m_);
    // Ожидаем, пока в очереди не появится сообщение или
    // очередь не будет закрыта
    cv_.wait(lock,
             [this] { return !q_.empty() || closed_; });

    if (q_.empty() && closed_)
      return std::nullopt;  // Если очередь пуста и закрыта
                            // — завершение
    auto msg = q_.front();  // Получаем первое сообщение
    q_.pop();  // Удаляем его из очереди
    return msg;
  }

  // Закрывает очередь — все ожидающие потоки будут
  // разбужены
  void close() {
    std::lock_guard<std::mutex> lock(m_);
    closed_ = true;
    cv_.notify_all();  // Пробуждает все потоки, ожидающие в
                       // pop()
  }

 private:
  std::queue<LogMessage> q_;  // Очередь лог-сообщений
  std::mutex m_;  // Мьютекс для синхронизации доступа
  std::condition_variable
    cv_;  // Условная переменная для ожидания
  bool closed_ = false;  // Флаг закрытия очереди
};

}  // namespace logger
