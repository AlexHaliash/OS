#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <vector>
#include <memory> // для std::unique_ptr

size_t arrSize = 0;
size_t threadsAmnt = 0;
std::unique_ptr<int[]> common_array; // Умный указатель вместо new/delete

HANDLE hStartEvent;
HANDLE hContinueEvent;
std::vector<HANDLE> stopEventsArr;
std::vector<BOOL> terminateEventsArr;
CRITICAL_SECTION common_arrayCS;

DWORD WINAPI Marker(LPVOID number);

bool CheckHandle(HANDLE h) {
    if (h == NULL || h == INVALID_HANDLE_VALUE) {
        std::cerr << "Error during handle creation. Error code: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}

bool CloseHandles(const std::vector<HANDLE>& handles) {
    bool success = true;
    for (HANDLE h : handles) {
        if (h != NULL && h != INVALID_HANDLE_VALUE) {
            if (!CloseHandle(h)) {
                std::cerr << "Failed to close handle. Error code: " << GetLastError() << std::endl;
                success = false;
            }
        }
    }
    return success;
}

int main() {
    std::cout << "Enter array size: ";
    std::cin >> arrSize;

    common_array = std::make_unique<int[]>(arrSize); // Безопасное выделение памяти
    
    std::cout << "Enter number of threads: ";
    std::cin >> threadsAmnt;

    std::vector<HANDLE> threadsArr(threadsAmnt);
    stopEventsArr.resize(threadsAmnt);
    terminateEventsArr.resize(threadsAmnt, FALSE);

    hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!CheckHandle(hStartEvent)) return EXIT_FAILURE;

    hContinueEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!CheckHandle(hContinueEvent)) return EXIT_FAILURE;

    InitializeCriticalSection(&common_arrayCS);

    for (size_t i = 0; i < threadsAmnt; ++i) {
        stopEventsArr[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!CheckHandle(stopEventsArr[i])) {
            CloseHandles(threadsArr);
            return EXIT_FAILURE;
        }

        threadsArr[i] = CreateThread(NULL, 0, Marker, reinterpret_cast<LPVOID>(i), 0, NULL);
        if (!CheckHandle(threadsArr[i])) {
            CloseHandles(threadsArr);
            return EXIT_FAILURE;
        }
    }

    SetEvent(hStartEvent);

    int currThrdsAmnt = static_cast<int>(threadsAmnt);
    while (currThrdsAmnt > 0) {
        WaitForMultipleObjects(static_cast<DWORD>(threadsAmnt), stopEventsArr.data(), TRUE, INFINITE);
        std::cout << "\nThreads paused. Current array state:\n";

        for (size_t i = 0; i < arrSize; ++i) {
            std::cout << common_array[i] << ' ';
        }
        std::cout << std::endl;

        size_t currNum;
        while (true) {
            std::cout << "\nEnter thread number to terminate (1 to " << threadsAmnt << "): ";
            std::cin >> currNum;

            if (currNum >= 1 && currNum <= threadsAmnt && !terminateEventsArr[currNum - 1])
                break;
            else
                std::cout << "Invalid or already terminated.\n";
        }

        terminateEventsArr[currNum - 1] = TRUE;
        SetEvent(hContinueEvent); // Надежнее, чем PulseEvent
        --currThrdsAmnt;
    }

    DeleteCriticalSection(&common_arrayCS);
    CloseHandles(threadsArr);
    CloseHandles(stopEventsArr);

    CloseHandle(hStartEvent);
    CloseHandle(hContinueEvent);

    return EXIT_SUCCESS;
}
