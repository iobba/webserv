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
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <iterator>
#include <sys/wait.h>

# define BACKLOG 100
# define BUFFER_SIZE 1024
# define MAX_URI_length 50
# define CHILD_TIMEOUT 5
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

int                         launch_server(void);
int                         which_type(std::string  pa_th);
std::vector<std::string>    split_to_lines(std::string all_scope);
std::vector<std::string>    split_to_words(std::string line);
int                         find_in_vector(std::vector<char> __vec, std::string to_find);
long unsigned int           get_file_len(std::string file__name);
std::string                 get_file_extention(std::string file_name);
std::string                 get_content_type(std::string to_find, int i);
std::string                 generate_html_page_dir(std::string dir_path);
int                         delete_directory_contents(std::string path);
int                         create_file(std::string _ext_, std::string& naaaame);

#endif