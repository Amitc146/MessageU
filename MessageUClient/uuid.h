/*
* UUID operations.
*/

#pragma once
#include <vector>

std::vector<char> hex_to_bytes(const std::string& hex);
void bytes_to_hex(uint8_t* u_uuid, char* res);
std::string get_uuid_string_from_bytes(uint8_t* uuid);
