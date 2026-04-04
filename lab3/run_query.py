import psycopg2


QUERY = '''
SELECT id
FROM users
WHERE (%(login)s IS NULL OR login ILIKE '%%' || %(login)s || '%%')
  AND (%(first_name)s IS NULL OR first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (%(last_name)s IS NULL OR last_name ILIKE '%%' || %(last_name)s || '%%');
'''


VARIABLES = {'login': 'user_', 'first_name': None, 'last_name': None}


def _explain_query(conn, query: str) -> str:
    with conn.cursor() as cursor:
        cursor.execute(f'EXPLAIN {QUERY}', VARIABLES)
        return '\n'.join(line for (line,) in cursor.fetchall())


def _analyze_query(conn, query: str) -> str:
    with conn.cursor() as cursor:
        cursor.execute(f'EXPLAIN ANALYZE {query}', VARIABLES)
        return '\n'.join(line for (line,) in cursor.fetchall())


with psycopg2.connect('dbname=postgres user=postgres password=postgres host=localhost port=5433') as conn:
    print('Query:')
    print(QUERY)
    print()

    print('Variables:', VARIABLES)
    print()

    print('Explain:')
    print(_explain_query(conn, QUERY))
    print()

    print('Analyze:')
    print(_analyze_query(conn, QUERY))
    print()
