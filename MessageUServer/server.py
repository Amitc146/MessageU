import socket
import selectors
from server_requests import *
import logging

REGISTER_REQUEST_CODE = 100
CLIENT_LIST_REQUEST_CODE = 101
PUBLIC_KEY_REQUEST_CODE = 102
MESSAGE_REQUEST_CODE = 103
PULL_REQUEST_CODE = 104

sel = selectors.DefaultSelector()

# Logging configuration
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - [%(levelname)s] - %(message)s',
                    datefmt='%d-%b-%y %H:%M:%S')


def get_port(file):
    with open(file, 'r') as f:
        return int(f.read(5))


def start_server(host):
    """
    Start the server and waiting for clients.
    :param host: ip address
    """

    port = get_port('port.info')
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen()
        print('listening on', (host, port))
        s.setblocking(False)
        sel.register(s, selectors.EVENT_READ, accept)

        while True:
            events = sel.select()
            for key, mask in events:
                callback = key.data
                callback(key.fileobj, mask)


def accept(sock, mask):
    """
    Accepting and registering a new client connection.
    :param sock: client socket
    :param mask: connection state
    """

    conn, addr = sock.accept()
    print(f'\nAccepted {conn}\n')
    conn.setblocking(False)
    sel.register(conn, selectors.EVENT_READ, read)


def read(conn, mask):
    """
    Read and process request for a client.
    :param conn: client connection
    :param mask: connection state
    """

    try:
        header = fetch_header(conn)
        client = client_service.find_by_id(header['client_id'])

        if client:
            client_service.update_last_seen(client)

        if header['code'] == REGISTER_REQUEST_CODE:
            logging.info(f'Client register request ({REGISTER_REQUEST_CODE}) received.')
            register_request(conn)

        if header['code'] == CLIENT_LIST_REQUEST_CODE:
            logging.info(f'Client list request ({CLIENT_LIST_REQUEST_CODE}) received from \'{client.username}\'.')
            client_list_request(conn, header)

        if header['code'] == PUBLIC_KEY_REQUEST_CODE:
            logging.info(f'Public key request ({PUBLIC_KEY_REQUEST_CODE}) received from \'{client.username}\'.')
            public_key_request(conn)

        if header['code'] == MESSAGE_REQUEST_CODE:
            logging.info(f'Message request ({MESSAGE_REQUEST_CODE}) received from \'{client.username}\'.')
            message_request(conn, header)

        if header['code'] == PULL_REQUEST_CODE:
            logging.info(f'Pull request ({PULL_REQUEST_CODE}) received from \'{client.username}\'.')
            pull_request(conn, header)

    except ConnectionResetError:
        close_connection(conn)


def close_connection(conn):
    print(f'\nClosing {conn}\n')
    sel.unregister(conn)
    conn.close()
