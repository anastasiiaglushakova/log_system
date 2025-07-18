#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "logger/LogEntry.h"

volatile std::sig_atomic_t stop_flag = 0;

void signal_handler(int) { stop_flag = 1; }

using namespace std;

// Вектор для хранения всех полученных лог-записей
vector<LogEntry> entries;
// Мьютекс для синхронизации доступа к данным
mutex m;

// Общая статистика
int totalMessages = 0;  // Общее количество сообщений
unordered_map<string, int>
  levelCount;  // Количество сообщений по уровням
size_t minLen = SIZE_MAX;  // Минимальная длина сообщения
size_t maxLen = 0;  // Максимальная длина сообщения
size_t totalLen = 0;  // Суммарная длина всех сообщений

bool updated
  = false;  // Флаг, указывающий, что статистика обновлена

// Функция вывода текущей статистики на экран
void printStats() {
  lock_guard<mutex> lock(
    m);  // Блокируем доступ к общим данным

  cout << "\n📊 Statistics:\n";
  cout << "  Total messages: " << totalMessages << "\n";
  cout << "  By level:\n";
  for (auto &[lvl, count] : levelCount) {
    cout << "    " << lvl << ": " << count << "\n";
  }

  // Считаем количество сообщений за последний час
  time_t now = time(nullptr);
  int countLastHour
    = count_if(entries.begin(), entries.end(),
               [now](const LogEntry &e) {
                 return now - e.timestamp <= 3600;
               });
  cout << "  Messages in last hour: " << countLastHour
       << "\n";

  // Выводим статистику по длинам сообщений, если сообщения
  // есть
  if (totalMessages > 0) {
    cout << "  Lengths:\n";
    cout << "    Min: " << minLen << "\n";
    cout << "    Max: " << maxLen << "\n";
    cout << "    Avg: " << totalLen / totalMessages << "\n";
  }

  updated = false;  // Сбрасываем флаг обновления статистики
}

// Функция таймера для периодического вывода статистики
// каждые T секунд
void statsTimer(int T) {
  while (true) {
    this_thread::sleep_for(chrono::seconds(T));
    if (updated)
      printStats();
  }
}

// Функция определения уровня лог-сообщения по его
// содержимому
string determineLevel(const string &line) {
  // Преобразуем строку в верхний регистр для удобного
  // поиска
  string upperLine = line;
  transform(upperLine.begin(), upperLine.end(),
            upperLine.begin(), ::toupper);

  // Проверяем наличие ключевых слов, определяющих уровень
  // логирования
  if (upperLine.find("ERROR") != string::npos
      || upperLine.find("ERR") != string::npos
      || upperLine.find("FATAL") != string::npos)
    return "ERROR";
  else if (upperLine.find("WARNING") != string::npos
           || upperLine.find("WARN") != string::npos
           || upperLine.find("WRN") != string::npos)
    return "WARNING";
  else if (upperLine.find("INFO") != string::npos
           || upperLine.find("INFORMATION") != string::npos)
    return "INFO";
  else if (upperLine.find("DEBUG") != string::npos
           || upperLine.find("DBG") != string::npos
           || upperLine.find("TRACE") != string::npos)
    return "DEBUG";
  else
    return "unknown";  // Если не найден известный уровень
}

// Функция обработки одной строки лога
void processLogLine(const string &line) {
  if (line.empty())
    return;

  time_t now = time(nullptr);
  string level = determineLevel(line);

  // Выводим полученное сообщение с определённым уровнем
  cout << "📝 [" << level << "] " << line << "\n";

  lock_guard<mutex> lock(
    m);  // Блокируем доступ к общим данным

  // Сохраняем запись и обновляем статистику
  entries.push_back({line, now, level});
  levelCount[level]++;

  totalMessages++;
  minLen = min(minLen, line.size());
  maxLen = max(maxLen, line.size());
  totalLen += line.size();

  updated = true;  // Помечаем, что статистика обновлена
}

