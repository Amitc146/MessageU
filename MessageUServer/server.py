import socket
import selectors
import sql
from server_requests import *
import logging

REGISTER_REQUEST_CODE = 100
CLIENT_LIST_REQUEST_CODE = 101
PUBLIC_KEY_REQUEST_CODE = 102
MESSAGE_REQUEST_CODE = 103
PULL_REQUEST_CODE = 104

# Logging configuration
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - [%(levelname)s] - %(message)s',
                    datefmt='%d-%b-%y %H:%M:%S')


class Server:

    def __init__(self, host, port_file, db_name) -> None:
        self.host = host
        self.port = None
        if not sql.is_db_exists(db_name):
            sql.create_db(db_name)
        self.selector = selectors.DefaultSelector()
        self.get_port(port_file)
        self.start_server()

    def get_port(self, port_file):
        with open(port_file, 'r') as f:
            self.port = int(f.read(5))

    def start_server(self):
        """
        Start the server and waiting for clients.
        """

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((self.host, self.port))
            s.listen()
            print('listening on', (self.host, self.port))
            s.setblocking(False)
            self.selector.register(s, selectors.EVENT_READ, self.accept)

            while True:
                events = self.selector.select()
                for key, mask in events:
                    callback = key.data
                    callback(key.fileobj, mask)

    def accept(self, sock, mask):
        """
        Accepting and registering a new client connection.
        :param sock: client socket
        :param mask: connection state
        """

        conn, addr = sock.accept()
        print(f'\nAccepted {conn}\n')
        conn.setblocking(False)
        self.selector.register(conn, selectors.EVENT_READ, self.read)

    def close_connection(self, conn):
        print(f'\nClosing {conn}\n')
        self.selector.unregister(conn)
        conn.close()

    def read(self, conn, mask):
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
            self.close_connection(conn)



