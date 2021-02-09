#include "Convert.h"
#include <string>
#include <iostream>

#define _STDOUT(target) std::cout << #target << ": " << target << std::endl

int main(int argc, char* argv[])
{
    unsigned long integer       = 0x12345678;
    std::string   base64Encoded = Convert::ToBase64("test");
    std::string   base64Decoded = Convert::FromBase64(base64Encoded);
    std::string   hexEncoded    = Convert::ToHex((const unsigned char*)&integer, sizeof(integer));
    std::string   hexDecoded    = Convert::FromHex(hexEncoded);
    _STDOUT(integer);
    _STDOUT(base64Encoded);
    _STDOUT(base64Decoded);
    _STDOUT(hexEncoded);
    _STDOUT(hexDecoded);
    return 0;
}