// Функция для обработки подключенного клиента
void handleClient(int clientSock, int N) {
  cout << "🔌 New client connected (socket: " << clientSock
       << ")\n";

  char buffer[1024];
  string leftover;  // Буфер для хранения неполных данных
                    // между recv

  while (true) {
    memset(buffer, 0, sizeof(buffer));  // Очищаем буфер

    ssize_t bytes
      = recv(clientSock, buffer, sizeof(buffer) - 1, 0);

    cout << "DEBUG: recv returned " << bytes << " bytes\n";

    if (bytes < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Неблокирующий сокет: нет данных сейчас, пробуем
        // снова
        continue;
      }
      cout << "ERROR: recv failed: " << strerror(errno)
           << "\n";
      break;
    }

    if (bytes == 0) {
      cout << "INFO: Client closed connection gracefully\n";
      // Обработка остатков данных, если есть
      if (!leftover.empty()) {
        processLogLine(leftover);
        if (totalMessages % N == 0)
          printStats();
      }
      break;
    }

    // Обеспечиваем корректное завершение строки
    buffer[bytes] = '\0';
    leftover.append(buffer, bytes);

    // Обработка всех полных строк (разделённых '\n')
    size_t pos;
    while ((pos = leftover.find('\n')) != string::npos) {
      string line = leftover.substr(0, pos);
      leftover.erase(0, pos + 1);

      // Убираем возможный символ возврата каретки '\r' для
      // Windows-совместимости
      if (!line.empty() && line.back() == '\r') {
        line.pop_back();
      }

      processLogLine(line);

      // Печатаем статистику каждые N сообщений
      if (totalMessages % N == 0)
        printStats();
    }
  }

  cout << "🔌 Client disconnected (socket: " << clientSock
       << ")\n";
  close(clientSock);
}

// Главная функция программы
int main(int argc, char *argv[]) {
  if (argc < 4) {
    cerr << "Usage: " << argv[0] << " <port> <N> <T>\n";
    cerr << "  port: Port number to listen on\n";
    cerr << "  N: Print stats every N messages\n";
    cerr
      << "  T: Print stats every T seconds (if updated)\n";
    return 1;
  }

  // Считываем параметры из аргументов командной строки
  int port = stoi(argv[1]);
  int N = stoi(argv[2]);
  int T = stoi(argv[3]);

  cout << "Starting log server with parameters:\n";
  cout << "  Port: " << port << "\n";
  cout << "  Stats every " << N << " messages\n";
  cout << "  Auto-stats every " << T << " seconds\n\n";

  // Запускаем поток таймера для периодического вывода
  // статистики
  thread timerThread(statsTimer, T);
  timerThread.detach();

  // Создаём TCP сокет
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket");
    return 1;
  }

  // Позволяем переиспользовать адрес, чтобы избежать ошибки
  // "Address already in use"
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                 sizeof(opt))
      < 0) {
    perror("setsockopt");
    close(server_fd);
    return 1;
  }

  // Настраиваем адрес и порт для привязки
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr
    = INADDR_ANY;  // Привязываемся ко всем интерфейсам
  address.sin_port = htons(port);

  // Привязываем сокет к адресу
  if (bind(server_fd, (struct sockaddr *)&address,
           sizeof(address))
      < 0) {
    perror("bind");
    close(server_fd);
    return 1;
  }

  // Установка обработчиков сигналов для корректного
  // завершения
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  // Начинаем слушать входящие подключения с очередью до 10
  // соединений
  if (listen(server_fd, 10) < 0) {
    perror("listen");
    close(server_fd);
    return 1;
  }

  // Установка таймаута 1 секунда для accept
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO,
                 (const char *)&tv, sizeof(tv))
      < 0) {
    perror("setsockopt SO_RCVTIMEO failed");
    close(server_fd);
    return 1;
  }

  cout << "🟢 Log statistics server listening on port "
       << port << "...\n";

  // Основной цикл принятия входящих соединений
  while (!stop_flag) {
    socklen_t addrlen = sizeof(address);
    int clientSock = accept(
      server_fd, (struct sockaddr *)&address, &addrlen);
    if (clientSock < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Таймаут accept — просто пробуем снова
        continue;
      }
      if (stop_flag)
        break;  // прерываем цикл при сигнале
      perror("accept");
      continue;
    }
    // Создаём новый поток для обработки клиента
    thread clientThread(handleClient, clientSock, N);
    clientThread.detach();
  }

  // После выхода из цикла - закрываем сокет
  close(server_fd);

  return 0;
}
