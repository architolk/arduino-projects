#include "DEV_Tools.h"

void DEV_SPI_WriteByte(UBYTE data)
{
    SPI.transfer(data);
}
