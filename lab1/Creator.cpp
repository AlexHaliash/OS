#include <iostream>
#include <fstream>
#include <string>

struct employee {
    int num; 
    char name[10]; 
    double hours; 
};

void createBinaryFile(const std::string& filename, int recordCount) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    for (int i = 0; i < recordCount; ++i) {
        employee emp;
        std::cout << "Enter employee ID: ";
        std::cin >> emp.num;
        std::cout << "Enter employee name: ";
        std::cin >> emp.name;
        std::cout << "Enter number of hours worked: ";
        std::cin >> emp.hours;

        outFile.write(reinterpret_cast<char*>(&emp), sizeof(emp));
    }

    outFile.close();
}
