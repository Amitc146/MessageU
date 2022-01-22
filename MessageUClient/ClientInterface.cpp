#include "ClientInterface.h"
#include <iostream>
#include <fstream>


ClientInterface::ClientInterface()
{
	FetchServerDetails();
	InitClient();
}

ClientInterface::~ClientInterface()
{
	if (m_Socket)
	{
		m_Socket->close();
		delete m_Socket;
	}
	delete m_Client;
}


void ClientInterface::FetchServerDetails()
{
	std::ifstream file("server.info");

	if (!file)
	{
		std::cout << "File 'server.info' is inaccessible." << std::endl;
		exit(1);
	}

	std::string line, temp;
	std::getline(file, line);
	std::stringstream lineStream(line);

	std::getline(lineStream, temp, ':');
	m_Host = temp;
	std::getline(lineStream, temp);
	m_Port = temp;

	file.close();
}


void ClientInterface::InitClient()
{
	std::ifstream file("me.info");

	if (!file)
	{
		m_Registered = false;
		m_Client = new Client();
	}
	else
	{
		m_Registered = true;
		std::string username, id, privateKey, line;
		std::getline(file, username);
		std::getline(file, id);

		while (std::getline(file, line))
			privateKey += line;

		std::getline(file, privateKey);

		m_Client = new Client(username, id, privateKey);
	}

	file.close();
}


void ClientInterface::RunClient()
{
	boost::asio::io_context io_context;
	m_Socket = new tcp::socket(io_context);
	tcp::resolver resolver(io_context);

	try
	{
		boost::asio::connect(*m_Socket, resolver.resolve(m_Host, m_Port));
	}
	catch (const std::exception&)
	{
		std::cout << "Failed to connect to server." << std::endl;
		exit(1);
	}

	m_Client->SetSocket(m_Socket);

	try
	{
		while (true)
		{
			PrintUserInterface();
			int input = GetCommandInput();

			if (input == exit_client)
			{
				std::cout << "\nGoobye." << std::endl;
				break;
			}
			else if (input == register_request)
			{
				SendRegisterRequest();
			}
			else if (m_Registered)
			{
				// Refreshing client list for future requests
				m_Client->ClientListRequest();

				if (input == client_list_request)
					SendClientListRequest();
				else if (input == public_key_request)
					SendPublicKeyRequest();
				else if (input == pull_request)
					SendPullRequest();
				else if (input == send_text_message_request)
					SendMessageRequest();
				else if (input == get_symmetric_key_request)
					SendGetSymmetricKeyRequest();
				else if (input == send_symmetric_key_request)
					SendSendSymmetricKeyRequest();
			}
			else
			{
				std::cout << "\nYou are not registered. Please register first and try again." << std::endl;
			}
		}

	}
	catch (const std::exception&)
	{
		std::cout << "Connection lost." << std::endl;
		exit(1);
	}


}


void ClientInterface::SendRegisterRequest()
{
	if (!m_Registered)
	{
		m_Client->SetUsername(GetUsernameInput());

		if (m_Client->RegisterRequest())
		{
			SaveUserDetails();
			m_Registered = true;
		}

		// Refreshing client list for future requests
		m_Client->ClientListRequest();
	}
	else
		std::cout << "\nYou're already registered.\n" << std::endl;
}


void ClientInterface::SendClientListRequest()
{
	if (m_Client->ClientListRequest())
		m_Client->PrintUsers();
}


void ClientInterface::SendPublicKeyRequest()
{
	std::string targetUsername = GetUsernameInput();
	m_Client->PublicKeyRequest(targetUsername);
}


void ClientInterface::SendPullRequest()
{
	m_Client->PullRequest();
	m_Client->PrintMessages();
}


void ClientInterface::SendMessageRequest()
{
	std::string targetUsername = GetUsernameInput();
	std::string message = GetMessageInput();
	m_Client->TextMessageRequest(targetUsername, message);
}


void ClientInterface::SendGetSymmetricKeyRequest()
{
	std::string targetUsername = GetUsernameInput();
	m_Client->GetSymmetricKeyRequest(targetUsername);
}


void ClientInterface::SendSendSymmetricKeyRequest()
{
	std::string targetUsername = GetUsernameInput();
	m_Client->SendSymmetricKeyRequest(targetUsername);
}


void ClientInterface::PrintUserInterface()
{
	std::cout << "\nMessageU client at your service.\n" << std::endl;
	std::cout << "1) Register" << std::endl;
	std::cout << "2) Request for clients list" << std::endl;
	std::cout << "3) Request for public key" << std::endl;
	std::cout << "4) Request for waiting messages" << std::endl;
	std::cout << "5) Send a text message" << std::endl;
	std::cout << "51) Send a request for symmetric key" << std::endl;
	std::cout << "52) Send your symmetric key" << std::endl;
	std::cout << "0) Exit client\n" << std::endl;
}


int ClientInterface::GetCommandInput()
{
	std::string input;
	int inputAsInt;

	while (true)
	{
		std::getline(std::cin, input);

		while (std::cin.fail() || std::cin.eof() || input.find_first_not_of("0123456789") != std::string::npos || input.empty()) {

			std::cout << "Invalid command." << std::endl;

			if (input.find_first_not_of("0123456789") == std::string::npos)
				std::cin.clear();

			std::getline(std::cin, input);
		}

		std::string::size_type st;
		inputAsInt = std::stoi(input, &st);

		if (IsValidCommand(inputAsInt))
			break;

		std::cout << "Invalid command." << std::endl;
	}

	return inputAsInt;
}


bool ClientInterface::IsValidCommand(int input)
{
	return ((input >= 0 && input < 6) || (input > 50 && input < 53));
}


std::string ClientInterface::GetUsernameInput()
{
	std::cout << "Enter username: ";

	std::string input;
	std::getline(std::cin, input);

	while (std::cin.fail() || std::cin.eof() || !IsValidUsername(input)) {
		std::cout << "Invalid input. Username must be an alphanumeric value." << std::endl;
		std::cout << "Enter username: ";
		std::getline(std::cin, input);
	}

	return input;
}


bool ClientInterface::IsValidUsername(const std::string& str)
{
	return (str.length() < 255) && find_if(str.begin(), str.end(),
		[](char c) { return !(isalnum(c) || (c == ' ')); }) == str.end();
}


std::string ClientInterface::GetMessageInput()
{
	std::cout << "Enter message: ";

	std::string input;
	std::getline(std::cin, input);

	return input;
}


std::string ClientInterface::CreateStringFromPrivateKey(CryptoPP::RSA::PrivateKey privateKey)
{
	std::string privKeyStr;

	CryptoPP::Base64Encoder privKeySink(new CryptoPP::StringSink(privKeyStr));
	privateKey.DEREncode(privKeySink);
	privKeySink.MessageEnd();

	return privKeyStr;
}


void ClientInterface::SaveUserDetails()
{
	std::string privKeyStr = CreateStringFromPrivateKey(m_Client->GetPrivateKey());

	std::ofstream file("me.info");
	file << m_Client->GetUsername() << "\n";
	file << m_Client->GetId() << "\n";
	file << privKeyStr << "\n";

	file.close();
}
