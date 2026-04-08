# Оптимизация запросов

## Поиск пользователей

Рассмотрим запрос для поиска пользователей ([ссылка](../api_gateway/postgresql/queries/find_users.sql)).
Для оценки оптимальности выполнения запроса воспользуемся утилитой [`analyze_query.py`](../utils/analyze_query.py)

```shell
uv --project utils \
  run utils/analyze_query.py \
  -v login=user \
  -v first_name=null \
  -v last_name=null \
  api_gateway/postgresql/queries/find_users.sql \
  connection.yaml
```

В итоге, получим следующий вывод:
```
Query:

SELECT id
FROM users
WHERE (%(login)s IS NULL OR login ILIKE '%%' || %(login)s || '%%')
  AND (%(first_name)s IS NULL OR first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (%(last_name)s IS NULL OR last_name ILIKE '%%' || %(last_name)s || '%%');


Variables: {'login': 'user_', 'first_name': None, 'last_name': None}

Analyze:
Seq Scan on users  (cost=0.00..921.15 rows=223 width=16) (actual time=3.757..3.758 rows=0.00 loops=1)
  Filter: ((login)::text ~~* '%user_%'::text)
  Buffers: shared hit=534
Planning Time: 0.031 ms
Execution Time: 3.765 ms
```

Как видим, оценка выполнения запроса показала, что запрос неоптимален. Попробуем добавить индексы,
так как они позволяют ускорить обработку запроса. Заметим, что все поля, по которым мы производим
поиск, являются строковыми, а фильтрацию мы проводим через поиск по подстроке.

Для этих целей лучше всего подходит индекс GIN. Добавим его, добавив следующие запросы в схему:

```sql
CREATE INDEX user_login_search_index ON users USING gin (login gin_trgm_ops);
CREATE INDEX user_first_name_search_index ON users USING gin (first_name gin_trgm_ops);
CREATE INDEX user_last_name_search_index ON users USING gin (last_name gin_trgm_ops);
```

Мы добавили 3 разных индекса, а не один общий, так как поиск происходит по трем полям независимо.
Проведем анализ запроса с учетом добавленного индекса:

```
Query:

SELECT id
FROM users
WHERE (%(login)s IS NULL OR login ILIKE '%%' || %(login)s || '%%')
  AND (%(first_name)s IS NULL OR first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (%(last_name)s IS NULL OR last_name ILIKE '%%' || %(last_name)s || '%%');


Variables: {'login': 'user_', 'first_name': None, 'last_name': None}

Analyze:
Bitmap Heap Scan on users  (cost=324.47..740.59 rows=223 width=16) (actual time=7.423..49.310 rows=40000.00 loops=1)
  Recheck Cond: ((login)::text ~~* '%user_%'::text)
  Heap Blocks: exact=534
  Buffers: shared hit=625
  ->  Bitmap Index Scan on users_login_search_index  (cost=0.00..324.41 rows=223 width=0) (actual time=7.221..7.222 rows=40000.00 loops=1)
        Index Cond: ((login)::text ~~* '%user_%'::text)
        Index Searches: 1
        Buffers: shared hit=91
Planning:
  Buffers: shared hit=1
Planning Time: 0.082 ms
Execution Time: 50.882 ms
```

Как видим, добавление GIN индекса оказало положительное влияние на производительность выполнения запроса.
Попробуем теперь добавить один общий индекс для трех полей.

```sql
CREATE INDEX idx_users_all_trgm 
  ON "users" USING gin (
      "login"        gin_trgm_ops,
      "first_name"   gin_trgm_ops,
      "last_name"    gin_trgm_ops
  );
```

И проанализируем производительность выполнения запроса:

```
Query:
SELECT id
FROM users
WHERE (%(login)s IS NULL OR login ILIKE '%%' || %(login)s || '%%')
  AND (%(first_name)s IS NULL OR first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (%(last_name)s IS NULL OR last_name ILIKE '%%' || %(last_name)s || '%%');

Variables: {'login': 'user_', 'first_name': None, 'last_name': None}

Analyze:
Seq Scan on users  (cost=0.00..921.15 rows=223 width=16) (actual time=2.526..2.526 rows=0.00 loops=1)
  Filter: ((login)::text ~~* '%user_%'::text)
  Buffers: shared hit=534
Planning:
  Buffers: shared hit=1
Planning Time: 0.042 ms
Execution Time: 2.534 ms
```

