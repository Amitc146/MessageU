#include <iostream>
#include <modes.h>
#include <immintrin.h>
#include <osrng.h>
#include "Client.h"
#include "uuid.h"

Client::Client() : m_Socket(nullptr) {}

Client::Client(const std::string& username, const std::string& id, const std::string& privKeyStr) : m_Username(username), m_Id(id), m_Socket(nullptr)
{
	// Loading private key for future use
	CryptoPP::ByteQueue bytes;
	CryptoPP::StringSource str(privKeyStr, true, new CryptoPP::Base64Decoder);
	str.TransferTo(bytes);
	bytes.MessageEnd();
	m_PrivateKey.Load(bytes);
}

Client::~Client()
{
	for (auto const& u : m_Users)
		delete u.second;
}


bool Client::RegisterRequest()
{
	m_PrivateKey = GeneratePrivateKey();

	std::shared_ptr<RequestPayload> p(
		new RequestRegisterPayload(*(m_Username.c_str()), GeneratePublicKey()));
	std::shared_ptr<RequestHeader> h(
		new RequestHeader(*(m_Username.c_str()), register_request_code, p->GetPayloadSize()));

	SendRequest(h.get(), p.get());
	return GetResponse();
}

CryptoPP::RSA::PrivateKey Client::GeneratePrivateKey()
{
	CryptoPP::AutoSeededRandomPool rng;
	CryptoPP::InvertibleRSAFunction privKey;
	privKey.Initialize(rng, private_key_bit_length);

	std::string privKeyStr;

	CryptoPP::Base64Encoder privKeySink(new CryptoPP::StringSink(privKeyStr));
	privKey.DEREncode(privKeySink);
	privKeySink.MessageEnd();

	CryptoPP::ByteQueue bytes;
	CryptoPP::StringSource str(privKeyStr, true, new CryptoPP::Base64Decoder);
	str.TransferTo(bytes);
	bytes.MessageEnd();
	CryptoPP::RSA::PrivateKey privateKey;
	privateKey.Load(bytes);

	return privateKey;
}

CryptoPP::RSA::PublicKey Client::GeneratePublicKey()
{
	std::string pubKey;

	CryptoPP::RSAFunction pubkey(m_PrivateKey);
	CryptoPP::Base64Encoder pubkeysink(new CryptoPP::StringSink(pubKey));
	pubkey.DEREncode(pubkeysink);
	pubkeysink.MessageEnd();

	CryptoPP::ByteQueue bytes;
	CryptoPP::StringSource str(pubKey, true, new CryptoPP::Base64Decoder);
	str.TransferTo(bytes);
	bytes.MessageEnd();
	CryptoPP::RSA::PublicKey publicKey;
	publicKey.Load(bytes);

	return publicKey;
}


bool Client::ClientListRequest()
{
	std::vector<char> clientId = hex_to_bytes(m_Id);

	std::shared_ptr<RequestHeader> h(
		new RequestHeader(*clientId.data(), client_list_request_code, 0));

	SendRequest(h.get(), nullptr);
	return GetResponse();
}


bool Client::PublicKeyRequest(const std::string& targetUsername)
{
	std::string id = GetIdByUsername(targetUsername);

	if (id.empty())
		return false;

	std::vector<char> clientId = hex_to_bytes(m_Id);
	std::vector<char> targetId = hex_to_bytes(id);

	std::shared_ptr<RequestPayload> p(
		new RequestPublicKeyPayload(*targetId.data()));
	std::shared_ptr<RequestHeader> h(
		new RequestHeader(*clientId.data(), public_key_request_code, p->GetPayloadSize()));

	SendRequest(h.get(), p.get());
	return GetResponse();
}


