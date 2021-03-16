class MessageService:
    """
    API for message entity.
    To use this class, provide MessageDAO implementation.
    """

    def __init__(self, message_dao):
        self.message_dao = message_dao

    def insert(self, message):
        self.message_dao.insert(message)

    def delete_by_id(self, mid):
        self.message_dao.delete_by_id(mid)

    def delete_all(self):
        self.message_dao.delete_all()

    def find_by_id(self, mid):
        return self.message_dao.find_by_id(mid)

    def find_all(self):
        return self.message_dao.find_all()

    def find_by_dst_client(self, client):
        return self.message_dao.find_by_dst_client(client)
