#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "logger/LogQueue.h"
#include "logger/Logger.h"

using namespace logger;

// Тест однопоточного добавления и извлечения сообщения из
// очереди
TEST(LogQueueTest, PushPopSingleThread) {
  LogQueue q;
  LogMessage msg{"Hello", LogLevel::Info};

  // Добавляем сообщение в очередь
  q.push(msg);

  // Извлекаем сообщение из очереди
  auto popped = q.pop();

  // Проверяем, что сообщение успешно извлечено и совпадает
  // с добавленным
  ASSERT_TRUE(popped.has_value());
  EXPECT_EQ(popped->text, "Hello");
  EXPECT_EQ(popped->level, LogLevel::Info);

  // Закрываем очередь
  q.close();

  // После закрытия очередь должна вернуть пустое значение
  // при попытке извлечь сообщение
  auto closedPop = q.pop();
  EXPECT_FALSE(closedPop.has_value());
}

// Тест многопоточной работы с очередью: один производитель
// и один потребитель
TEST(LogQueueTest, PushPopMultiThread) {
  LogQueue q;

  // Поток производителя, который добавляет 10 сообщений и
  // закрывает очередь
  std::thread producer([&q]() {
    for (int i = 0; i < 10; ++i) {
      q.push(LogMessage{"msg" + std::to_string(i),
                        LogLevel::Info});
    }
    q.close();
  });

  std::vector<LogMessage> results;

  // Поток потребителя, который извлекает сообщения из
  // очереди до её закрытия
  std::thread consumer([&q, &results]() {
    while (true) {
      auto msg = q.pop();
      if (!msg)
        break;  // очередь закрыта и пуста
      results.push_back(*msg);
    }
  });

  // Ожидаем завершения работы потоков
  producer.join();
  consumer.join();

  // Проверяем, что все 10 сообщений были получены
  EXPECT_EQ(results.size(), 10);

  // Проверяем корректность каждого полученного сообщения
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(results[i].text, "msg" + std::to_string(i));
    EXPECT_EQ(results[i].level, LogLevel::Info);
  }
}