bool Client::TextMessageRequest(const std::string& targetUsername, const std::string& messageContent)
{
	std::string targetId = GetIdByUsername(targetUsername);

	if (targetId.empty())
		return false;

	CryptoPP::byte* symmKey = m_Users.at(targetId)->GetSymmetricKey();

	if (!symmKey)
		CreateSymmetricKeyForClient(targetId);

	symmKey = m_Users.at(targetId)->GetSymmetricKey();

	// Encrypting with target's symmetric key
	std::string encMessage = EncryptWithSymmetricKey(messageContent, symmKey);

	return SendMessageRequest(targetId, text_message, encMessage);
}


bool Client::GetSymmetricKeyRequest(const std::string& targetUsername)
{
	std::string targetId = GetIdByUsername(targetUsername);

	if (targetId.empty())
		return false;

	CryptoPP::RSA::PublicKey* pubKey = m_Users.at(targetId)->GetPublicKey();
	if (pubKey)
	{
		// Encrypting the message with the target's public key
		std::string encMessage = EncryptWithPublicKey("Request for symmetric key", *pubKey);
		return SendMessageRequest(targetId, get_symmetric_key, encMessage);
	}

	std::cout << "*** ERROR: You need to get the user's public key first. ***" << std::endl;
	return false;
}


bool Client::SendSymmetricKeyRequest(const std::string& targetUsername)
{
	std::string targetId = GetIdByUsername(targetUsername);

	if (targetId.empty())
		return false;

	CryptoPP::RSA::PublicKey* pubKey = m_Users.at(targetId)->GetPublicKey();
	if (pubKey)
	{
		CryptoPP::byte* symmKey = m_Users.at(targetId)->GetSymmetricKey();

		if (!symmKey)
			CreateSymmetricKeyForClient(targetId);

		symmKey = m_Users.at(targetId)->GetSymmetricKey();

		// Encrypting symmetric key with target's public key
		std::string encMessage = EncryptWithPublicKey(reinterpret_cast<char*>(symmKey), *pubKey);

		return SendMessageRequest(targetId, send_symmetric_key, encMessage);
	}

	std::cout << "*** ERROR: You need to get the user's public key first. ***" << std::endl;
	return false;
}


bool Client::SendMessageRequest(const std::string& targetId, uint8_t messageType, const std::string& content)
{
	std::vector<char> b_ClientId = hex_to_bytes(m_Id);
	std::vector<char> b_TargetId = hex_to_bytes(targetId);

	std::shared_ptr<RequestPayload> p(
		new RequestMessagePayload(*b_TargetId.data(), messageType, content));
	std::shared_ptr<RequestHeader> h(
		new RequestHeader(*b_ClientId.data(), message_request_code, p->GetPayloadSize()));

	SendRequest(h.get(), p.get());
	return GetResponse();
}


void Client::CreateSymmetricKeyForClient(const std::string& clientId)
{
	CryptoPP::byte symmKey[CryptoPP::AES::DEFAULT_KEYLENGTH];
	memset(symmKey, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
	GenerateSymmetricKey(reinterpret_cast<char*>(symmKey), CryptoPP::AES::DEFAULT_KEYLENGTH);
	m_Users.at(clientId)->SetSymmetricKey(symmKey);
}

void Client::GenerateSymmetricKey(char* buff, size_t size)
{
	for (size_t i = 0; i < size; i += 4)
		_rdrand32_step(reinterpret_cast<unsigned int*>(&buff[i]));
}

std::string Client::EncryptWithPublicKey(const std::string& message, CryptoPP::RSA::PublicKey pubKey)
{
	CryptoPP::AutoSeededRandomPool rng;

	std::string encrypted;
	CryptoPP::RSAES_OAEP_SHA_Encryptor e(pubKey);
	CryptoPP::StringSource ss(message, true, new CryptoPP::PK_EncryptorFilter(rng, e, new CryptoPP::StringSink(encrypted)));

	return encrypted;
}

std::string Client::EncryptWithSymmetricKey(const std::string& message, CryptoPP::byte* symmKey)
{
	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
	memset(iv, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);

	std::string encMessage;

	CryptoPP::AES::Encryption aesEncryption(symmKey, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(encMessage));
	stfEncryptor.Put(reinterpret_cast<const unsigned char*>(message.c_str()), message.length());
	stfEncryptor.MessageEnd();

	return encMessage;
}

std::string Client::DecryptWithPrivateKey(const std::string& message)
{
	CryptoPP::AutoSeededRandomPool rng;

	std::string decrypted;
	CryptoPP::RSAES_OAEP_SHA_Decryptor d(m_PrivateKey);
	CryptoPP::StringSource ss(message, true, new CryptoPP::PK_DecryptorFilter(rng, d, new CryptoPP::StringSink(decrypted)));

	return decrypted;
}

std::string Client::DecryptWithSymmetricKey(const std::string& message, CryptoPP::byte* symmKey)
{
	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
	memset(iv, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);

	std::string decrypted;

	CryptoPP::AES::Decryption aesDecryption(symmKey, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

	CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decrypted));
	stfDecryptor.Put(reinterpret_cast<const unsigned char*>(message.c_str()), message.size());
	stfDecryptor.MessageEnd();

	return decrypted;
}


