#include "../Includes/client.hpp"

void    Client::set_socket(int socket)
{
    this->_socket = socket;
}

const int& Client::get_socket(void)     const
{
    return (this->_socket);
}



