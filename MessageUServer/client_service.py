class ClientService:
    """
    API for client entity.
    To use this class, provide ClientDAO implementation.
    """

    def __init__(self, client_dao):
        self.client_dao = client_dao

    def insert(self, client):
        self.client_dao.insert(client)

    def delete_by_id(self, cid):
        self.client_dao.delete_by_id(cid)

    def delete_by_name(self, name):
        self.client_dao.delete_by_name(name)

    def update_name(self, client, new_name):
        self.client_dao.update_name(client, new_name)

    def update_public_key(self, client, new_key):
        self.client_dao.update_public_key(client, new_key)

    def update_last_seen(self, client):
        self.client_dao.update_last_seen(client)

    def find_by_name(self, name):
        return self.client_dao.find_by_name(name)

    def find_by_id(self, cid):
        return self.client_dao.find_by_id(cid)

    def find_all(self):
        return self.client_dao.find_all()
