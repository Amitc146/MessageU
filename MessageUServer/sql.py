import os
from pathlib import Path
import sqlite3


def get_sql_script(file_name):
    if not Path(file_name).is_file():
        raise Exception('File does not exist')
    elif file_name.lower().endswith('.sql'):
        with open(file_name, 'r') as sql_file:
            return sql_file.read()
    raise Exception('File is not in sql format')


def create_db(db_name):
    conn = sqlite3.connect(db_name)
    cur = conn.cursor()
    cur.executescript(get_sql_script('script.sql'))
    conn.close()


def delete_db(db_name):
    if Path(db_name).is_file():
        os.remove(db_name)
