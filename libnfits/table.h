#ifndef LIBNFITS_TABLE_H
#define LIBNFITS_TABLE_H

#include <cstdint>

//// Tables are not supported yet
namespace libnfits
{

class Table
{
private:
    uint8_t*            m_dataBuffer;

public:
    Table();
};

}
#endif // LIBNFITS_TABLE_H
