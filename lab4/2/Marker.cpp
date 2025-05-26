#include <Windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

extern size_t arrSize;
extern std::unique_ptr<int[]> common_array;
extern HANDLE hStartEvent;
extern HANDLE hContinueEvent;
extern std::vector<HANDLE> stopEventsArr;
extern std::vector<BOOL> terminateEventsArr;
extern CRITICAL_SECTION common_arrayCS;

DWORD WINAPI Marker(LPVOID number) {
    size_t threadNum = reinterpret_cast<size_t>(number);
    size_t markedItems = 0;

    std::vector<bool> markedPositions(arrSize, false); // Лучше bool, чем int

    srand(static_cast<unsigned int>(threadNum + 1));
    std::cout << "Thread " << threadNum + 1 << " created..." << std::endl;

    WaitForSingleObject(hStartEvent, INFINITE);

    while (true) {
        size_t position = rand() % arrSize;

        EnterCriticalSection(&common_arrayCS);
        if (common_array[position] == 0) {
            Sleep(5);
            common_array[position] = static_cast<int>(threadNum + 1);
            Sleep(5);
            markedPositions[position] = true;
            ++markedItems;
            LeaveCriticalSection(&common_arrayCS);
        } else {
            LeaveCriticalSection(&common_arrayCS);

            std::cout << "\nThread num " << threadNum + 1 << std::endl;
            std::cout << "Elements marked: " << markedItems << std::endl;
            std::cout << "Unmarkable position: " << position << std::endl;

            SetEvent(stopEventsArr[threadNum]); // Сигнализируем о остановке
            WaitForSingleObject(hContinueEvent, INFINITE); // Ждем продолжения

            if (terminateEventsArr[threadNum]) {
                EnterCriticalSection(&common_arrayCS);
                for (size_t i = 0; i < arrSize; ++i) {
                    if (markedPositions[i]) {
                        common_array[i] = 0;
                    }
                }
                LeaveCriticalSection(&common_arrayCS);

                std::cout << "Thread " << threadNum + 1 << " is dead..." << std::endl;
                SetEvent(stopEventsArr[threadNum]);
                return 0; // Корректное завершение потока
            }
        }
    }
}
