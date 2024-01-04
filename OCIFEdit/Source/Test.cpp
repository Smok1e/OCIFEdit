#include <fstream>

#include <OCIF/IO.hpp>

//===========================================

int main()
{
	std::ofstream stream("test.txt", std::ios::binary);
	OCIF::WriteUnicodeCharacter(stream, 0x0024);
	OCIF::WriteUnicodeCharacter(stream, 0x0418);
	OCIF::WriteUnicodeCharacter(stream, 0x20AC);
	stream.close();
}

//===========================================