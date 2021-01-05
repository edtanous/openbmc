#include "primary_src.hpp"

namespace attn
{
namespace pel
{

PrimarySrc::PrimarySrc(Stream& pel)
{
    unflatten(pel);
}

void PrimarySrc::flatten(Stream& stream) const
{
    stream << _header << _version << _flags << _reserved1B << _wordCount
           << _reserved2B << _size;

    for (auto& word : _srcWords)
    {
        stream << word;
    }

    stream.write(_asciiString.data(), _asciiString.size());
}

void PrimarySrc::unflatten(Stream& stream)
{
    stream >> _header >> _version >> _flags >> _reserved1B >> _wordCount >>
        _reserved2B >> _size;

    for (auto& word : _srcWords)
    {
        stream >> word;
    }

    stream.read(_asciiString.data(), _asciiString.size());
}

void PrimarySrc::setSrcWords(std::array<uint32_t, numSrcWords> srcWords)
{
    _srcWords = srcWords;
}

void PrimarySrc::setAsciiString(std::array<char, asciiStringSize> asciiString)
{
    _asciiString = asciiString;
}

} // namespace pel
} // namespace attn
