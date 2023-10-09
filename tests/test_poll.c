#include <iostream>
#include <map>

int main() {
    std::map<std::string, int> myMap;
    
    // Insert some key-value pairs into the map
    myMap["apple"] = 5;
    myMap["banana"] = 3;
    myMap["cherry"] = 8;
    
    // Specify the key you want to remove
    std::string keyToRemove = "banana";
    
    // Use the erase() method to remove the element
    myMap.erase(keyToRemove);
    
    // Print the updated map
    for (const auto& pair : myMap) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    return 0;
}
