import psycopg2


QUERY = '''
SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s);
'''


VARIABLES = {'city': 'dr', 'min_price': None, 'max_price': None}


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
