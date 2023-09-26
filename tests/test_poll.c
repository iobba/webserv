#include <iostream>
#include <unistd.h>
#include <fcntl.h>


int main() {
    // Open a file for reading
    int fileDescriptor = open("/nfs/homes/iobba/Desktop/webserv/tests/yoooo.txt", O_RDONLY);
    if (fileDescriptor == -1) {
        std::cerr << "Error opening file for reading" << std::endl;
        return 1;
    }

    // Read from the file
    char buffer[1];
    ssize_t bytesRead;
    while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0) {
        // Process the data read (e.g., print it)
        write(STDOUT_FILENO, buffer, bytesRead);
        std::cout << "\n";
    }

    // Close the file
    close(fileDescriptor);

    return 0;
}
