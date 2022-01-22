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

