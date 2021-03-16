import logging
import struct

SERVER_VERSION = 2
REGISTER_RESPONSE_CODE = 1000
CLIENT_LIST_RESPONSE_CODE = 1001
PUBLIC_KEY_RESPONSE_CODE = 1002
MESSAGE_RESPONSE_CODE = 1003
PULL_RESPONSE_CODE = 1004
ERROR_RESPONSE_CODE = 9000
MESSAGE_ID_LENGTH = 4
CLIENT_ID_LENGTH = 16


def register_response(conn, client):
    """
    Sending a response after client register
    :param conn: client connection
    :param client: registered client
    """

    s = struct.Struct('<BHI16s')
    s = s.pack(SERVER_VERSION, REGISTER_RESPONSE_CODE, CLIENT_ID_LENGTH, client.cid)
    conn.sendall(s)
    logging.info(f'Register response ({REGISTER_RESPONSE_CODE}) has been sent.')


def client_list_response(conn, response):
    """
    Sending a client list response
    :param conn: client connection
    :param response: client list to send
    """

    s = struct.Struct('<BHI' + '16s255s' * response['number_of_clients'])
    s = s.pack(SERVER_VERSION, CLIENT_LIST_RESPONSE_CODE, response['payload_size'], *response['payload'])
    conn.sendall(s)
    logging.info(f'Client list response ({CLIENT_LIST_RESPONSE_CODE}) has been sent.')


def public_key_response(conn, client_id, public_key):
    """
    Sending a public key response
    :param conn: client connection
    :param client_id: ID of the target client
    :param public_key: public key of the target client
    """

    payload_size = len(client_id) + len(public_key)
    s = struct.Struct('<BHI16s160s')
    s = s.pack(SERVER_VERSION, PUBLIC_KEY_RESPONSE_CODE, payload_size, client_id, public_key)
    conn.sendall(s)
    logging.info(f'Public key response ({PUBLIC_KEY_RESPONSE_CODE}) has been sent.')


def message_response(conn, client_id, message_id):
    """
    Sending response for message request
    :param conn: client connection
    :param client_id: ID of the target client
    :param message_id: ID of the sent message
    """

    s = struct.Struct('<BHI16sI')
    s = s.pack(SERVER_VERSION, MESSAGE_RESPONSE_CODE, CLIENT_ID_LENGTH + MESSAGE_ID_LENGTH, client_id, message_id)
    conn.sendall(s)
    logging.info(f'Message response ({MESSAGE_RESPONSE_CODE}) has been sent.')


def pull_response(conn, payload, payload_size):
    """
    Sending a response for pull request
    :param conn: client connection
    :param payload: messages to send back to client
    :param payload_size: size of all messages combined
    """

    s = struct.Struct('<BHI')
    s = s.pack(SERVER_VERSION, PULL_RESPONSE_CODE, payload_size)
    conn.sendall(s)
    for p in payload:
        conn.sendall(p)
    logging.info(f'Pull response ({PULL_RESPONSE_CODE}) has been sent.')


def error_response(conn, message):
    """
    Sending an error response to the client
    :param conn: client connection
    :param message: error message for logging
    """

    logging.error(message)
    s = struct.Struct('<BHI')
    s = s.pack(SERVER_VERSION, ERROR_RESPONSE_CODE, 0)
    conn.sendall(s)
    logging.info(f'Error response ({ERROR_RESPONSE_CODE}) has been sent.')
