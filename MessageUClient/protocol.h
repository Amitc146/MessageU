/***************************************
*									   *
* Networking protocol implementation.  *
*									   *
****************************************/

#pragma once

#include <boost/asio.hpp>
#include <rsa.h>

namespace protocol
{
	using boost::asio::ip::tcp;

	static const int id_length = 16;
	static const int public_key_length = 160;
	static const int username_length = 255;


	static const int client_version = 1;
	static const int header_size = 22;

	struct RequestHeader
	{
		uint8_t m_ClientId[id_length];
		uint8_t m_Version;
		uint8_t m_Code;
		uint32_t m_PayloadSize;

		RequestHeader(const char& clientId, uint8_t code, uint32_t payloadSize);
		void Serialize(char* buff);
	};

	struct RequestPayload
	{
		virtual void Serialize(char* buff) = 0;
		virtual uint32_t GetPayloadSize() = 0;
	};

	struct RequestRegisterPayload : public RequestPayload
	{
		char m_Name[username_length];
		CryptoPP::byte m_PublicKey[public_key_length];

		RequestRegisterPayload(const char& name, CryptoPP::RSA::PublicKey pubKey);

		virtual void Serialize(char* buff) override;
		virtual uint32_t GetPayloadSize() override;
	};

	struct RequestPublicKeyPayload : public RequestPayload
	{
		uint8_t m_ClientId[id_length];

		RequestPublicKeyPayload(const char& targetId);

		virtual void Serialize(char* buff) override;
		virtual uint32_t GetPayloadSize() override;
	};

	struct RequestMessagePayload : public RequestPayload
	{
		uint8_t m_ClientId[id_length];
		uint8_t m_MessageType;
		uint32_t m_ContentSize;
		char* m_MessageContent;

		RequestMessagePayload(const char& targetId, uint8_t messageType, std::string messageContent);
		~RequestMessagePayload();

		virtual void Serialize(char* buff) override;
		virtual uint32_t GetPayloadSize() override;
	};



	struct ResponseRegisterPayload
	{
		uint8_t m_ClientId[id_length];

		ResponseRegisterPayload(tcp::socket* s);
	};

	struct ResponseClientListPayload
	{
		uint8_t m_ClientId[id_length];
		char m_Username[username_length + 1];

		ResponseClientListPayload(tcp::socket* s);
	};

	struct ResponsePublicKeyPayload
	{
		uint8_t m_ClientId[id_length];
		CryptoPP::byte m_PublicKey[public_key_length];

		ResponsePublicKeyPayload(tcp::socket* s);
	};

	struct ResponsePullPayload
	{
		uint8_t m_ClientId[id_length];
		uint32_t m_MessageId;
		uint8_t m_MessageType;
		uint32_t m_MessageSize;
		unsigned char* m_Content;

		ResponsePullPayload(tcp::socket* s);
		~ResponsePullPayload();
		std::string GetContentAsString();
	};

};