bool Client::PullRequest()
{
	std::vector<char> clientId = hex_to_bytes(m_Id);

	std::shared_ptr<RequestHeader> h(new RequestHeader(*clientId.data(), pull_request_code, 0));

	SendRequest(h.get(), nullptr);
	return GetResponse();
}


void Client::SendRequest(RequestHeader* header, RequestPayload* payload)
{
	int requestSize = header_size + (payload ? payload->GetPayloadSize() : 0);
	std::shared_ptr<char> buffer(new char[requestSize]);

	header->Serialize(buffer.get());
	if (payload)
		payload->Serialize(buffer.get() + header_size);

	boost::asio::write(*m_Socket, boost::asio::buffer(buffer.get(), requestSize));
}


bool Client::GetResponse()
{
	uint8_t version;
	uint16_t code;
	uint32_t payloadSize;

	boost::asio::read(*m_Socket, boost::asio::buffer(&version, sizeof(version)));
	boost::asio::read(*m_Socket, boost::asio::buffer(&code, sizeof(code)));
	boost::asio::read(*m_Socket, boost::asio::buffer(&payloadSize, sizeof(payloadSize)));

	if (code == error_response_code)
	{
		std::cout << "*** Server responded with error code 9000 ***\n" << std::endl;
		return false;
	}
	else if (code == register_response_code)
		RegisterResponse();
	else if (code == client_list_response_code)
		ClientListResponse(payloadSize);
	else if (code == public_key_response_code)
		PublicKeyResponse();
	else if (code == message_response_code)
		MessageResponse();
	else if (code == pull_response_code)
		PullResponse(payloadSize);

	return true;
}


void Client::RegisterResponse()
{
	ResponseRegisterPayload payload = ResponseRegisterPayload(m_Socket);

	std::cout << "Registered successfully." << std::endl;
	std::cout << "Username: " << m_Username << std::endl;
	m_Id = get_uuid_string_from_bytes(payload.m_ClientId);
	std::cout << "ID: " << m_Id << std::endl;
}


void Client::ClientListResponse(uint32_t payloadSize)
{
	uint16_t numberOfClients = payloadSize / (id_length + username_length);

	// Saving the received users in a <username, User> map
	for (int i = 0; i < numberOfClients; i++)
	{
		ResponseClientListPayload payload = ResponseClientListPayload(m_Socket);
		std::string id = get_uuid_string_from_bytes(payload.m_ClientId);	// UUID conversion
		if (!m_Users[id])
			m_Users[id] = new User(id, payload.m_Username);
	}
}


void Client::PublicKeyResponse()
{
	ResponsePublicKeyPayload payload = ResponsePublicKeyPayload(m_Socket);

	std::string id = get_uuid_string_from_bytes(payload.m_ClientId);
	User* u = m_Users.at(id);

	CryptoPP::ArraySource as(payload.m_PublicKey, public_key_length, true);
	CryptoPP::RSA::PublicKey pubKey;
	pubKey.Load(as);
	u->SetPublicKey(pubKey);

	std::cout << "Public key received." << std::endl;
}


