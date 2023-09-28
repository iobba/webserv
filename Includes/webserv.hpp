#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdio> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <csignal>
#include <vector>
#include <dirent.h>
#include <sys/time.h>
#include <algorithm>
#include <map>
#include <sstream>
#include <ctime>
#include <string>
#include <exception>
#include <fstream>
#include <cstdlib>
#include <fcntl.h>
#include <csignal>
#include <cstdio> // remove files
#include <unistd.h>
#include <sys/stat.h>
#include <iterator>

# define PORT "3490"  // the port users will be connecting to
# define BACKLOG 10   // how many pending connections queue will hold
# define BUFFER_SIZE 1024 // Size of the buffer to read incoming data
# define MAX_URI_length 30
# define MAX_BODY_SIZE 100000000 // this value not correct, cuz it should be taken from the config file 

//extern long unsigned int num_files; // i am not sure if it is allowed to use a globale variable


// possible requests
# define GET 1
# define POST 2
# define DELETE 3
# define NONE 0
# define FILE_BODY 1
# define STR_BODY 2

class HTTPException : public std::exception
{
    private:
        int error_code_;
        std::string error_str;

    public:
        HTTPException(int error_code);

        const char* what() const throw();
    ~HTTPException() throw() {}
};

class CONFIGException : public std::exception
{
    private:
        std::string __error;

    public:
        CONFIGException(std::string error);

        const char* what() const throw();
    ~CONFIGException() throw() {}
};

class SERVER_Exception : public std::exception
{
    private:
        std::string __error;

    public:
        SERVER_Exception(std::string error);

        const char* what() const throw();
    ~SERVER_Exception() throw() {}
};

/// server ////
int launch_server(void);

// global functions
int                         which_type(std::string  pa_th);
std::vector<std::string>    split_to_lines(std::string all_scope);
std::vector<std::string>    split_to_words(std::string line);
int                         find_in_vector(std::vector<char> __vec, std::string to_find);
long unsigned int           get_file_len(std::string file__name);
std::string                 get_file_extention(std::string file_name);
void                        clear_and_replace(std::string file_name, std::string new_content);
std::string                 get_conetnt_type(std::string to_find, int i);

#endif