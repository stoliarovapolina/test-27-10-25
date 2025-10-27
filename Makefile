# Makefile для игры Марио

CXX = g++
CXXFLAGS = -std=c++11 -Wall
TARGET = mario
SOURCE = mario.cpp

# Основная цель - компиляция игры
all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

# Очистка скомпилированных файлов
clean:
	rm -f $(TARGET) $(TARGET).exe

# Запуск игры
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
