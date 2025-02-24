#include <iostream>
#include <fstream>
#include <string>

struct employee {
    int num;
    char name[10];
    double hours;
};

void generateReport(const std::string& inputFile, const std::string& reportFile, double hourlyRate) {
    std::ifstream inFile(inputFile, std::ios::binary);
    std::ofstream outFile(reportFile);
    if (!inFile || !outFile) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    outFile << "Report for file \"" << inputFile << "\"\n";
    outFile << "ID\t Name\tHours\tSalary\n";

    employee emp;
    while (inFile.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
        double salary = emp.hours * hourlyRate;
        outFile << emp.num << "\t" << emp.name << "\t" << emp.hours << "\t" << salary << "\n";
    }

    inFile.close();
    outFile.close();
}
