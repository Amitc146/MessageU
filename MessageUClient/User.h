/*
* User operations.
*/

#pragma once

#include <rsa.h>
#include <aes.h>

class User
{
	std::string m_ClientId;
	std::string m_Username;
	CryptoPP::RSA::PublicKey* m_PublicKey;
	CryptoPP::byte* m_SymmetricKey;

public:
	User(std::string clientId, std::string username);
	~User();

	std::string GetClientId();
	std::string GetUsername();
	CryptoPP::RSA::PublicKey* GetPublicKey();
	CryptoPP::byte* GetSymmetricKey();

	void SetClientId(std::string clientId);
	void SetUsername(std::string username);
	void SetPublicKey(CryptoPP::RSA::PublicKey publicKey);
	void SetSymmetricKey(CryptoPP::byte* symmetricKey);

	void PrintUser();
};