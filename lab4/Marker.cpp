#include <Windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

extern size_t arrSize;
extern int* common_array;
extern HANDLE hStartEvent;
extern HANDLE hContinueEvent;
extern HANDLE* stopEventsArr;
extern BOOL* terminateEventsArr;
extern CRITICAL_SECTION common_arrayCS;

DWORD WINAPI Marker(LPVOID number) {
    // Инициализация потока
    size_t threadNum = reinterpret_cast<size_t>(number);
    size_t markedItems = 0;

    std::vector<int> positionsArr(arrSize, -1);

    srand(static_cast<unsigned int>(threadNum + 1));
    std::cout << "Thread " << threadNum + 1 << " created..." << std::endl;

    // Ожидаем начала работы
    WaitForSingleObject(hStartEvent, INFINITE);

    // Работаем, пока поток не будет завершён
    while (true) {
        size_t position = rand() % arrSize;

        EnterCriticalSection(&common_arrayCS);
        if (common_array[position] == 0 && markedItems < arrSize) {
            Sleep(5);
            common_array[position] = static_cast<int>(threadNum + 1);
            Sleep(5);
            LeaveCriticalSection(&common_arrayCS);

            positionsArr[position] = 0;
            ++markedItems;
        } else {
            LeaveCriticalSection(&common_arrayCS);

            std::cout << "\nThread num " << threadNum + 1 << std::endl;
            std::cout << "Elements marked: " << markedItems << std::endl;
            std::cout << "Unmarkable position: " << position << std::endl;

            SignalObjectAndWait(stopEventsArr[threadNum], hContinueEvent, INFINITE, FALSE);

            if (terminateEventsArr[threadNum]) {
                EnterCriticalSection(&common_arrayCS);
                for (size_t i = 0; i < arrSize; ++i) {
                    if (positionsArr[i] != -1) {
                        common_array[i] = 0;
                    }
                }
                LeaveCriticalSection(&common_arrayCS);

                std::cout << "Thread " << threadNum + 1 << " is dead..." << std::endl;
                SetEvent(stopEventsArr[threadNum]);
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}
