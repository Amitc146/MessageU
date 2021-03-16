/**********************************************
*											  *
* Implementing communication with the server. *
*											  *
***********************************************/

#pragma once

#include <files.h>
#include <base64.h>
#include "User.h"
#include "protocol.h"

using namespace protocol;

constexpr uint8_t register_request_code = 100;
constexpr uint8_t client_list_request_code = 101;
constexpr uint8_t public_key_request_code = 102;
constexpr uint8_t message_request_code = 103;
constexpr uint8_t pull_request_code = 104;
constexpr uint16_t register_response_code = 1000;
constexpr uint16_t client_list_response_code = 1001;
constexpr uint16_t public_key_response_code = 1002;
constexpr uint16_t message_response_code = 1003;
constexpr uint16_t pull_response_code = 1004;
constexpr uint16_t error_response_code = 9000;
constexpr uint8_t get_symmetric_key = 1;
constexpr uint8_t send_symmetric_key = 2;
constexpr uint8_t text_message = 3;
constexpr int private_key_bit_length = 1024;

class Client
{
private:
	tcp::socket* m_Socket;
	std::string m_Username;
	std::string m_Id;
	CryptoPP::RSA::PrivateKey m_PrivateKey;
	std::map<std::string, User*> m_Users;
	std::vector<std::pair<std::string, std::string>> m_Messages;

public:
	Client();
	Client(std::string username, std::string id, std::string privKeyStr);
	~Client();

	bool RegisterRequest();
	bool ClientListRequest();
	bool PublicKeyRequest(std::string targetUsername);
	bool TextMessageRequest(std::string targetUsername, std::string messageContent);
	bool GetSymmetricKeyRequest(std::string targetUsername);
	bool SendSymmetricKeyRequest(std::string targetUsername);
	bool PullRequest();

private:
	void SendRequest(request::Header* header, request::Payload* payload);
	bool GetResponse();
	void RegisterResponse();
	void ClientListResponse(uint32_t payloadSize);
	void PublicKeyResponse();
	void MessageResponse();
	void PullResponse(uint32_t payloadSize);

	bool SendMessageRequest(std::string targetId, uint8_t messageType, std::string content);
	CryptoPP::RSA::PrivateKey GeneratePrivateKey();
	CryptoPP::RSA::PublicKey GeneratePublicKey();
	void CreateSymmetricKeyForClient(std::string clientId);
	void GenerateSymmetricKey(char* buff, size_t size);
	std::string EncryptWithPublicKey(std::string message, CryptoPP::RSA::PublicKey pubKey);
	std::string EncryptWithSymmetricKey(std::string message, CryptoPP::byte* symmKey);
	std::string DecryptWithPrivateKey(std::string message);
	std::string DecryptWithSymmetricKey(std::string message, CryptoPP::byte* symmKey);
	std::string GetIdByUsername(std::string username);

public:
	void PrintUsers();
	void PrintMessages();
	void SetSocket(tcp::socket* socket);
	void SetUsername(std::string username);
	void SetId(std::string id);
	std::string GetUsername();
	std::string GetId();
	CryptoPP::RSA::PrivateKey GetPrivateKey();

};
