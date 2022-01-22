import sqlite3
from message import *


class MessageDAOSQLite:
    """
    Message DAO using SQLite3.
    """

    def __init__(self, db_path, client_service):
        super().__init__()
        self.db_path = db_path
        self.client_service = client_service

    def insert(self, message):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('INSERT INTO Messages (to_client, from_client, type, content) VALUES (?,?,?,?)',
                           [message.to_client.cid, message.from_client.cid, message.message_type, message.content])
            db.commit()

    def convert_to_message(self, message_tuple):
        message = Message(message_tuple[1], message_tuple[2], message_tuple[3], message_tuple[4])
        message.mid = message_tuple[0]
        message.to_client = self.client_service.find_by_id(message.to_client)
        message.from_client = self.client_service.find_by_id(message.from_client)
        return message

    def delete_by_id(self, mid):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('DELETE FROM Messages WHERE id = ?', [mid])
            db.commit()

    def delete_all(self):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('DELETE FROM Messages')
            db.commit()

    def find_by_id(self, mid):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('SELECT * FROM Messages WHERE id = ?', [mid])
            message = self.convert_to_message(cursor.fetchone())
            return message

    def find_all(self):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('SELECT * FROM Messages')
            messages = []
            for result in cursor:
                messages.append(self.convert_to_message(result))
            return messages

    def find_by_dst_client(self, client):
        with sqlite3.connect(self.db_path) as db:
            cursor = db.cursor()
            cursor.execute('SELECT * FROM Messages WHERE to_client = ?', [client.cid])
            messages = []
            for result in cursor:
                messages.append(self.convert_to_message(result))
            return messages
