#include "user_header.hpp"

namespace attn
{
namespace pel
{

UserHeader::UserHeader(Stream& pel)
{
    unflatten(pel);
}

void UserHeader::flatten(Stream& stream) const
{
    stream << _header << _eventSubsystem << _eventScope << _eventSeverity
           << _eventType << _reserved4Byte1 << _problemDomain << _problemVector
           << _actionFlags << _reserved4Byte2;
}

void UserHeader::unflatten(Stream& stream)
{
    stream >> _header >> _eventSubsystem >> _eventScope >> _eventSeverity >>
        _eventType >> _reserved4Byte1 >> _problemDomain >> _problemVector >>
        _actionFlags >> _reserved4Byte2;
}

void UserHeader::setSubsystem(uint8_t subsystem)
{
    _eventSubsystem = subsystem;
}

void UserHeader::setSeverity(uint8_t severity)
{
    _eventSeverity = severity;
}

void UserHeader::setType(uint8_t type)
{
    _eventType = type;
}

void UserHeader::setAction(uint16_t action)
{
    _actionFlags = action;
}

} // namespace pel
} // namespace attn
