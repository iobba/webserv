#include "../Includes/client.hpp"

void    Client::set_socket(int socket)
{
    this->_socket = socket;
    this->_first_send = true;
    this->_sending_offset = 0;
}

const int& Client::get_socket(void)     const
{
    return (this->_socket);
}



