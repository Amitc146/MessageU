class Message:
    """
    A message entity.
    """

    def __init__(self, to_client, from_client, message_type, content):
        self.mid = None
        self.to_client = to_client
        self.from_client = from_client
        self.message_type = message_type
        self.content = content

    def __repr__(self) -> str:
        return f'{{ID: {self.mid}, To: {self.to_client.username}, From: {self.from_client.username}, ' \
               f'Type: {self.message_type}, Content: {self.content}}}'


