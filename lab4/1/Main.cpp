#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <vector>
#include <conio.h>

size_t arrSize = 0;
size_t threadsAmnt = 0;
int* common_array = nullptr;

HANDLE hStartEvent;
HANDLE hContinueEvent;
std::vector<HANDLE> stopEventsArr;
std::vector<BOOL> terminateEventsArr;
CRITICAL_SECTION common_arrayCS;

DWORD WINAPI Marker(LPVOID number);

bool CheckHandle(HANDLE h) {
    if (h == NULL) {
        std::cerr << "Error during handle creation. Error code: " << GetLastError() << std::endl;
        system("pause");
        return false;
    }
    return true;
}

bool CloseHandles(const std::vector<HANDLE>& handles) {
    for (HANDLE h : handles) {
        if (!CloseHandle(h)) return false;
    }
    return true;
}

int main() {
    std::cout << "Enter array size: ";
    std::cin >> arrSize;

    common_array = new int[arrSize] {};
    
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
        if (!CheckHandle(stopEventsArr[i])) return EXIT_FAILURE;

        threadsArr[i] = CreateThread(NULL, 0, Marker, (LPVOID)i, 0, NULL);
        if (!CheckHandle(threadsArr[i])) return EXIT_FAILURE;
    }

    SetEvent(hStartEvent);

    int currThrdsAmnt = static_cast<int>(threadsAmnt);
    while (currThrdsAmnt > 0) {
        WaitForMultipleObjects(static_cast<DWORD>(threadsAmnt), stopEventsArr.data(), TRUE, 450);
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
        PulseEvent(hContinueEvent);
        --currThrdsAmnt;
    }

    DeleteCriticalSection(&common_arrayCS);
    CloseHandles(threadsArr);
    CloseHandles(stopEventsArr);

    delete[] common_array;

    CloseHandle(hStartEvent);
    CloseHandle(hContinueEvent);

    return EXIT_SUCCESS;
}
