#include "protocol.h"

using namespace protocol;

request::Header::Header(const char& clientId, uint8_t code, uint32_t payloadSize) :
	m_Version(client_version), m_Code(code), m_PayloadSize(payloadSize)
{
	if (clientId)
		memcpy(m_ClientId, &clientId, id_length);
}

void request::Header::Serialize(char* buff)
{
	memcpy(buff, &m_ClientId, id_length);
	char* temp = buff + id_length;
	memcpy(temp, &m_Version, sizeof(m_Version));
	temp++;
	memcpy(temp, &m_Code, sizeof(m_Code));
	temp++;
	memcpy(temp, &m_PayloadSize, sizeof(m_PayloadSize));
}

request::RegisterPayload::RegisterPayload(const char& name, CryptoPP::RSA::PublicKey pubKey)
{
	CryptoPP::byte buff[public_key_length];
	CryptoPP::ArraySink as(buff, public_key_length);
	pubKey.Save(as);
	memcpy(m_PublicKey, &buff, public_key_length);

	for (int i = 0; i < username_length; i++)
		m_Name[i] = 0;
	strcpy(m_Name, &name);
}

void request::RegisterPayload::Serialize(char* buff)
{
	memcpy(buff, m_Name, username_length);
	char* temp = buff + username_length;
	memcpy(temp, m_PublicKey, public_key_length);
}

uint32_t request::RegisterPayload::GetPayloadSize()
{
	return username_length + public_key_length;
}

request::PublicKeyPayload::PublicKeyPayload(const char& targetId)
{
	memcpy(m_ClientId, &targetId, id_length);
}

void request::PublicKeyPayload::Serialize(char* buff)
{
	memcpy(buff, m_ClientId, id_length);
}

uint32_t request::PublicKeyPayload::GetPayloadSize()
{
	return id_length;
}


request::MessagePayload::MessagePayload(const char& targetId, uint8_t messageType, std::string messageContent) : m_MessageType(messageType), m_ContentSize(messageContent.length())
{
	memcpy(m_ClientId, &targetId, id_length);

	m_MessageContent = new char[m_ContentSize];

	for (uint32_t i = 0; i < m_ContentSize; i++)
		m_MessageContent[i] = 0;

	memcpy(m_MessageContent, messageContent.c_str(), m_ContentSize);
}

request::MessagePayload::~MessagePayload()
{
	delete m_MessageContent;
}

void request::MessagePayload::Serialize(char* buff)
{
	memcpy(buff, m_ClientId, id_length);
	char* temp = buff + id_length;
	memcpy(temp, &m_ContentSize, sizeof(m_ContentSize));
	temp += sizeof(m_ContentSize);
	memcpy(temp, &m_MessageType, sizeof(m_MessageType));
	temp += sizeof(m_MessageType);
	memcpy(temp, m_MessageContent, m_ContentSize);
}

uint32_t request::MessagePayload::GetPayloadSize()
{
	return id_length + sizeof(m_MessageType) + sizeof(m_ContentSize) + m_ContentSize;
}

response::RegisterPayload::RegisterPayload(tcp::socket* s)
{
	boost::asio::read(*s, boost::asio::buffer(&m_ClientId, id_length));
}

response::ClientListPayload::ClientListPayload(tcp::socket* s)
{
	m_Username[username_length] = 0;
	boost::asio::read(*s, boost::asio::buffer(&m_ClientId, id_length));
	boost::asio::read(*s, boost::asio::buffer(&m_Username, username_length));
}

response::PublicKeyPayload::PublicKeyPayload(tcp::socket* s)
{
	boost::asio::read(*s, boost::asio::buffer(&m_ClientId, id_length));
	boost::asio::read(*s, boost::asio::buffer(&m_PublicKey, public_key_length));
}

response::PullPayload::PullPayload(tcp::socket* s)
{
	boost::asio::read(*s, boost::asio::buffer(&m_ClientId, id_length));
	boost::asio::read(*s, boost::asio::buffer(&m_MessageId, sizeof(m_MessageId)));
	boost::asio::read(*s, boost::asio::buffer(&m_MessageType, sizeof(m_MessageType)));
	boost::asio::read(*s, boost::asio::buffer(&m_MessageSize, sizeof(m_MessageSize)));

	m_Content = new unsigned char[m_MessageSize];
	boost::asio::read(*s, boost::asio::buffer(m_Content, m_MessageSize));
}

response::PullPayload::~PullPayload()
{
	delete m_Content;
}

std::string response::PullPayload::GetContentAsString()
{
	return std::string(reinterpret_cast<char*>(m_Content), m_MessageSize);
}