void Client::MessageResponse()
{
	uint8_t id[id_length];
	uint32_t mid;

	// Reading the response from the server to flush the socket
	boost::asio::read(*m_Socket, boost::asio::buffer(&id, id_length));
	boost::asio::read(*m_Socket, boost::asio::buffer(&mid, sizeof(mid)));

	std::cout << "Message sent successfully." << std::endl;
}


void Client::PullResponse(uint32_t payloadSize)
{
	while (payloadSize > 0)
	{
		ResponsePullPayload payload = ResponsePullPayload(m_Socket);
		User* user = m_Users[get_uuid_string_from_bytes(payload.m_ClientId)];
		std::pair<std::string, std::string> message;


		// Request for symmetric key
		if (payload.m_MessageType == get_symmetric_key)
		{
			// Decrypting the message using the private key
			message = std::make_pair(user->GetUsername(), DecryptWithPrivateKey(payload.GetContentAsString()));
		}

		// Symmeric key received
		else if (payload.m_MessageType == send_symmetric_key)
		{
			// Adding a generic message for printing
			message = std::make_pair(user->GetUsername(), "Symmetric key received");

			// Decrypting the received symmetric key using the private key
			std::string symmKey = DecryptWithPrivateKey(payload.GetContentAsString());
			char* temp = new char[symmKey.length()];
			std::copy(symmKey.begin(), symmKey.end(), temp);
			user->SetSymmetricKey(reinterpret_cast<CryptoPP::byte*>(temp));
			delete[] temp;

		}

		// Text message received
		else
		{
			// Decrypting the message using source user's symmetric key

			CryptoPP::byte* symmKey = user->GetSymmetricKey();

			if (symmKey)
			{
				std::string decryptedMessage = DecryptWithSymmetricKey(payload.GetContentAsString(), symmKey);
				message = std::make_pair(user->GetUsername(), decryptedMessage);
			}
			else
			{
				// No symmetric key received, can't decrypt.
				message = std::make_pair("", "");
			}
			
		}

		m_Messages.push_back(message);
		payloadSize -= payload.m_MessageSize;
	}
}


void Client::PrintMessages()
{
	if (!m_Messages.empty())
	{
		std::cout << "Messages received: " << std::endl;
		for (auto const& m : m_Messages)
		{
			std::cout << "\n--------------------<Message>--------------------" << std::endl;
			if (m.first.empty() && m.second.empty())
				std::cout << "Can't decrypt message." << std::endl;
			else
			{
				std::cout << "From: '" << m.first << "'" << std::endl;
				std::cout << "Content: " << m.second << std::endl;
			}
			std::cout << "----------------------<EOM>----------------------" << std::endl;
		}
		m_Messages.clear();
	}

	else
		std::cout << "No messages received." << std::endl;
}


void Client::PrintUsers()
{
	std::cout << "Client list:" << std::endl;
	for (auto const& u : m_Users)
		u.second->PrintUser();
	std::cout << std::endl;
}


std::string Client::GetIdByUsername(const std::string& username)
{
	for (auto const& u : m_Users)
	{
		if (u.second->GetUsername() == username)
			return u.second->GetClientId();
	}

	std::cout << "Username not found." << std::endl;
	return "";
}


void Client::SetSocket(tcp::socket* socket)
{
	m_Socket = socket;
}


void Client::SetUsername(const std::string& username)
{
	m_Username = username;
}


void Client::SetId(const std::string& id)
{
	m_Id = id;
}


std::string Client::GetUsername()
{
	return m_Username;
}


std::string Client::GetId()
{
	return m_Id;
}


CryptoPP::RSA::PrivateKey Client::GetPrivateKey()
{
	return m_PrivateKey;
}
