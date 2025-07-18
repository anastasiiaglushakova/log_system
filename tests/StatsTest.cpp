#include <gtest/gtest.h>

#include <algorithm>
#include <ctime>
#include <mutex>
#include <string>
#include <vector>

#include "logger/LogEntry.h"

// Класс для сбора статистики по логам: хранит записи,
// считает количество, распределение по уровням,
// минимальную, максимальную и среднюю длину сообщений.
class Stats {
 public:
  // Добавляет новую запись в статистику с блокировкой
  // мьютекса для потокобезопасности
  void addEntry(const LogEntry &entry) {
    std::lock_guard<std::mutex> lock(m_);
    entries_.push_back(entry);
    totalMessages_++;
    levelCount_[entry.level]++;
    minLen_ = std::min(minLen_, entry.message.size());
    maxLen_ = std::max(maxLen_, entry.message.size());
    totalLen_ += entry.message.size();
  }

  // Возвращает общее количество сообщений
  int totalMessages() const { return totalMessages_; }

  // Возвращает количество сообщений заданного уровня, если
  // нет — 0
  int countByLevel(const std::string &level) const {
    auto it = levelCount_.find(level);
    if (it != levelCount_.end())
      return it->second;
    return 0;
  }

  // Возвращает минимальную длину сообщений (0, если
  // сообщений нет)
  size_t minLength() const {
    return totalMessages_ == 0 ? 0 : minLen_;
  }
  // Возвращает максимальную длину сообщений
  size_t maxLength() const { return maxLen_; }
  // Возвращает среднюю длину сообщений (0 при отсутствии
  // сообщений)
  double avgLength() const {
    if (totalMessages_ == 0)
      return 0;
    return static_cast<double>(totalLen_) / totalMessages_;
  }

 private:
  mutable std::mutex
    m_;  // Мьютекс для потокобезопасности доступа к данным
  std::vector<LogEntry>
    entries_;  // Все добавленные записи логов
  int totalMessages_ = 0;  // Общее количество сообщений
  std::unordered_map<std::string, int>
    levelCount_;  // Счётчик по уровням логов
  size_t minLen_ = SIZE_MAX;  // Минимальная длина сообщения
  size_t maxLen_ = 0;  // Максимальная длина сообщения
  size_t totalLen_ = 0;  // Суммарная длина всех сообщений
};

// Проверка базового добавления и подсчёта (твой изначальный
// тест)
TEST(StatsTest, AddAndCount) {
  Stats stats;
  time_t now = time(nullptr);

  // Добавляем несколько записей с разными уровнями и
  // длинами
  stats.addEntry({"Test error message", now, "ERROR"});
  stats.addEntry({"Warning here", now, "WARNING"});
  stats.addEntry({"Info msg", now, "INFO"});
  stats.addEntry({"Another info", now, "INFO"});

  // Проверяем общее число сообщений и распределение по
  // уровням
  EXPECT_EQ(stats.totalMessages(), 4);
  EXPECT_EQ(stats.countByLevel("ERROR"), 1);
  EXPECT_EQ(stats.countByLevel("WARNING"), 1);
  EXPECT_EQ(stats.countByLevel("INFO"), 2);

  // Проверяем минимальную, максимальную и среднюю длины
  // сообщений
  EXPECT_EQ(stats.minLength(),
            std::string("Info msg").size());
  EXPECT_EQ(stats.maxLength(),
            std::string("Test error message").size());
  EXPECT_DOUBLE_EQ(stats.avgLength(),
                   (18 + 12 + 8 + 12) / 4.0);
}

// Проверка поведения при пустом Stats — все значения должны
// быть 0
TEST(StatsTest, EmptyStats) {
  Stats stats;

  EXPECT_EQ(stats.totalMessages(), 0);
  EXPECT_EQ(stats.countByLevel("ERROR"), 0);
  EXPECT_EQ(stats.minLength(), 0);
  EXPECT_EQ(stats.maxLength(), 0);
  EXPECT_DOUBLE_EQ(stats.avgLength(), 0);
}

// Проверка добавления большого количества сообщений одного
// уровня
TEST(StatsTest, MultipleSameLevel) {
  Stats stats;
  time_t now = time(nullptr);

  // Добавляем 10 сообщений уровня INFO с разной длиной
  for (int i = 0; i < 10; i++) {
    stats.addEntry(
      {"Message " + std::to_string(i), now, "INFO"});
  }

  EXPECT_EQ(stats.totalMessages(), 10);
  EXPECT_EQ(stats.countByLevel("INFO"), 10);
  EXPECT_EQ(stats.countByLevel("ERROR"), 0);

  size_t minLen = std::string("Message 0").size();
  size_t maxLen = std::string("Message 9").size();
  EXPECT_EQ(stats.minLength(), minLen);
  EXPECT_EQ(stats.maxLength(), maxLen);

  // Вычисляем ожидаемое среднее значение длины
  double expectedAvg = 0;
  for (int i = 0; i < 10; i++) {
    expectedAvg
      += std::string("Message " + std::to_string(i)).size();
  }
  expectedAvg /= 10.0;

  EXPECT_DOUBLE_EQ(stats.avgLength(), expectedAvg);
}

// Пример простого теста потокобезопасности — несколько
// потоков одновременно добавляют данные
#include <thread>

TEST(StatsTest, ThreadSafety) {
  Stats stats;

  // Лямбда-функция, добавляющая 100 сообщений с разным
  // текстом
  auto worker = [&stats](int id) {
    time_t now = time(nullptr);
    for (int i = 0; i < 100; i++) {
      stats.addEntry({"Thread " + std::to_string(id)
                        + " msg " + std::to_string(i),
                      now, "INFO"});
    }
  };

  // Запускаем два потока-работника
  std::thread t1(worker, 1);
  std::thread t2(worker, 2);
  t1.join();
  t2.join();

  // Проверяем итоговые значения — общее число сообщений и
  // распределение
  EXPECT_EQ(stats.totalMessages(), 200);
  EXPECT_EQ(stats.countByLevel("INFO"), 200);
  EXPECT_GE(stats.minLength(), 0);
  EXPECT_GE(stats.maxLength(), 0);
  EXPECT_GT(stats.avgLength(), 0);
}
