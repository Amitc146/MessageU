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

	namespace request
	{
		static const int client_version = 1;
		static const int header_size = 22;

		struct Header
		{
			uint8_t m_ClientId[id_length];
			uint8_t m_Version;
			uint8_t m_Code;
			uint32_t m_PayloadSize;

			request::Header(const char& clientId, uint8_t code, uint32_t payloadSize);
			void Serialize(char* buff);
		};

		struct Payload
		{
			virtual void Serialize(char* buff) = 0;
			virtual uint32_t GetPayloadSize() = 0;
		};

		struct RegisterPayload : public request::Payload
		{
			char m_Name[username_length];
			CryptoPP::byte m_PublicKey[public_key_length];

			RegisterPayload(const char& name, CryptoPP::RSA::PublicKey pubKey);

			virtual void Serialize(char* buff) override;
			virtual uint32_t GetPayloadSize() override;
		};

		struct PublicKeyPayload : public request::Payload
		{
			uint8_t m_ClientId[id_length];

			PublicKeyPayload(const char& targetId);

			virtual void Serialize(char* buff) override;
			virtual uint32_t GetPayloadSize() override;
		};

		struct MessagePayload : public request::Payload
		{
			uint8_t m_ClientId[id_length];
			uint8_t m_MessageType;
			uint32_t m_ContentSize;
			char* m_MessageContent;

			MessagePayload(const char& targetId, uint8_t messageType, std::string messageContent);
			~MessagePayload();

			virtual void Serialize(char* buff) override;
			virtual uint32_t GetPayloadSize() override;
		};
	};


	namespace response
	{
		struct RegisterPayload
		{
			uint8_t m_ClientId[id_length];

			RegisterPayload(tcp::socket* s);
		};

		struct ClientListPayload
		{
			uint8_t m_ClientId[id_length];
			char m_Username[username_length + 1];

			ClientListPayload(tcp::socket* s);
		};

		struct PublicKeyPayload
		{
			uint8_t m_ClientId[id_length];
			CryptoPP::byte m_PublicKey[public_key_length];

			PublicKeyPayload(tcp::socket* s);
		};

		struct PullPayload
		{
			uint8_t m_ClientId[id_length];
			uint32_t m_MessageId;
			uint8_t m_MessageType;
			uint32_t m_MessageSize;
			unsigned char* m_Content;

			PullPayload(tcp::socket* s);
			~PullPayload();
			std::string GetContentAsString();
		};

	};
};

