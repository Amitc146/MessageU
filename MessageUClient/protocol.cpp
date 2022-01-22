#include "protocol.h"

using namespace protocol;

RequestHeader::RequestHeader(const char& clientId, uint8_t code, uint32_t payloadSize) :
	m_Version(client_version), m_Code(code), m_PayloadSize(payloadSize)
{
	if (clientId)
		memcpy(m_ClientId, &clientId, id_length);
	else
	{
		std::cout << "Failed to create header. Client ID is missing." << std::endl;
		exit(1);
	}
}

void RequestHeader::Serialize(char* buff)
{
	memcpy(buff, &m_ClientId, id_length);
	char* temp = buff + id_length;
	memcpy(temp, &m_Version, sizeof(m_Version));
	temp++;
	memcpy(temp, &m_Code, sizeof(m_Code));
	temp++;
	memcpy(temp, &m_PayloadSize, sizeof(m_PayloadSize));
}

RequestRegisterPayload::RequestRegisterPayload(const char& name, CryptoPP::RSA::PublicKey pubKey)
{
	CryptoPP::byte buff[public_key_length];
	CryptoPP::ArraySink as(buff, public_key_length);
	pubKey.Save(as);
	memcpy(m_PublicKey, &buff, public_key_length);

	for (int i = 0; i < username_length; i++)
		m_Name[i] = 0;
	strcpy(m_Name, &name);
}

void RequestRegisterPayload::Serialize(char* buff)
{
	memcpy(buff, m_Name, username_length);
	char* temp = buff + username_length;
	memcpy(temp, m_PublicKey, public_key_length);
}

uint32_t RequestRegisterPayload::GetPayloadSize()
{
	return username_length + public_key_length;
}

RequestPublicKeyPayload::RequestPublicKeyPayload(const char& targetId)
{
	memcpy(m_ClientId, &targetId, id_length);
}

void RequestPublicKeyPayload::Serialize(char* buff)
{
	memcpy(buff, m_ClientId, id_length);
}

uint32_t RequestPublicKeyPayload::GetPayloadSize()
{
	return id_length;
}


RequestMessagePayload::RequestMessagePayload(const char& targetId, uint8_t messageType, std::string messageContent) : m_MessageType(messageType), m_ContentSize(messageContent.length())
{
	memcpy(m_ClientId, &targetId, id_length);

	m_MessageContent = new char[m_ContentSize];

	for (uint32_t i = 0; i < m_ContentSize; i++)
		m_MessageContent[i] = 0;

	memcpy(m_MessageContent, messageContent.c_str(), m_ContentSize);
}

RequestMessagePayload::~RequestMessagePayload()
{
	delete m_MessageContent;
}

void RequestMessagePayload::Serialize(char* buff)
{
	memcpy(buff, m_ClientId, id_length);
	char* temp = buff + id_length;
	memcpy(temp, &m_ContentSize, sizeof(m_ContentSize));
	temp += sizeof(m_ContentSize);
	memcpy(temp, &m_MessageType, sizeof(m_MessageType));
	temp += sizeof(m_MessageType);
	memcpy(temp, m_MessageContent, m_ContentSize);
}

uint32_t RequestMessagePayload::GetPayloadSize()
{
	return id_length + sizeof(m_MessageType) + sizeof(m_ContentSize) + m_ContentSize;
}

ResponseRegisterPayload::ResponseRegisterPayload(tcp::socket* s)
{
	boost::asio::read(*s, boost::asio::buffer(&m_ClientId, id_length));
}

ResponseClientListPayload::ResponseClientListPayload(tcp::socket* s)
{
	m_Username[username_length] = 0;
	boost::asio::read(*s, boost::asio::buffer(&m_ClientId, id_length));
	boost::asio::read(*s, boost::asio::buffer(&m_Username, username_length));
}

ResponsePublicKeyPayload::ResponsePublicKeyPayload(tcp::socket* s)
{
	boost::asio::read(*s, boost::asio::buffer(&m_ClientId, id_length));
	boost::asio::read(*s, boost::asio::buffer(&m_PublicKey, public_key_length));
}

ResponsePullPayload::ResponsePullPayload(tcp::socket* s)
{
	boost::asio::read(*s, boost::asio::buffer(&m_ClientId, id_length));
	boost::asio::read(*s, boost::asio::buffer(&m_MessageId, sizeof(m_MessageId)));
	boost::asio::read(*s, boost::asio::buffer(&m_MessageType, sizeof(m_MessageType)));
	boost::asio::read(*s, boost::asio::buffer(&m_MessageSize, sizeof(m_MessageSize)));

	m_Content = new unsigned char[m_MessageSize];
	boost::asio::read(*s, boost::asio::buffer(m_Content, m_MessageSize));
}

ResponsePullPayload::~ResponsePullPayload()
{
	delete m_Content;
}

std::string ResponsePullPayload::GetContentAsString()
{
	return std::string(reinterpret_cast<char*>(m_Content), m_MessageSize);
}