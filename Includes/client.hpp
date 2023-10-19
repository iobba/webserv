#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "webserv.hpp"
#include "request.hpp"

class   Client
{
    private:
        int                 _socket;

    public:
        bool                _is_favicon;
        Request             _request;
        bool                _first_send;
        unsigned long int   _sending_offset;

        void        set_socket(int socket);
        const int&  get_socket(void)     const;
};

#endif