#include "extended_user_header.hpp"

namespace attn
{
namespace pel
{

ExtendedUserHeader::ExtendedUserHeader(Stream& pel)
{
    unflatten(pel);
}

void ExtendedUserHeader::flatten(Stream& pel) const
{
    pel << _header;
    pel.write(_mtms, mtmsSize);
    pel.write(_serverFWVersion.data(), _serverFWVersion.size());
    pel.write(_subsystemFWVersion.data(), _subsystemFWVersion.size());
    pel << _reserved4B << _refTime << _reserved1B1 << _reserved1B2
        << _reserved1B3 << _symptomIdSize << _symptomId;
}

void ExtendedUserHeader::unflatten(Stream& pel)
{
    pel >> _header;
    pel.read(_mtms, mtmsSize);
    pel.read(_serverFWVersion.data(), _serverFWVersion.size());
    pel.read(_subsystemFWVersion.data(), _subsystemFWVersion.size());
    pel >> _reserved4B >> _refTime >> _reserved1B1 >> _reserved1B2 >>
        _reserved1B3 >> _symptomIdSize >> _symptomId;

    //_symptomId.resize(_symptomIdSize);
    pel >> _symptomId;
}

void ExtendedUserHeader::setSymptomId(const std::string& symptomId)
{
    // set symptomId to new symptomId
    std::copy(symptomId.begin(), symptomId.end(),
              std::back_inserter(_symptomId));

    // new symptom Id cannot be larger than existing symptom Id
    if (_symptomId.size() > size_t((_symptomIdSize - 1)))
    {
        _symptomId.resize(_symptomIdSize - 1);
    }

    // null terminate new symptom Id (it may have been smaller)
    _symptomId.push_back(0);

    // pad if new symptom ID (it may have been smaller)
    while ((_symptomId.size() != _symptomIdSize))
    {
        _symptomId.push_back(0);
    }
}

} // namespace pel
} // namespace attn
