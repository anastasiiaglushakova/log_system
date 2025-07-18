#include <gtest/gtest.h>

// Главная функция для запуска всех тестов Google Test
int main(int argc, char **argv) {
  // Инициализация фреймворка Google Test с параметрами
  // командной строки
  ::testing::InitGoogleTest(&argc, argv);

  // Запуск всех зарегистрированных тестов и возврат
  // результата
  return RUN_ALL_TESTS();
}
