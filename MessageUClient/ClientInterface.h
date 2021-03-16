/**********************************
*  		 						  *
* User interface implementation.  *
*							      *
***********************************/

#pragma once

#include "Client.h"

using boost::asio::ip::tcp;

constexpr int exit_client = 0;
constexpr int register_request = 1;
constexpr int client_list_request = 2;
constexpr int public_key_request = 3;
constexpr int pull_request = 4;
constexpr int send_text_message_request = 5;
constexpr int get_symmetric_key_request = 51;
constexpr int send_symmetric_key_request = 52;

class ClientInterface
{
private:
	Client* m_Client;
	tcp::socket* m_Socket;
	bool m_Registered;
	std::string m_Host;
	std::string m_Port;

public:
	ClientInterface();
	~ClientInterface();
	void RunClient();

private:
	void SendRegisterRequest();
	void SendClientListRequest();
	void SendPublicKeyRequest();
	void SendPullRequest();
	void SendMessageRequest();
	void SendGetSymmetricKeyRequest();
	void SendSendSymmetricKeyRequest();

	void FetchServerDetails();
	void InitClient();
	void SaveUserDetails();
	void PrintUserInterface();
	
	int GetCommandInput();
	std::string GetUsernameInput();
	std::string GetMessageInput();	
	bool IsValidCommand(int input);
	bool IsValidUsername(const std::string& str);
	
	std::string CreateStringFromPrivateKey(CryptoPP::RSA::PrivateKey privateKey);
	void PrintRequestBlock(std::string requestName);
	void EndBlock();
};

