import psycopg2


QUERY = '''
SELECT
    p.id,
    p.status,
    p.price,
    a.country as "address.country",
    a.city as "address.city",
    a.street as "address.street",
    a.building as "address.building",
    a.apartment as "address.apartment"
FROM properties as p
JOIN ADDRESSES a ON a.id = p.address_id
WHERE p.owner_id = '31fef485-65a9-4f3f-bc90-65927279f1c9';
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
