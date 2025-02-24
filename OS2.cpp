#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <mutex>

std::mutex mtx; // ������� ��� ������������� ������� � �������
std::vector<int> numbers;
int min_value, max_value;
double average_value;

void min_max() {
    std::lock_guard<std::mutex> lock(mtx);
    min_value = *std::min_element(numbers.begin(), numbers.end());
    max_value = *std::max_element(numbers.begin(), numbers.end());

    std::cout << "����������� �������: " << min_value << std::endl;
    std::cout << "������������ �������: " << max_value << std::endl;

    // �������� ��������
    for (size_t i = 0; i < numbers.size(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
    }
}

void average() {
    std::lock_guard<std::mutex> lock(mtx);
    average_value = static_cast<double>(std::accumulate(numbers.begin(), numbers.end(), 0)) / numbers.size();

    std::cout << "������� ��������: " << average_value << std::endl;

    // �������� ��������
    for (size_t i = 0; i < numbers.size(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
}

int main() {
    int size;
    std::cout << "������� razmer �������: ";
    std::cin >> size;

    numbers.resize(size);
    std::cout << "������� elementi �������: ";
    for (int i = 0; i < size; ++i) {
        std::cin >> numbers[i];
    }

    std::thread t1(min_max);
    std::thread t2(average);

    t1.join();
    t2.join();

    // �������� ����������� � ������������ �������� �� ������� ��������
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::replace(numbers.begin(), numbers.end(), min_value, static_cast<int>(average_value));
        std::replace(numbers.begin(), numbers.end(), max_value, static_cast<int>(average_value));
    }

    std::cout << "���������������� ������: ";
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}