#include "slp.hpp"

slp::Message::Message(const slp::Message& rhs)
{
    *this = rhs;
}

slp::Message& slp::Message::operator=(const slp::Message& rhs)
{

    if (this != &rhs)
    {
        header = rhs.header;
        body = rhs.body;
    }
}


slp::Payload::Payload(const slp::Payload& rhs)
{
    *this = rhs;
}

slp::Payload& slp::Payload::operator=(const slp::Payload& rhs)
{
    if (this != &rhs)
    {
        this->srvtyperqst = rhs.srvtyperqst ;
        this->srvrqst = rhs.srvrqst;
    }
}
