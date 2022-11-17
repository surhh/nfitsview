#ifndef LIBNFITS_FITS2PNG_H
#define LIBNFITS_FITS2PNG_H

#include <cstdint>
#include <string>

namespace libnfits
{

//// This function will be needed in the future, to support standalone conversions via libnfits
int32_t convertFITS2PNG(const std::string& a_fitsFileName, const std::string& a_pngFileName = "");

}

#endif // LIBNFITS_FITS2PNG_H
