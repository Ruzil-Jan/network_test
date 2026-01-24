# Network Test
Оригинал: https://github.com/Ruzil-Jan/network_test.git
---

Зависимости:

Для сборки и работы проекта требуются следующие инструменты:

- **CMake** ≥ 3.10 — для конфигурации и генерации сборки;  
- **g++** (GNU C++ Compiler) ≥ 11 — для компиляции кода с поддержкой стандарта **C++11**;  
- **Make** или **Ninja** — система сборки (обычно ставится вместе с CMake);  
- **POSIX-среда** (Linux, Android или Windows с WSL / MinGW) — требуется для работы сокетов (`sys/socket.h`, `arpa/inet.h`, `unistd.h`);  

---

### Получение проекта

Склонировать репозиторий можно с помощью команды:

```bash
cd network_test
mkdir build
cd build
cmake ..
cmake --build .

