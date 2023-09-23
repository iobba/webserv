#include <iostream>
#include <string>
#include <ctime>

int main() {
    // Get the current time
    std::time_t currentTime;
    std::time(&currentTime);

    // Format the current time as "Wed, 14 Sep 2023 12:00:00 GMT"
    char buffer[50];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&currentTime));

    // Get the formatted time as a string
    std::string dateHeader(buffer);

    std::cout << "Date Header: " << dateHeader << std::endl;

    return 0;
}
