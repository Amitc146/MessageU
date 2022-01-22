import sqlite3
from client import *


class ClientDAOSQLite:
    """
    Client DAO using SQLite3.
    """

    def __init__(self, db_path):
        self.db_path = db_path

    def insert(self, client):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()

            # Creating UUID and updating last seen
            client.create_uuid()
            client.update_last_seen()

            cursor.execute('INSERT INTO Clients VALUES (?,?,?,?)',
                           [client.cid, client.username, client.public_key, client.last_seen])
            db.commit()

    def delete_by_id(self, cid):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('DELETE FROM Clients WHERE id = ?', [cid])
            db.commit()

    def delete_by_name(self, username):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('DELETE FROM Clients WHERE username = ?', [username])
            db.commit()

    def update_name(self, client, new_name):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('UPDATE Clients SET username = ? WHERE id = ?', [new_name, client.cid])
            db.commit()

    def update_public_key(self, client, new_key):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('UPDATE Clients SET public_key = ? WHERE id = ?', [new_key, client.cid])
            db.commit()

    def update_last_seen(self, client):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            client.update_last_seen()
            cursor.execute('UPDATE Clients SET last_seen = ? WHERE id = ?', [client.last_seen, client.cid])
            db.commit()

    def find_by_name(self, username):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('SELECT * FROM Clients WHERE username = ?', [username])
            client = convert_to_client(cursor.fetchone())
            return client

    def find_by_id(self, cid):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('SELECT * FROM Clients WHERE id = ?', [cid])
            client = convert_to_client(cursor.fetchone())
            return client

    def find_all(self):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('SELECT * FROM Clients')
            clients = []
            for result in cursor:
                clients.append(convert_to_client(result))
            return clients


def convert_to_client(client_tuple):
    if client_tuple is not None:
        client = Client(client_tuple[1], client_tuple[2])
        client.cid = client_tuple[0]
        client.last_seen = client_tuple[3]
        return client
