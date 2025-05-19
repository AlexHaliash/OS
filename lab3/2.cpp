#include <iostream>
#include <windows.h>
#include <vector>
#include <stdexcept>

class MarkerManager {
private:
    std::vector<int> arr;             
    std::vector<HANDLE> stopEvents;    
    std::vector<HANDLE> continueEvents; 
    std::vector<HANDLE> threads;      
    std::vector<bool> isWorking;      
    CRITICAL_SECTION cs;            
    int numThreads;                     

    // Сброс событий в несигнальное состояние
    void resetEvents(std::vector<HANDLE>& events) {
        for (auto& event : events) {
            ResetEvent(event);
        }
    }

    // Вывод текущего состояния массива
    void showArray() const {
        for (size_t i = 0; i < arr.size(); ++i) {
            std::cout << i + 1 << ": " << arr[i] << "\n";
        }
    }

    // Статическая функция-обертка для потока
    static DWORD WINAPI marker(LPVOID lpParam) {
        auto* manager = reinterpret_cast<MarkerManager*>(lpParam);
        return manager->markerThread();
    }

    // Основная функция потока-маркера
    DWORD markerThread() {
        const int threadId = GetCurrentThreadId();
        const int markerNumber = threadId % numThreads; 
        int markedElements = 0;
        srand(threadId);  // Инициализация ГСЧ с уникальным seed для каждого потока

        std::vector<bool> visitedArray(arr.size(), false);  // Отслеживание помеченных элементов

        while (true) {
            markedElements = 0;
            while (true) {
                int randomNum = rand() % arr.size();  // Выбор случайного элемента

                EnterCriticalSection(&cs);  // Вход в критическую секцию
                if (arr[randomNum] != 0) {
                    // Если элемент уже помечен - остановка потока
                    std::cout << "Thread " << markerNumber + 1 << " stopped. "
                        << "Marked elements: " << markedElements << ". "
                        << "Blocked at element: " << randomNum + 1 << "\n";
                    SetEvent(stopEvents[markerNumber]);  // Сигнализация о остановке
                    LeaveCriticalSection(&cs);
                    break;
                }

                if (arr[randomNum] == 0) {
                    Sleep(5);  // Имитация работы
                    arr[randomNum] = markerNumber + 1;  // Помечаем элемент
                    markedElements++;
                    visitedArray[randomNum] = true;  // Запоминаем помеченный элемент
                    Sleep(5);
                }
                LeaveCriticalSection(&cs);  // Выход из критической секции
            }

            // Ожидание команды продолжить
            WaitForSingleObject(continueEvents[markerNumber], INFINITE);
            ResetEvent(continueEvents[markerNumber]);

            if (!isWorking[markerNumber]) {
                // Если поток должен завершиться
                EnterCriticalSection(&cs);
                std::cout << "Thread " << markerNumber + 1 << " terminating\n";
                // Обнуляем все помеченные этим потоком элементы
                for (size_t i = 0; i < arr.size(); ++i) {
                    if (visitedArray[i]) {
                        arr[i] = 0;
                    }
                }
                SetEvent(stopEvents[markerNumber]);  // Сигнализация о завершении
                LeaveCriticalSection(&cs);
                return 0;
            }
        }
    }

public:
    // Конструктор
    MarkerManager(int arraySize, int threadCount)
        : arr(arraySize, 0), numThreads(threadCount) {
        
        // Проверка входных параметров
        if (arraySize <= 0 || threadCount <= 0) {
            throw std::invalid_argument("Array size and thread count must be positive");
        }

        InitializeCriticalSection(&cs);  // Инициализация критической секции

        // Создание событий для каждого потока
        stopEvents.resize(threadCount);
        continueEvents.resize(threadCount);
        isWorking.resize(threadCount, true);
        threads.resize(threadCount);

        for (int i = 0; i < threadCount; ++i) {
            stopEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
            continueEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (!stopEvents[i] || !continueEvents[i]) {
                throw std::runtime_error("Failed to create event");
            }
        }
    }

    // Деструктор - освобождение ресурсов
    ~MarkerManager() {
        for (auto& thread : threads) {
            if (thread) CloseHandle(thread);
        }
        for (auto& event : stopEvents) {
            if (event) CloseHandle(event);
        }
        for (auto& event : continueEvents) {
            if (event) CloseHandle(event);
        }
        DeleteCriticalSection(&cs);
    }

    // Основной метод запуска
    void run() {
        // Создание потоков
        for (int i = 0; i < numThreads; ++i) {
            threads[i] = CreateThread(NULL, 0, marker, this, 0, NULL);
            if (!threads[i]) {
                throw std::runtime_error("Failed to create thread");
            }
        }

        // Основной цикл управления
        for (int i = 0; i < numThreads; ++i) {
            resetEvents(stopEvents);
            // Ожидание остановки всех потоков
            WaitForMultipleObjects(numThreads, stopEvents.data(), TRUE, INFINITE);

            std::cout << "Array state:\n";
            showArray();  // Вывод состояния массива

            // Выбор потока для остановки
            int threadToStop;
            while (true) {
                std::cout << "Active threads: ";
                for (int j = 0; j < numThreads; ++j) {
                    if (isWorking[j]) {
                        std::cout << j + 1 << " ";
                    }
                }
                std::cout << "\nEnter thread number to stop (1-" << numThreads << "): ";
                std::cin >> threadToStop;
                threadToStop--;

                if (threadToStop >= 0 && threadToStop < numThreads && isWorking[threadToStop]) {
                    isWorking[threadToStop] = false;
                    ResetEvent(stopEvents[threadToStop]);
                    SetEvent(continueEvents[threadToStop]);  // Разрешаем потоку продолжить для завершения
                    WaitForSingleObject(stopEvents[threadToStop], INFINITE);  // Ожидаем завершения
                    break;
                }
                std::cout << "Invalid thread number, try again.\n";
            }

            // Продолжение работы оставшихся потоков
            for (int j = 0; j < numThreads; ++j) {
                if (isWorking[j]) {
                    SetEvent(continueEvents[j]);
                }
            }
        }

        // Ожидание завершения всех потоков
        WaitForMultipleObjects(numThreads, threads.data(), TRUE, INFINITE);
    }
};

int main() {
    try {
        int arraySize, threadCount;

        // Ввод параметров
        std::cout << "Enter number of elements in array: ";
        std::cin >> arraySize;

        std::cout << "Enter number of threads: ";
        std::cin >> threadCount;

        // Создание и запуск менеджера
        MarkerManager manager(arraySize, threadCount);
        manager.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
