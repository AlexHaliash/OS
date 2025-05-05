#include <iostream>
#include <windows.h>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace std;

// Глобальные данные для обработки
vector<int> data_values;
double calculated_average;
int found_min;
int found_max;
HANDLE min_max_thread_handle;
HANDLE average_thread_handle;

// Функция потока для поиска минимального и максимального значений
DWORD WINAPI FindMinMax(LPVOID param) {
    found_min = *min_element(data_values.begin(), data_values.end());
    found_max = *max_element(data_values.begin(), data_values.end());

    // Имитация обработки данных
    Sleep(7);
    cout << "Minimum value: " << found_min << ", Maximum value: " << found_max << endl;

    return 0;
}

// Функция потока для вычисления среднего значения
DWORD WINAPI CalculateAverage(LPVOID param) {
    calculated_average = accumulate(data_values.begin(), data_values.end(), 0.0) / data_values.size();

    // Имитация обработки данных
    Sleep(12);
    cout << "Average value: " << calculated_average << endl;

    return 0;
}

int main() {
    int data_size;

    // Ввод данных
    cout << "Enter array size: ";
    cin >> data_size;

    data_values.resize(data_size);
    cout << "Enter " << data_size << " integer elements:\n";
    for (int i = 0; i < data_size; ++i) {
        cin >> data_values[i];
    }

    // Создание и запуск потоков
    min_max_thread_handle = CreateThread(NULL, 0, FindMinMax, NULL, 0, NULL);
    average_thread_handle = CreateThread(NULL, 0, CalculateAverage, NULL, 0, NULL);

    // Ожидание завершения потоков
    WaitForSingleObject(min_max_thread_handle, INFINITE);
    WaitForSingleObject(average_thread_handle, INFINITE);

    // Модификация массива - замена min/max на среднее значение
    for (int& value : data_values) {
        if (value == found_min || value == found_max) {
            value = static_cast<int>(calculated_average);
        }
    }

    // Вывод результатов
    cout << "Processed array: ";
    for (int value : data_values) {
        cout << value << " ";
    }
    cout << endl;

    // Освобождение ресурсов
    CloseHandle(min_max_thread_handle);
    CloseHandle(average_thread_handle);

    return 0;
}