Как видим использование единого GIN-индекса плохо справляется с составным поиском.
Выгоднее использовать BinaryOR в данном кейсе.

## Поиск недвижимости

Рассмотрим запрос для поиска недвижимости ([ссылка](../api_gateway/postgresql/queries/find_properties.sql)).
Для оценки оптимальности выполнения запроса воспользуемся утилитой [`analyze_query.py`](../utils/analyze_query.py)

```shell
uv --project utils \
  run utils/analyze_query.py \
  -v city=null \
  -v min_price=300 \
  -v max_price=null \
  -v owner_id=null \
  api_gateway/postgresql/queries/find_properties.sql \
  connection.yaml
```

В итоге, получим следующий вывод:

```
Query:
SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s)
  AND (%(owner_id)s IS NULL OR p.owner_id = %(owner_id)s);

Variables: {'city': None, 'min_price': 300, 'max_price': None, 'owner_id': None}

Analyze:
Hash Join  (cost=1296.00..2312.93 rows=39589 width=16) (actual time=15.466..28.483 rows=39576.00 loops=1)
  Hash Cond: (p.address_id = a.id)
  Buffers: shared hit=809
  ->  Seq Scan on properties p  (cost=0.00..913.00 rows=39589 width=32) (actual time=0.008..5.978 rows=39576.00 loops=1)
        Filter: (price >= 300)
        Rows Removed by Filter: 424
        Buffers: shared hit=413
  ->  Hash  (cost=796.00..796.00 rows=40000 width=16) (actual time=14.612..14.613 rows=40000.00 loops=1)
        Buckets: 65536  Batches: 1  Memory Usage: 2387kB
        Buffers: shared hit=396
        ->  Seq Scan on addresses a  (cost=0.00..796.00 rows=40000 width=16) (actual time=0.005..6.332 rows=40000.00 loops=1)
              Buffers: shared hit=396
Planning:
  Buffers: shared hit=12
Planning Time: 0.172 ms
Execution Time: 29.884 ms
```

Попробуем добавить B-Tree индекс для `price`, так как в этом поле хранятся числовые значения.
А как известно, B-Tree хорошо справляется с поиском по нижней / верхней границе для таких значений.

```sql
CREATE INDEX property_price_search_index on properties(price);
```

И проанализируем производительность выполнения запроса:

```
Query:
SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s)
  AND (%(owner_id)s IS NULL OR p.owner_id = %(owner_id)s);

Variables: {'city': None, 'min_price': 300, 'max_price': None, 'owner_id': None}

Analyze:
Hash Join  (cost=954.63..1625.62 rows=13354 width=16) (actual time=18.114..30.278 rows=39543.00 loops=1)
  Hash Cond: (a.id = p.address_id)
  Buffers: shared hit=883
  ->  Seq Scan on addresses a  (cost=0.00..613.80 rows=21780 width=16) (actual time=0.055..5.094 rows=40000.00 loops=1)
        Buffers: shared hit=396
  ->  Hash  (cost=787.71..787.71 rows=13354 width=32) (actual time=17.931..17.933 rows=39543.00 loops=1)
        Buckets: 65536 (originally 16384)  Batches: 1 (originally 1)  Memory Usage: 2984kB
        Buffers: shared hit=487
        ->  Bitmap Heap Scan on properties p  (cost=207.78..787.71 rows=13354 width=32) (actual time=2.026..9.568 rows=39543.00 loops=1)
              Recheck Cond: (price >= 300)
              Heap Blocks: exact=413
              Buffers: shared hit=487
              ->  Bitmap Index Scan on property_price_search_index  (cost=0.00..204.44 rows=13354 width=0) (actual time=1.971..1.971 rows=39543.00 loops=1)
                    Index Cond: (price >= 300)
                    Index Searches: 1
                    Buffers: shared hit=74
Planning Time: 0.086 ms
Execution Time: 31.719 ms
```

Теперь попробуем выполнить тот же запрос, но с другими значениями переменных.
Выполним поиск недвижимости по городу:

```shell
uv --project utils \
  run utils/analyze_query.py \
  -v dr \
  -v min_price=null \
  -v max_price=null \
  -v owner_id=null \
  api_gateway/postgresql/queries/find_properties.sql \
  connection.yaml
```

