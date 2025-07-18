#include <iostream>
#include <thread>

#include "logger/LogQueue.h"
#include "logger/Logger.h"
#include "logger/SocketLogger.h"

using namespace logger;

// Функция для преобразования строки в уровень логирования
LogLevel parseLevel(const std::string &s,
                    LogLevel defaultLevel) {
  if (s == "error")
    return LogLevel::Error;
  if (s == "warning")
    return LogLevel::Warning;
  if (s == "info")
    return LogLevel::Info;
  return defaultLevel;  // Возвращаем уровень по умолчанию,
                        // если распознать не удалось
}

// Функция для изменения уровня важности логирования во
// время выполнения
void changeLogLevel(LogLevel &currentLevel,
                    const std::string &newLevelStr) {
  LogLevel newLevel = parseLevel(newLevelStr, currentLevel);
  if (newLevel != currentLevel) {
    currentLevel = newLevel;
    std::cout << "Уровень важности сообщений изменен на: "
              << newLevelStr << std::endl;
  } else {
    std::cout << "Уровень важности остался прежним."
              << std::endl;
  }
}

int main(int argc, char *argv[]) {
  // Проверка аргументов командной строки
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0]
              << " <log_file> [default_level]\n";
    return 1;
  }

  std::string mode
    = argv[1];  // Имя файла или режим "socket"
  LogLevel defaultLevel = LogLevel::Info;
  if (argc >= 3) {
    defaultLevel = parseLevel(argv[2], LogLevel::Info);
  }

  std::unique_ptr<ILogger> loggerPtr;

  if (mode == "socket") {
    // Создаём SocketLogger и подключаемся к серверу
    // логирования
    loggerPtr = std::make_unique<SocketLogger>(
      "127.0.0.1", 5000, defaultLevel);
  } else {
    // Создаём обычный файл-логгер
    loggerPtr
      = std::make_unique<Logger>(mode, defaultLevel);
  }

  LogQueue logQueue;  // Очередь логов между потоками

  // Запускаем рабочий поток, который извлекает сообщения из
  // очереди и логирует
  std::thread worker([&loggerPtr, &logQueue] {
    while (true) {
      auto optMsg
        = logQueue.pop();  // Получаем сообщение из очереди
      if (!optMsg)
        break;  // Если очередь закрыта — завершаем поток
      loggerPtr->log(optMsg->text,
                     optMsg->level);  // Логируем сообщение
    }
  });

  std::cout
    << "Введите сообщения для логирования. Вы можете "
       "указать уровень "
       "(error/warning/info) перед сообщением, разделив их "
       "пробелом. "
       "По умолчанию используется уровень 'info'.\n";
  std::cout << "  change_level <level>  Изменяет уровень "
               "логирования на: info, "
               "warning или error.\n";
  std::cout << "                        Пример: "
               "change_level warning\n";
  std::cout << "  exit                  Завершает работу "
               "приложения.\n";
  std::cout << "                        Можно ввести в "
               "любой момент для "
               "корректного выхода.\n";
  std::cout << "  <уровень> <сообщение> Отправляет "
               "сообщение с указанным "
               "уровнем: error/warning/info.\n";
  std::cout << "                        Уровень должен "
               "быть указан перед "
               "сообщением.\n";
  std::cout << "                        Пример: error "
               "Что-то пошло не так\n";
  std::cout << "                        Логируются только "
               "сообщения с уровнем, "
               "равным или выше текущего.\n";
  std::cout << "  <сообщение>           Сообщение будет "
               "отправлено с текущим "
               "уровнем логирования.\n";

  std::string line;
  while (true) {
    std::cout << "> ";
    if (!std::getline(std::cin, line) || line == "exit") {
      break;  // Выход из программы
    }
    if (line.empty())
      continue;  // Пропускаем пустые строки

    // Обработка команды "change_level <уровень>": изменяет
    // текущий уровень логирования приложения
    size_t pos = line.find(' ');
    LogLevel lvl = defaultLevel;
    std::string msgText = line;

    if (pos != std::string::npos) {
      std::string firstWord = line.substr(0, pos);

      // Проверка на команду изменения уровня
      if (firstWord == "change_level") {
        std::string newLevelStr = line.substr(
          pos + 1);  // Получаем аргумент команды
        changeLogLevel(defaultLevel,
                       newLevelStr);  // Меняем уровень
        loggerPtr->setLogLevel(
          defaultLevel);  // Обновляем логгер
        continue;
      }

      // Если первое слово — допустимый уровень
      // (error/warning/info), использовать его для текущего
      // сообщения
      LogLevel parsed = parseLevel(firstWord, defaultLevel);
      if (parsed != defaultLevel || firstWord == "error"
          || firstWord == "warning"
          || firstWord == "info") {
        lvl = parsed;
        msgText = line.substr(
          pos + 1);  // Остальная часть — это сообщение
      }
    }

    // Отправляем сообщение в очередь логирования
    logQueue.push(LogMessage{msgText, lvl});
  }

  // Завершаем рабочий поток
  logQueue.close();  // Закрываем очередь
  worker.join();  // Ожидаем завершения потока

  std::cout << "Logger stopped.\n";
  return 0;
}
