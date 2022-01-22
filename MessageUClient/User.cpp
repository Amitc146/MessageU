#include <iostream>
#include "User.h"

User::User(const std::string& clientId, const std::string& username) : m_PublicKey(nullptr), m_SymmetricKey(nullptr)
{
	m_ClientId = clientId;
	m_Username = username;
}

User::~User()
{
	if (m_PublicKey)
		delete m_PublicKey;
	if (m_SymmetricKey)
		delete[] m_SymmetricKey;
}

std::string User::GetClientId()
{
	return m_ClientId;
}

std::string User::GetUsername()
{
	return m_Username;
}

CryptoPP::RSA::PublicKey* User::GetPublicKey()
{
	return m_PublicKey;
}

CryptoPP::byte* User::GetSymmetricKey()
{
	return m_SymmetricKey;
}

void User::SetClientId(const std::string& clientId)
{
	m_ClientId = clientId;
}

void User::SetUsername(const std::string& username)
{
	m_Username = username;
}

void User::SetPublicKey(CryptoPP::RSA::PublicKey publicKey)
{
	m_PublicKey = new CryptoPP::RSA::PublicKey(publicKey);
}

void User::SetSymmetricKey(CryptoPP::byte* symmetricKey)
{
	m_SymmetricKey = new CryptoPP::byte[CryptoPP::AES::DEFAULT_KEYLENGTH];
	memcpy(m_SymmetricKey, symmetricKey, CryptoPP::AES::DEFAULT_KEYLENGTH);
}

void User::PrintUser()
{
	std::cout << "{ ID: " << m_ClientId << ", Username: " << m_Username;
	std::cout << " }" << std::endl;
}