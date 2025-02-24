#include <iostream>
#include <fstream>
#include <string>

struct employee {
    int num;
    char name[10];
    double hours;
};

void createBinaryFile(const std::string& filename, int recordCount);
void generateReport(const std::string& inputFile, const std::string& reportFile, double hourlyRate);

int main() {
    std::string binaryFileName;
    int recordCount;

    std::cout << "Enter binary file name: ";
    std::cin >> binaryFileName;
    std::cout << "Enter number of records: ";
    std::cin >> recordCount;

    createBinaryFile(binaryFileName, recordCount);

    std::ifstream inFile(binaryFileName, std::ios::binary);
    employee emp;
    std::cout << "Contents of the binary file:\n";
    while (inFile.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
        std::cout << emp.num << " " << emp.name << " " << emp.hours << "\n";
    }
    inFile.close();

    std::string reportFileName;
    double hourlyRate;
    std::cout << "Enter report file name: ";
    std::cin >> reportFileName;
    std::cout << "Enter hourly rate: ";
    std::cin >> hourlyRate;

    generateReport(binaryFileName, reportFileName, hourlyRate);

    std::cout << "Report generated in file: " << reportFileName << std::endl;

    return 0;
}

