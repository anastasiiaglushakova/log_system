.PHONY: all build run_tests run_app run_stats run_app_stats clean help

# Цель по умолчанию
all: build

# Определение директории сборки
BUILD_DIR := $(if $(findstring ON,$(STATIC)),build_static,build)

# Переменные по умолчанию
LOG_FILE := ./$(BUILD_DIR)/logs.txt
LOG_LEVEL := info
PORT ?= 5000
N ?= 3
T ?= 10

# Сборка с опцией STATIC=ON или STATIC=OFF (по умолчанию shared)
build:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DBUILD_SHARED_LIBS=$(if $(findstring ON,$(STATIC)),OFF,ON) ..
	cd $(BUILD_DIR) && cmake --build . --target app tests_runner log_stats

# Запуск тестов
run_tests: build
	mkdir -p $(BUILD_DIR)/test_logs
	./$(BUILD_DIR)/tests_runner

# Запуск приложения (файл логирования)
run_app: build
	./$(BUILD_DIR)/bin/app $(LOG_FILE) $(LOG_LEVEL)

# Запуск приложения с SocketLogger
run_app_stats: build
	./$(BUILD_DIR)/bin/app socket $(LOG_LEVEL)

# Универсальный запуск сервера статистики
run_stats: build
	@if [ -x $(BUILD_DIR)/bin/log_stats ]; then \
		echo "🔧 Запуск log_stats из $(BUILD_DIR)..."; \
		./$(BUILD_DIR)/bin/log_stats $(PORT) $(N) $(T); \
	else \
		echo "❌ log_stats не найден. Выполните 'make build'."; \
	fi

# Очистка всех build-директорий
clean:
	rm -rf build build_static

help:
	@echo "По умолчанию сборка динамическая (shared)."
	@echo ""
	@echo "Доступные цели:"
	@echo "  all                   Цель по умолчанию (эквивалент make build)."
	@echo "  build                 Сборка проекта."
	@echo "  run_tests             Запуск тестов."
	@echo "  run_app               Запуск приложения с логированием в файл."
	@echo "  run_app_stats         Запуск приложения с SocketLogger, отправляет логи на сервер."
	@echo "  run_stats             Запуск сервера статистики."
	@echo "                        Запустите в отдельном терминале."
	@echo "                        Параметры по умолчанию: PORT=5000 N=3 T=10"
	@echo "                        Для указания параметров используйте:"
	@echo "                          make run_stats PORT=6000 N=5 T=20"
	@echo ""
	@echo "Использование статической сборки:"
	@echo "  Для статической сборки используйте STATIC=ON с любой целью:"
	@echo "    make build STATIC=ON"
	@echo "    make run_tests STATIC=ON"
	@echo "    make run_app STATIC=ON"
	@echo "    make run_app_stats STATIC=ON"
	@echo "    make run_stats STATIC=ON"
	@echo ""
	@echo "  clean                 Удаление всех директорий сборки (build и build_static)."
	@echo "  help                  Вывод этого справочного сообщения."
	@echo ""
	@echo "Доступные команды во время работы приложения:"
	@echo "  change_level <level>  Изменяет уровень логирования на: info, warning или error."
	@echo "  exit                  Завершает работу приложения."
	@echo "  <уровень> <сообщение> Отправляет сообщение с указанным уровнем: error/warning/info."
	@echo "                        Логируются только сообщения с уровнем, равным или выше текущего уровня логгера."
	@echo "                        Уровни по убыванию важности: error > warning > info."
	@echo "  <сообщение>           Сообщение будет отправлено с текущим уровнем логирования."
	@echo ""
	@echo "Использование переменных логирования:"
	@echo "  LOG_FILE              Укажите файл для записи логов, например:"
	@echo "                        make run_app LOG_FILE=./logs.txt"
	@echo "  LOG_LEVEL             Укажите уровень логирования, например:"
	@echo "                        make run_app LOG_LEVEL=warning"
	@echo "                        По умолчанию используется уровень 'info'."
	@echo ""
	@echo "Пример использования:"
	@echo "  make run_app STATIC=ON LOG_FILE=./my_logs.txt LOG_LEVEL=warning"
	@echo "Порядок указания переменных не имеет значения."

