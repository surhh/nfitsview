#include "fits2png.h"
#include "defs.h"

namespace libnfits
{

int32_t convertFITS2PNG(const std::string& a_fitsFileName, const std::string& a_pngFileName)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    std::string fitsFileName = a_fitsFileName, pngFileName = a_pngFileName;

    //// TODO: This function will be needed in the future, to support command-line interface.
    ////       Will be implemented when converting FITS to PNG via command line arguments.

    return retVal;
}


}