Получим следующую оценку производительности:

```
Query:
SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s)
  AND (%(owner_id)s IS NULL OR p.owner_id = %(owner_id)s);

Variables: {'city': 'dr', 'min_price': None, 'max_price': None, 'owner_id': None}

Analyze:
Hash Join  (cost=924.80..1842.81 rows=2464 width=16) (actual time=26.462..33.196 rows=2396.00 loops=1)
  Hash Cond: (p.address_id = a.id)
  Buffers: shared hit=807
  ->  Seq Scan on properties p  (cost=0.00..813.00 rows=40000 width=32) (actual time=0.012..2.951 rows=40000.00 loops=1)
        Buffers: shared hit=413
  ->  Hash  (cost=894.00..894.00 rows=2464 width=16) (actual time=26.378..26.379 rows=2396.00 loops=1)
        Buckets: 4096  Batches: 1  Memory Usage: 145kB
        Buffers: shared hit=394
        ->  Seq Scan on addresses a  (cost=0.00..894.00 rows=2464 width=16) (actual time=0.030..25.660 rows=2396.00 loops=1)
              Filter: ((city)::text ~~* '%dr%'::text)
              Rows Removed by Filter: 37604
              Buffers: shared hit=394
Planning:
  Buffers: shared hit=12
Planning Time: 0.164 ms
Execution Time: 33.290 ms
```

Как видим, производительность выполнения запроса для этого случая неоптимальная.
Попробуем, по аналогии с поиском пользователей, добавить GIN-индекс.

```sql
CREATE INDEX address_city_search_index ON addresses USING gin (city gin_trgm_ops);
```

Получим следующий результат:

```
Query:
SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s)
  AND (%(owner_id)s IS NULL OR p.owner_id = %(owner_id)s);

Variables: {'city': 'dr', 'min_price': None, 'max_price': None, 'owner_id': None}

Analyze:
Hash Join  (cost=825.07..1743.09 rows=2476 width=16) (actual time=30.801..36.940 rows=2469.00 loops=1)
  Hash Cond: (p.address_id = a.id)
  Buffers: shared hit=879
  ->  Seq Scan on properties p  (cost=0.00..813.00 rows=40000 width=32) (actual time=0.010..2.545 rows=40000.00 loops=1)
        Buffers: shared hit=413
  ->  Hash  (cost=794.12..794.12 rows=2476 width=16) (actual time=30.747..30.747 rows=2469.00 loops=1)
        Buckets: 4096  Batches: 1  Memory Usage: 148kB
        Buffers: shared hit=466
        ->  Bitmap Heap Scan on addresses a  (cost=369.17..794.12 rows=2476 width=16) (actual time=4.511..30.145 rows=2469.00 loops=1)
              Recheck Cond: ((city)::text ~~* '%dr%'::text)
              Rows Removed by Index Recheck: 37531
              Heap Blocks: exact=394
              Buffers: shared hit=466
              ->  Bitmap Index Scan on address_city_search_index  (cost=0.00..368.56 rows=2476 width=0) (actual time=4.453..4.453 rows=40000.00 loops=1)
                    Index Cond: ((city)::text ~~* '%dr%'::text)
                    Index Searches: 1
                    Buffers: shared hit=72
Planning:
  Buffers: shared hit=13
Planning Time: 0.165 ms
Execution Time: 37.165 ms
```

Теперь рассмотрим сценарий с поиском недвижимости пользователя:

```
Query:
SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s)
  AND (%(owner_id)s IS NULL OR p.owner_id = %(owner_id)s);

Variables: {'city': None, 'min_price': None, 'max_price': None, 'owner_id': '8402d32c-94b6-4fe8-ad5f-75b15441661b'}

Analyze:
Nested Loop  (cost=0.29..921.31 rows=1 width=57) (actual time=0.053..3.791 rows=1.00 loops=1)
  Buffers: shared hit=416
  ->  Seq Scan on properties p  (cost=0.00..913.00 rows=1 width=40) (actual time=0.019..3.755 rows=1.00 loops=1)
        Filter: (owner_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)
        Rows Removed by Filter: 39999
        Buffers: shared hit=413
  ->  Index Scan using addresses_pkey on addresses a  (cost=0.29..8.31 rows=1 width=49) (actual time=0.029..0.029 rows=1.00 loops=1)
        Index Cond: (id = p.address_id)
        Index Searches: 1
        Buffers: shared hit=3
Planning:
  Buffers: shared hit=12
Planning Time: 0.120 ms
Execution Time: 3.809 ms
```

