DROP TABLE IF EXISTS Clients;
DROP TABLE IF EXISTS Messages;

CREATE TABLE Clients
(
    id         VARCHAR(16) PRIMARY KEY,
    username   VARCHAR(255) UNIQUE,
    public_key BLOB,
    last_seen  VARCHAR(20)
);

CREATE TABLE Messages
(
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    to_client   INTEGER NOT NULL,
    from_client INTEGER NOT NULL,
    type        INT(1),
    content     BLOB,
    FOREIGN KEY (to_client) REFERENCES Clients (id),
    FOREIGN KEY (from_client) REFERENCES Clients (id)
);

