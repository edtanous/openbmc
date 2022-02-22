#include "pel_minimal.hpp"

#include "stream.hpp"

namespace attn
{
namespace pel
{

PelMinimal::PelMinimal(std::vector<uint8_t>& data)
{
    initialize(data);
}

void PelMinimal::initialize(std::vector<uint8_t>& data)
{
    Stream pelData{data};

    _ph = std::make_unique<PrivateHeader>(pelData);
    _uh = std::make_unique<UserHeader>(pelData);
    _ps = std::make_unique<PrimarySrc>(pelData);
    _eh = std::make_unique<ExtendedUserHeader>(pelData);
}

void PelMinimal::raw(std::vector<uint8_t>& pelBuffer) const
{
    Stream pelData{pelBuffer};

    // stream from object to buffer
    _ph->flatten(pelData);
    _uh->flatten(pelData);
    _ps->flatten(pelData);
    _eh->flatten(pelData);
}

size_t PelMinimal::size() const
{
    size_t size = 0;

    // size of private header section
    if (_ph)
    {
        size += _ph->header().size;
    }

    // size of user header section
    if (_uh)
    {
        size += _uh->header().size;
    }

    // size of primary SRC section
    if (_ps)
    {
        size += _ph->header().size;
    }

    // size of extended user section
    if (_eh)
    {
        size += _eh->header().size;
    }

    return ((size > _maxPELSize) ? _maxPELSize : size);
}

void PelMinimal::setSubsystem(uint8_t subsystem)
{
    _uh->setSubsystem(subsystem);
}

void PelMinimal::setSeverity(uint8_t severity)
{
    _uh->setSeverity(severity);
}

void PelMinimal::setType(uint8_t type)
{
    _uh->setType(type);
}

void PelMinimal::setAction(uint16_t action)
{
    _uh->setAction(action);
}

void PelMinimal::setSrcWords(std::array<uint32_t, numSrcWords> srcWords)
{
    _ps->setSrcWords(srcWords);
}

void PelMinimal::setAsciiString(std::array<char, asciiStringSize> asciiString)
{
    _ps->setAsciiString(asciiString);
}

uint8_t PelMinimal::getSectionCount()
{
    return _ph->getSectionCount();
}

void PelMinimal::setSectionCount(uint8_t sectionCount)
{
    _ph->setSectionCount(sectionCount);
}

void PelMinimal::setSymptomId(const std::string& symptomId)
{
    _eh->setSymptomId(symptomId);
}

void PelMinimal::setPlid(uint32_t plid)
{
    _ph->setPlid(plid);
}

} // namespace pel
} // namespace attn