Тут можно добавить B-Tree индекс, так как в условии используется только операция сравнения `=`:

```sql
CREATE INDEX property_owner_id_search_index on properties(owner_id);
```

Получаем следующий результат:

```
Query:
SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s)
  AND (%(owner_id)s IS NULL OR p.owner_id = %(owner_id)s);

Variables: {'city': None, 'min_price': None, 'max_price': None, 'owner_id': '8402d32c-94b6-4fe8-ad5f-75b15441661b'}

Analyze:
Nested Loop  (cost=0.58..16.62 rows=1 width=57) (actual time=0.117..0.119 rows=1.00 loops=1)
  Buffers: shared hit=4 read=2
  ->  Index Scan using property_owner_id_search_index on properties p  (cost=0.29..8.31 rows=1 width=40) (actual time=0.070..0.071 rows=1.00 loops=1)
        Index Cond: (owner_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)
        Index Searches: 1
        Buffers: shared hit=1 read=2
  ->  Index Scan using addresses_pkey on addresses a  (cost=0.29..8.31 rows=1 width=49) (actual time=0.042..0.042 rows=1.00 loops=1)
        Index Cond: (id = p.address_id)
        Index Searches: 1
        Buffers: shared hit=3
Planning:
  Buffers: shared hit=12
Planning Time: 0.245 ms
Execution Time: 0.147 ms
```

## Поиск просмотров

Рассмотрим запрос для поиска просмотров ([ссылка](../api_gateway/postgresql/queries/find_viewings.sql)).
Для оценки оптимальности выполнения запроса воспользуемся утилитой [`analyze_query.py`](../utils/analyze_query.py).
Рассмотрим сценарий с получением просмотров пользователя.

```shell
uv --project utils \
  run utils/analyze_query.py \
  -v user_id=8402d32c-94b6-4fe8-ad5f-75b15441661b \
  --v property_id=null \
  api_gateway/postgresql/queries/find_viewings.sql \
  connection.yaml
```

В итоге, получим следующий вывод:

```
Query:
SELECT id
FROM viewings
WHERE (%(user_id)s IS NULL OR user_id = %(user_id)s)
  AND (%(property_id)s IS NULL OR property_id = %(property_id)s);

Variables: {'user_id': '8402d32c-94b6-4fe8-ad5f-75b15441661b', 'property_id': None}

Analyze:
Seq Scan on viewings  (cost=0.00..913.00 rows=2 width=36) (actual time=1.218..2.964 rows=2.00 loops=1)
  Filter: (user_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)
  Rows Removed by Filter: 39998
  Buffers: shared hit=413
Planning Time: 0.062 ms
Execution Time: 2.980 ms
```

Как видим, запрос выполняется очень долго. Учтем, что в фильтре поиска используется только операция равенства `=`.
Попробуем добавить B-Tree индекс для полей `user_id`.

```sql
CREATE INDEX viewing_user_id_search_index on viewings(user_id);
```

Получим следующий результат:

```
Query:
SELECT id, property_id, viewing_date
FROM viewings
WHERE user_id = %(user_id)s;

Variables: {'user_id': '8402d32c-94b6-4fe8-ad5f-75b15441661b', 'property_id': None}

Analyze:
Bitmap Heap Scan on viewings  (cost=4.31..11.91 rows=2 width=36) (actual time=0.065..0.072 rows=2.00 loops=1)
  Recheck Cond: (user_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)
  Heap Blocks: exact=2
  Buffers: shared hit=2 read=2
  ->  Bitmap Index Scan on viewing_user_id_search_index  (cost=0.00..4.30 rows=2 width=0) (actual time=0.040..0.040 rows=2.00 loops=1)
        Index Cond: (user_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)
        Index Searches: 1
        Buffers: shared read=2
Planning Time: 0.042 ms
Execution Time: 0.086 ms
```

Для поиска по полю `property_id` запрос уже оптимально выполняется, так как он уже был добавлен
в индекс на этапе объявления ограничения:

```sql
CONSTRAINT unique_date_per_property UNIQUE (property_id, viewing_date)
```
