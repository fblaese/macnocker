#include "nockpackage.h"

#include <sstream>

NockPackage::NockPackage()
{

}

NockPackage::NockPackage(const Mac &sourceMac, const Hood &hoodName)
    : m_sourceMac(sourceMac)
    , m_hoodName(hoodName)
{

}

size_t NockPackage::serialize(uint8_t *buf, size_t maxlen) const
{
    uint8_t *target = buf;
    if (maxlen < (m_sourceMac.size() + 1))
        return 0;

    *target = VERSION;
    target++;

    for (Mac::const_iterator it=m_sourceMac.begin(); it!=m_sourceMac.end(); ++it)
    {
        *target = *it;
        target++;
    }

    for (Hood::const_iterator it=m_hoodName.begin(); it!=m_hoodName.end(); ++it)
    {
        *target = *it;
        target++;
    }

    return target-buf;
}

bool NockPackage::deserialize(const uint8_t *buf, size_t len)
{
    const uint8_t *source = buf;

    if (len < (m_sourceMac.size() + 1))
    {
        return false;
    }

    if (*source != VERSION)
    {
        return false;
    }
    source++;

    for (Mac::iterator it=m_sourceMac.begin(); it!=m_sourceMac.end(); ++it)
    {
        *it = *source;
        source++;
    }

    m_hoodName = std::string(source, buf+len);

    return true;
}

Mac NockPackage::getMac() const
{
    return m_sourceMac;
}

std::string NockPackage::getHood() const
{
    return m_hoodName;
}
