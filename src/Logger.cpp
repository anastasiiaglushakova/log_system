#include "logger/Logger.h"

namespace logger {

// Конструктор: открывает файл лога в режиме добавления
// (append) И устанавливает уровень логирования по умолчанию
Logger::Logger(const std::string &filename, LogLevel level)
    : currentLevel_(level) {
  logFile_.open(filename, std::ios::app);
  if (!logFile_.is_open()) {
    // Если не удалось открыть файл — выводим ошибку в
    // stderr, но не бросаем исключение
    fprintf(stderr, "Failed to open log file: %s\n",
            filename.c_str());
  }
}

// Деструктор: закрывает файл лога, если он открыт
Logger::~Logger() {
  if (logFile_.is_open()) {
    logFile_.close();
  }
}

// Метод записи сообщения в лог
void Logger::log(const std::string &message,
                 LogLevel level) {
  // Если уровень сообщения выше (меньше важен), чем текущий
  // уровень — игнорируем сообщение
  if (level > currentLevel_) {
    return;
  }

  // Блокируем мьютекс для потокобезопасного доступа к файлу
  std::lock_guard<std::mutex> lock(logMutex_);

  if (logFile_.is_open()) {
    // Записываем строку лога в формате: [время] [уровень]
    // сообщение
    logFile_ << "[" << getCurrentTime() << "] "
             << "[" << Logger::logLevelToString(level)
             << "] " << message << std::endl;
  }
}

// Установка текущего уровня логирования с защитой мьютексом
void Logger::setLogLevel(LogLevel level) {
  std::lock_guard<std::mutex> lock(logMutex_);
  currentLevel_ = level;
}

// Получение текущего уровня логирования с защитой мьютексом
LogLevel Logger::getLogLevel() const {
  std::lock_guard<std::mutex> lock(logMutex_);
  return currentLevel_;
}

// Вспомогательный метод для получения текущего времени в
// строковом формате
std::string Logger::getCurrentTime() const {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  char buf[64];
  // Форматируем время в строку вида "YYYY-MM-DD HH:MM:SS"
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S",
                std::localtime(&t));
  return buf;
}

// Преобразование значения LogLevel в строку для вывода в
// лог
std::string Logger::logLevelToString(LogLevel level) {
  switch (level) {
    case LogLevel::Error:
      return "ERROR";
    case LogLevel::Warning:
      return "WARNING";
    case LogLevel::Info:
      return "INFO";
    default:
      return "UNKNOWN";
  }
}

}  // namespace logger
