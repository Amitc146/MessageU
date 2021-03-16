# Before running the server for the first time, you must initialize the database
# by executing the 'script.sql' script file. You can either run it yourself
# with sqlite3, or you can 'import sql' and run 'sql.create_db('server.db')'.

# import sql
import server

if __name__ == '__main__':
    # sql.create_db('server.db')  # Reseting database. Use only when running the server for the first time.
    server.start_server('localhost')
