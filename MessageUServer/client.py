import datetime
import uuid


class Client:
    """
    A client entity.
    """

    def __init__(self, username, public_key):
        self.cid = None
        self.username = username
        self.public_key = public_key
        self.last_seen = None

    def __repr__(self) -> str:
        return f'{{ID: {uuid.UUID(bytes=self.cid)}, Name: {self.username}, ' \
               f'Public Key: {self.public_key}, Last Seen: {self.last_seen}}}'

    def update_last_seen(self):
        self.last_seen = datetime.datetime.now().strftime('%d/%m/%Y %H:%M:%S')

    def create_uuid(self):
        self.cid = uuid.uuid4().bytes


class ClientDAO:
    """
    An interface for database operations on clients.
    """

    def __init__(self):
        pass

    def insert(self, client):
        pass

    def delete_by_id(self, cid):
        pass

    def delete_by_name(self, username):
        pass

    def update_name(self, client, new_name):
        pass

    def update_public_key(self, client, new_key):
        pass

    def update_last_seen(self, client):
        pass

    def find_by_name(self, username):
        pass

    def find_by_id(self, cid):
        pass

    def find_all(self):
        pass
