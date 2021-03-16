#include <iostream>
#include "uuid.h"

std::vector<char> hex_to_bytes(const std::string& hex) {
	std::vector<char> bytes;

	for (unsigned int i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		char byte = (char)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}

void bytes_to_hex(uint8_t* u_uuid, char* res)
{
	sprintf(res,
		"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		u_uuid[0], u_uuid[1], u_uuid[2], u_uuid[3], u_uuid[4], u_uuid[5], u_uuid[6], u_uuid[7],
		u_uuid[8], u_uuid[9], u_uuid[10], u_uuid[11], u_uuid[12], u_uuid[13], u_uuid[14], u_uuid[15]
	);
}

std::string get_uuid_string_from_bytes(uint8_t* uuid)
{
	char temp[37] = {};
	temp[36] = 0;
	bytes_to_hex(uuid, temp);
	return std::string(temp);
}
