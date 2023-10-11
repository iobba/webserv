// #include <iostream>
// #include <filesystem>

// int main() {
//     const std::string directoryPath = "/Users/iobba/Desktop/our_root/videos/to_delete/"; // Replace with the directory path you want to delete.

//     try {
//         std::filesystem::remove_all(directoryPath);
//         std::cout << "Directory and its contents deleted successfully." << std::endl;
//     } catch (const std::filesystem::filesystem_error& e) {
//         std::cerr << "Error deleting directory: " << e.what() << std::endl;
//     }

//     return 0;
// }
#include <cstdlib>
#include <iostream>

int main() {
    std::string directoryPath = "/Users/iobba/Desktop/our_root/videos/to_delete/"; // Replace with the directory path you want to delete.

    int result = std::system(("rm -r " + directoryPath).c_str());
    if (result == 0) {
        // Directory deleted successfully
        std::cout << "HHHHHHHHHHHH\n";
    } else {
        // Handle the error
    }

    return 0;
}
