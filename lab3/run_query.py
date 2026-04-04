import psycopg2


QUERY = '''
SELECT user_id, verify_secret
FROM credentials 
WHERE key = 'user_buzz_31fef485-65a9-4f3f-bc90-65927279f1c9'
LIMIT 1;
'''


def _explain_query(conn, query: str) -> str:
    with conn.cursor() as cursor:
        cursor.execute(f'EXPLAIN {query}')
        return '\n'.join(line for (line,) in cursor.fetchall())


def _analyze_query(conn, query: str) -> str:
    with conn.cursor() as cursor:
        cursor.execute(f'EXPLAIN ANALYZE {query}')
        return '\n'.join(line for (line,) in cursor.fetchall())


with psycopg2.connect('dbname=postgres user=postgres password=postgres host=localhost port=5433') as conn:
    print('Query:')
    print(QUERY)
    print()

    print('Explain:')
    print(_explain_query(conn, QUERY))
    print()

    print('Analyze:')
    print(_analyze_query(conn, QUERY))
    print()
