## Названия полей

```
users: id, login, first_name, last_name
credentials: user_id, verify_secret, payload
addresses: id, country, city, street, building_number, apartment_number, postal_code, latitude, longitude
properties: id, owner_id, address_id, status, price
viewings: id, user_id, property_id, viewing_date, status
```

## Первичные ключи

```
users: id
credentials: user_id
addresses: id
properties: id
viewings: id
```

## Внешние ключи

| Таблица     | Foreign Key | На что ссылается | Тип связи |
|-------------|-------------|------------------|-----------|
| credentials | user_id     | users.id         | 1:1       |
| properties  | owner_id    | users.id         | 1:n       |
| properties  | address_id  | addresses.id     | 1:1       |
| viewings    | user_id     | users.id         | 1:n       |
| viewings    | property_id | properties.id    | 1:n       |


## Схема

```dbml
Enum PropertyStatus {
  active
  sold
}

Table users {
  id VARCHAR [pk, not null]
  login VARCHAR [not null, unique]
  first_name VARCHAR [not null]
  last_name VARCHAR [not null]
}

Table credentials {
  key VARCHAR [pk, not null]
  verify_secret VARCHAR [not null]
  user_id VARCHAR [unique, not null, ref: - users.id]
}

Table addresses {
  id VARCHAR [pk, not null]
  country VARCHAR [not null]
  city VARCHAR [not null]
  street VARCHAR [not null]
  building INTEGER [not null, check: `building > 0`]
  apartment INTEGER [check: `apartment > 0`]
}

Table properties {
  id VARCHAR [pk, not null]
  owner_id VARCHAR [not null, ref :> users.id]
  address_id VARCHAR [not null, ref : - addresses.id]
  status PropertyStatus [not null]
  price INTEGER [not null, check: `price > 0`]
}

Table viewings {
  id VARCHAR [pk, not null]
  user_id VARCHAR [not null, ref: > users.id]
  property_id VARCHAR [not null, ref: > properties.id]
  viewing_date DATE [not null]

  indexes {
    (property_id, viewing_date) [unique]
  }
}
```

### PostgresQL (миграция)

```sql
CREATE TYPE property_status AS ENUM ('active', 'sold');

CREATE TABLE "users" (
  "id" UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  "login" VARCHAR UNIQUE NOT NULL,
  "first_name" VARCHAR NOT NULL,
  "last_name" VARCHAR NOT NULL
);

CREATE TABLE "credentials" (
  "key" VARCHAR PRIMARY KEY NOT NULL,
  "verify_secret" VARCHAR NOT NULL,
  "user_id" UUID UNIQUE NOT NULL REFERENCES users(id)
);

CREATE TABLE "addresses" (
  "id" UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  "country" VARCHAR NOT NULL,
  "city" VARCHAR NOT NULL,
  "street" VARCHAR NOT NULL,
  "building" INTEGER NOT NULL CHECK (building > 0),
  "apartment" INTEGER CHECK (apartment > 0)
);

CREATE TABLE "properties" (
  "id" UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  "owner_id" UUID NOT NULL REFERENCES users(id),
  "address_id" UUID UNIQUE NOT NULL REFERENCES addresses(id),
  "status" property_status NOT NULL,
  "price" INTEGER NOT NULL CHECK (price > 0)
);

CREATE TABLE "viewings" (
  "id" UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  "user_id" UUID NOT NULL REFERENCES users(id),
  "property_id" UUID NOT NULL REFERENCES properties(id),
  viewing_date DATE NOT NULL,

  CONSTRAINT unique_date_per_property UNIQUE (property_id, viewing_date)
);
```

## Валидация данных

```sql
-- Test users
INSERT INTO users (id, login, first_name, last_name) VALUES (md5('user1')::uuid, 'user1', 'Alexey', 'Grishin');

-- Test addresses
INSERT INTO addresses (id, country, city, street, building) VALUES (md5('addr1')::uuid, 'Russia', 'Moscow', 'Voykovskaya', 13);

-- Test properties
INSERT INTO properties (id, owner_id, address_id, status, price) VALUES (md5('prop1')::uuid, md5('user1')::uuid, md5('addr1')::uuid, 'active', 200);
INSERT INTO properties (id, owner_id, address_id, status, price) VALUES (md5('prop2')::uuid, md5('user-unknown')::uuid, md5('addr2')::uuid, 'active', 200);
INSERT INTO properties (id, owner_id, address_id, status, price) VALUES (md5('prop2')::uuid, md5('user1')::uuid, md5('addr2')::uuid, 'unknown', 200);
INSERT INTO properties (id, owner_id, address_id, status, price) VALUES (md5('prop2')::uuid, md5('user1')::uuid, md5('addr2')::uuid, 'active', -200);

-- Test viewings
INSERT INTO viewings (id, user_id, property_id, viewing_date) VALUES (md5('view1')::uuid, md5('user1')::uuid, md5('prop1')::uuid, '2026-04-02');
INSERT INTO viewings (id, user_id, property_id, viewing_date) VALUES (md5('view2')::uuid, md5('user-unknown')::uuid, md5('prop1')::uuid, '2026-04-03');
INSERT INTO viewings (id, user_id, property_id, viewing_date) VALUES (md5('view2')::uuid, md5('user1')::uuid, md5('prop-unknown')::uuid, '2026-04-03');
INSERT INTO viewings (id, user_id, property_id, viewing_date) VALUES (md5('view2')::uuid, md5('user1')::uuid, md5('prop1')::uuid, 'foo');
INSERT INTO viewings (id, user_id, property_id, viewing_date) VALUES (md5('view3')::uuid, md5('user1')::uuid, md5('prop1')::uuid, '2026-04-02');
```

## Заполнение таблицы

```sql
INSERT INTO users (id, login, first_name, last_name) VALUES
    (md5('user_01')::uuid, 'user_01', 'Alexey', 'Grishin'),
    (md5('user_02')::uuid, 'user_02', 'Maria', 'Petrova'),
    (md5('user_03')::uuid, 'user_03', 'Dmitry', 'Sokolov'),
    (md5('user_04')::uuid, 'user_04', 'Elena', 'Kuznetsova'),
    (md5('user_05')::uuid, 'user_05', 'Ivan', 'Smirnov'),
    (md5('user_06')::uuid, 'user_06', 'Anna', 'Volkova'),
    (md5('user_07')::uuid, 'user_07', 'Sergey', 'Popov'),
    (md5('user_08')::uuid, 'user_08', 'Olga', 'Mikhailova'),
    (md5('user_09')::uuid, 'user_09', 'Nikita', 'Fedorov'),
    (md5('user_10')::uuid, 'user_10', 'Victoria', 'Orlova');

INSERT INTO addresses (id, country, city, street, building, apartment) VALUES
    (md5('addr_01')::uuid, 'Russia', 'Moscow', 'Tverskaya', 1, NULL),
    (md5('addr_02')::uuid, 'Russia', 'Moscow', 'Arbat', 15, NULL),
    (md5('addr_03')::uuid, 'Russia', 'Saint Petersburg', 'Nevsky Prospekt', 28, NULL),
    (md5('addr_04')::uuid, 'Russia', 'Kazan', 'Baumana', 42, NULL),
    (md5('addr_05')::uuid, 'Russia', 'Novosibirsk', 'Krasny Prospekt', 7, NULL),
    (md5('addr_06')::uuid, 'Russia', 'Yekaterinburg', 'Lenina', 54, 1),
    (md5('addr_07')::uuid, 'Russia', 'Moscow', 'Leninsky Prospekt', 33, 2),
    (md5('addr_08')::uuid, 'Russia', 'Sochi', 'Kurortny Prospekt', 12, 3),
    (md5('addr_09')::uuid, 'Russia', 'Vladivostok', 'Svetlanskaya', 89, 4),
    (md5('addr_10')::uuid, 'Russia', 'Moscow', 'Myasnitskaya', 20, 5);

INSERT INTO credentials (key, verify_secret, user_id) VALUES
    ('user_01', md5('user_01'), md5('user_01')::uuid),
    ('user_02', md5('user_02'), md5('user_02')::uuid),
    ('user_03', md5('user_03'), md5('user_03')::uuid),
    ('user_04', md5('user_04'), md5('user_04')::uuid),
    ('user_05', md5('user_05'), md5('user_05')::uuid),
    ('user_06', md5('user_06'), md5('user_06')::uuid),
    ('user_07', md5('user_07'), md5('user_07')::uuid),
    ('user_08', md5('user_08'), md5('user_08')::uuid),
    ('user_09', md5('user_09'), md5('user_09')::uuid),
    ('user_10', md5('user_10'), md5('user_10')::uuid);

INSERT INTO properties (id, owner_id, address_id, status, price) VALUES
    (md5('property_01')::uuid, md5('user_01')::uuid, md5('addr_01')::uuid, 'active', 100),    
    (md5('property_02')::uuid, md5('user_02')::uuid, md5('addr_02')::uuid, 'active', 200),    
    (md5('property_03')::uuid, md5('user_03')::uuid, md5('addr_03')::uuid, 'active', 300),    
    (md5('property_04')::uuid, md5('user_04')::uuid, md5('addr_04')::uuid, 'active', 400),    
    (md5('property_05')::uuid, md5('user_05')::uuid, md5('addr_05')::uuid, 'active', 500),    
    (md5('property_06')::uuid, md5('user_06')::uuid, md5('addr_06')::uuid, 'sold', 600),    
    (md5('property_07')::uuid, md5('user_07')::uuid, md5('addr_07')::uuid, 'sold', 700),    
    (md5('property_08')::uuid, md5('user_08')::uuid, md5('addr_08')::uuid, 'sold', 800),    
    (md5('property_09')::uuid, md5('user_09')::uuid, md5('addr_09')::uuid, 'sold', 900),    
    (md5('property_10')::uuid, md5('user_10')::uuid, md5('addr_10')::uuid, 'sold', 1000);

INSERT INTO viewings (id, user_id, property_id, viewing_date) VALUES
    (md5('viewing_01')::uuid, md5('user_01')::uuid, md5('property_10')::uuid, '2026-04-01'),
    (md5('viewing_02')::uuid, md5('user_02')::uuid, md5('property_01')::uuid, '2026-04-02'),
    (md5('viewing_03')::uuid, md5('user_03')::uuid, md5('property_02')::uuid, '2026-04-03'),
    (md5('viewing_04')::uuid, md5('user_04')::uuid, md5('property_03')::uuid, '2026-04-01'),
    (md5('viewing_05')::uuid, md5('user_05')::uuid, md5('property_04')::uuid, '2026-04-04'),
    (md5('viewing_06')::uuid, md5('user_06')::uuid, md5('property_05')::uuid, '2026-04-05'),
    (md5('viewing_07')::uuid, md5('user_07')::uuid, md5('property_06')::uuid, '2026-04-02'),
    (md5('viewing_08')::uuid, md5('user_08')::uuid, md5('property_07')::uuid, '2026-04-06'),
    (md5('viewing_09')::uuid, md5('user_09')::uuid, md5('property_08')::uuid, '2026-04-07'),
    (md5('viewing_10')::uuid, md5('user_10')::uuid, md5('property_09')::uuid, '2026-04-08');
```

## Примеры запросов

```sql
-- User authorization
SELECT user_id 
FROM credentials 
WHERE key = $1 AND verify_secret = $2 
LIMIT 1;

-- Find users by login / first name / last name
SELECT *
FROM users
WHERE (login ILIKE '%' || $1 || '%')
  AND (first_name ILIKE '%' || $2 || '%')
  AND (last_name ILIKE '%' || $3 || '%')

-- Get user info by ID
SELECT id, login, first_name, last_name
FROM users
WHERE (id = $1)

-- Find properties by city wildcard / min price / max price
SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (a.city ILIKE '%' || $1 || '%')
  AND (p.price >= $2::int)
  AND (p.price <= $3::int);

-- Find property by ID
SELECT 
  p.id AS property_id,
  p.owner_id,
  p.status,
  p.price,
  a.country as "address.country",
  a.city as "address.city",
  a.street as "address.street",
  a.building as "address.building",
  a.apartment as "address.apartment"
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE p.id = $1;

-- Get property viewings
SELECT id, user_id, viewing_date
FROM viewings
WHERE property_id = $1;

-- Delete property viewing
DELETE
FROM viewings
WHERE (property_id = $1) AND (id = $2);

-- Get user viewings
SELECT id, property_id, viewing_date
FROM viewings
WHERE user_id = $1;

-- Get user properties
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
WHERE p.owner_id = $1;
```

# Анализ запросов

Query:

SELECT user_id, verify_secret
FROM credentials 
WHERE key = 'user_buzz_31fef485-65a9-4f3f-bc90-65927279f1c9'
LIMIT 1;


Explain:
Limit  (cost=0.41..8.43 rows=1 width=53)
  ->  Index Scan using credentials_pkey on credentials  (cost=0.41..8.43 rows=1 width=53)
        Index Cond: ((key)::text = 'user_buzz_31fef485-65a9-4f3f-bc90-65927279f1c9'::text)

Analyze:
Limit  (cost=0.41..8.43 rows=1 width=53) (actual time=0.104..0.104 rows=1.00 loops=1)
  Buffers: shared hit=4
  ->  Index Scan using credentials_pkey on credentials  (cost=0.41..8.43 rows=1 width=53) (actual time=0.103..0.103 rows=1.00 loops=1)
        Index Cond: ((key)::text = 'user_buzz_31fef485-65a9-4f3f-bc90-65927279f1c9'::text)
        Index Searches: 1
        Buffers: shared hit=4
Planning Time: 0.062 ms
Execution Time: 0.115 ms

---

Query:

SELECT id
FROM users
WHERE (login ILIKE '%%' || %(login)s || '%%')
  AND (first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (last_name ILIKE '%%' || %(last_name)s || '%%');


Variables: {'login': 'user_', 'first_name': '', 'last_name': ''}

Explain:
Seq Scan on users  (cost=0.00..1265.50 rows=40996 width=16)
  Filter: (((login)::text ~~* '%user_%'::text) AND ((first_name)::text ~~* '%%'::text) AND ((last_name)::text ~~* '%%'::text))

Analyze:
Seq Scan on users  (cost=0.00..1265.50 rows=40996 width=16) (actual time=0.019..73.889 rows=41000.00 loops=1)
  Filter: (((login)::text ~~* '%user_%'::text) AND ((first_name)::text ~~* '%%'::text) AND ((last_name)::text ~~* '%%'::text))
  Buffers: shared hit=548
Planning Time: 0.135 ms
Execution Time: 75.211 ms

---

Query:

SELECT id, login, first_name, last_name
FROM users
WHERE (id = '31fef485-65a9-4f3f-bc90-65927279f1c9')


Explain:
Index Scan using users_pkey on users  (cost=0.29..8.31 rows=1 width=76)
  Index Cond: (id = '31fef485-65a9-4f3f-bc90-65927279f1c9'::uuid)

Analyze:
Index Scan using users_pkey on users  (cost=0.29..8.31 rows=1 width=76) (actual time=0.063..0.064 rows=1.00 loops=1)
  Index Cond: (id = '31fef485-65a9-4f3f-bc90-65927279f1c9'::uuid)
  Index Searches: 1
  Buffers: shared hit=3
Planning Time: 0.033 ms
Execution Time: 0.071 ms

---

Query:

SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s);


Variables: {'city': 'gr', 'min_price': None, 'max_price': None}

Explain:
Hash Join  (cost=711.75..1630.53 rows=8012 width=16)
  Hash Cond: (p.address_id = a.id)
  ->  Seq Scan on properties p  (cost=0.00..813.61 rows=40061 width=32)
  ->  Hash  (cost=658.12..658.12 rows=4290 width=16)
        ->  Seq Scan on addresses a  (cost=0.00..658.12 rows=4290 width=16)
              Filter: ((city)::text ~~* '%gr%'::text)

Analyze:
Hash Join  (cost=711.75..1630.53 rows=8012 width=16) (actual time=25.851..33.232 rows=4002.00 loops=1)
  Hash Cond: (p.address_id = a.id)
  Buffers: shared hit=803
  ->  Seq Scan on properties p  (cost=0.00..813.61 rows=40061 width=32) (actual time=0.027..3.681 rows=40000.00 loops=1)
        Buffers: shared hit=413
  ->  Hash  (cost=658.12..658.12 rows=4290 width=16) (actual time=25.698..25.698 rows=4002.00 loops=1)
        Buckets: 8192  Batches: 1  Memory Usage: 252kB
        Buffers: shared hit=390
        ->  Seq Scan on addresses a  (cost=0.00..658.12 rows=4290 width=16) (actual time=0.204..24.778 rows=4002.00 loops=1)
              Filter: ((city)::text ~~* '%gr%'::text)
              Rows Removed by Filter: 35998
              Buffers: shared hit=390
Planning Time: 0.073 ms
Execution Time: 33.378 ms

---

Query:

SELECT 
  p.id AS property_id,
  p.owner_id,
  p.status,
  p.price,
  a.country as "address.country",
  a.city as "address.city",
  a.street as "address.street",
  a.building as "address.building",
  a.apartment as "address.apartment"
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE p.id = '9010bad5-369d-4579-94f3-c1e3a607c641';


Explain:
Nested Loop  (cost=0.58..16.62 rows=1 width=72)
  ->  Index Scan using properties_pkey on properties p  (cost=0.29..8.31 rows=1 width=56)
        Index Cond: (id = '9010bad5-369d-4579-94f3-c1e3a607c641'::uuid)
  ->  Index Scan using addresses_pkey on addresses a  (cost=0.29..8.31 rows=1 width=48)
        Index Cond: (id = p.address_id)

Analyze:
Nested Loop  (cost=0.58..16.62 rows=1 width=72) (actual time=0.075..0.076 rows=1.00 loops=1)
  Buffers: shared hit=6
  ->  Index Scan using properties_pkey on properties p  (cost=0.29..8.31 rows=1 width=56) (actual time=0.049..0.049 rows=1.00 loops=1)
        Index Cond: (id = '9010bad5-369d-4579-94f3-c1e3a607c641'::uuid)
        Index Searches: 1
        Buffers: shared hit=3
  ->  Index Scan using addresses_pkey on addresses a  (cost=0.29..8.31 rows=1 width=48) (actual time=0.023..0.024 rows=1.00 loops=1)
        Index Cond: (id = p.address_id)
        Index Searches: 1
        Buffers: shared hit=3
Planning:
  Buffers: shared hit=12
Planning Time: 0.143 ms
Execution Time: 0.087 ms

---

Query:

SELECT id, user_id, viewing_date
FROM viewings
WHERE property_id = '7034511b-e1f6-4058-b9f1-17dc1d54b705';


Explain:
Bitmap Heap Scan on viewings  (cost=4.31..11.92 rows=2 width=36)
  Recheck Cond: (property_id = '7034511b-e1f6-4058-b9f1-17dc1d54b705'::uuid)
  ->  Bitmap Index Scan on unique_date_per_property  (cost=0.00..4.30 rows=2 width=0)
        Index Cond: (property_id = '7034511b-e1f6-4058-b9f1-17dc1d54b705'::uuid)

Analyze:
Bitmap Heap Scan on viewings  (cost=4.31..11.92 rows=2 width=36) (actual time=0.072..0.072 rows=0.00 loops=1)
  Recheck Cond: (property_id = '7034511b-e1f6-4058-b9f1-17dc1d54b705'::uuid)
  Buffers: shared hit=2
  ->  Bitmap Index Scan on unique_date_per_property  (cost=0.00..4.30 rows=2 width=0) (actual time=0.062..0.062 rows=0.00 loops=1)
        Index Cond: (property_id = '7034511b-e1f6-4058-b9f1-17dc1d54b705'::uuid)
        Index Searches: 1
        Buffers: shared hit=2
Planning Time: 0.034 ms
Execution Time: 0.088 ms

---

Query:

SELECT id, property_id, viewing_date
FROM viewings
WHERE user_id = '31fef485-65a9-4f3f-bc90-65927279f1c9';


Explain:
Seq Scan on viewings  (cost=0.00..935.50 rows=2 width=36)
  Filter: (user_id = '31fef485-65a9-4f3f-bc90-65927279f1c9'::uuid)

Analyze:
Seq Scan on viewings  (cost=0.00..935.50 rows=2 width=36) (actual time=3.595..3.596 rows=0.00 loops=1)
  Filter: (user_id = '31fef485-65a9-4f3f-bc90-65927279f1c9'::uuid)
  Rows Removed by Filter: 41000
  Buffers: shared hit=423
Planning Time: 0.056 ms
Execution Time: 3.610 ms

---

Query:

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


Explain:
Nested Loop  (cost=0.29..943.81 rows=1 width=56)
  ->  Seq Scan on properties p  (cost=0.00..935.50 rows=1 width=40)
        Filter: (owner_id = '31fef485-65a9-4f3f-bc90-65927279f1c9'::uuid)
  ->  Index Scan using addresses_pkey on addresses a  (cost=0.29..8.31 rows=1 width=48)
        Index Cond: (id = p.address_id)

Analyze:
Nested Loop  (cost=0.29..943.81 rows=1 width=56) (actual time=0.029..2.625 rows=1.00 loops=1)
  Buffers: shared hit=426
  ->  Seq Scan on properties p  (cost=0.00..935.50 rows=1 width=40) (actual time=0.015..2.611 rows=1.00 loops=1)
        Filter: (owner_id = '31fef485-65a9-4f3f-bc90-65927279f1c9'::uuid)
        Rows Removed by Filter: 40999
        Buffers: shared hit=423
  ->  Index Scan using addresses_pkey on addresses a  (cost=0.29..8.31 rows=1 width=48) (actual time=0.009..0.009 rows=1.00 loops=1)
        Index Cond: (id = p.address_id)
        Index Searches: 1
        Buffers: shared hit=3
Planning:
  Buffers: shared hit=12
Planning Time: 0.120 ms
Execution Time: 2.637 ms

## Оптимизации

### Поиск пользователей

Возьмем запрос

```
SELECT id
FROM users
WHERE (login ILIKE '%%' || %(login)s || '%%')
  AND (first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (last_name ILIKE '%%' || %(last_name)s || '%%');
```

Видно, что всегда искать по маске неоптимально. Пробуем `NULL` использовать, чтобы проверка по маске пропускалась.

```
Query:

SELECT id
FROM users
WHERE (%(login)s IS NULL OR login ILIKE '%%' || %(login)s || '%%')
  AND (%(first_name)s IS NULL OR first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (%(last_name)s IS NULL OR last_name ILIKE '%%' || %(last_name)s || '%%');


Variables: {'login': 'user_', 'first_name': None, 'last_name': None}

Explain:
Seq Scan on users  (cost=0.00..1060.50 rows=40996 width=16)
  Filter: ((login)::text ~~* '%user_%'::text)

Analyze:
Seq Scan on users  (cost=0.00..1060.50 rows=40996 width=16) (actual time=0.019..36.967 rows=41000.00 loops=1)
  Filter: ((login)::text ~~* '%user_%'::text)
  Buffers: shared hit=548
Planning Time: 0.115 ms
Execution Time: 38.219 ms
```

С индексом B-Tree:

```
Query:

SELECT id
FROM users
WHERE (%(login)s IS NULL OR login ILIKE '%%' || %(login)s || '%%')
  AND (%(first_name)s IS NULL OR first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (%(last_name)s IS NULL OR last_name ILIKE '%%' || %(last_name)s || '%%');


Variables: {'login': 'user_', 'first_name': None, 'last_name': None}

Explain:
Seq Scan on users  (cost=0.00..921.15 rows=223 width=16)
  Filter: ((login)::text ~~* '%user_%'::text)

Analyze:
Seq Scan on users  (cost=0.00..921.15 rows=223 width=16) (actual time=3.757..3.758 rows=0.00 loops=1)
  Filter: ((login)::text ~~* '%user_%'::text)
  Buffers: shared hit=534
Planning Time: 0.031 ms
Execution Time: 3.765 ms
```

Далее, видим, что нет индекса. Добавим индекс в БД. Так как ищем по тексту, то больше подойдет индекс GIN
([обоснование](https://postgrespro.ru/docs/postgrespro/current/gin))

```
Query:

SELECT id
FROM users
WHERE (%(login)s IS NULL OR login ILIKE '%%' || %(login)s || '%%')
  AND (%(first_name)s IS NULL OR first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (%(last_name)s IS NULL OR last_name ILIKE '%%' || %(last_name)s || '%%');


Variables: {'login': 'user_', 'first_name': None, 'last_name': None}

Explain:
Bitmap Heap Scan on users  (cost=324.47..740.59 rows=223 width=16)
  Recheck Cond: ((login)::text ~~* '%user_%'::text)
  ->  Bitmap Index Scan on users_login_search_index  (cost=0.00..324.41 rows=223 width=0)
        Index Cond: ((login)::text ~~* '%user_%'::text)

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

Попробуем создать единый индекс для всех трех полей

```
CREATE INDEX idx_users_all_trgm 
  ON "users" USING gin (
      "login"        gin_trgm_ops,
      "first_name"   gin_trgm_ops,
      "last_name"    gin_trgm_ops
  );
```

Результат:
```
Query:

SELECT id
FROM users
WHERE (%(login)s IS NULL OR login ILIKE '%%' || %(login)s || '%%')
  AND (%(first_name)s IS NULL OR first_name ILIKE '%%' || %(first_name)s || '%%')
  AND (%(last_name)s IS NULL OR last_name ILIKE '%%' || %(last_name)s || '%%');


Variables: {'login': 'user_', 'first_name': None, 'last_name': None}

Explain:
Seq Scan on users  (cost=0.00..921.15 rows=223 width=16)
  Filter: ((login)::text ~~* '%user_%'::text)

Analyze:
Seq Scan on users  (cost=0.00..921.15 rows=223 width=16) (actual time=2.526..2.526 rows=0.00 loops=1)
  Filter: ((login)::text ~~* '%user_%'::text)
  Buffers: shared hit=534
Planning:
  Buffers: shared hit=1
Planning Time: 0.042 ms
Execution Time: 2.534 ms
```

Видим, что хуже, так как GIN плохо справляется с составным поиском. Выгоднее использовать BinaryOR
как оптимизацию.

Итоговая схема:

```sql
CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE TABLE "users" (
  "id" UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  "login" VARCHAR UNIQUE NOT NULL,
  "first_name" VARCHAR NOT NULL,
  "last_name" VARCHAR NOT NULL
);

CREATE INDEX users_login_search_index ON users USING gin ("login" gin_trgm_ops);
CREATE INDEX users_first_name_search_index ON users USING gin ("first_name" gin_trgm_ops);
CREATE INDEX users_last_name_search_index ON users USING gin ("last_name" gin_trgm_ops);
```

### Поиск недвижимости

Следующий кандидат на оптимизацию:
```sql
SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s);
```

Для начала рассмотрим вот такой запрос:
```
Query:

SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s);


Variables: {'city': None, 'min_price': 300, 'max_price': None}

Explain:
Hash Join  (cost=1296.00..2312.93 rows=39589 width=16)
  Hash Cond: (p.address_id = a.id)
  ->  Seq Scan on properties p  (cost=0.00..913.00 rows=39589 width=32)
        Filter: (price >= 300)
  ->  Hash  (cost=796.00..796.00 rows=40000 width=16)
        ->  Seq Scan on addresses a  (cost=0.00..796.00 rows=40000 width=16)

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

Попробуем добавить B-Tree индекс для `price`:

```sql
CREATE INDEX property_price_search_index on properties(price);
```

Как видим, для этого запроса все лучше стало:
```
Query:

SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s);


Variables: {'city': None, 'min_price': 300, 'max_price': None}

Explain:
Hash Join  (cost=954.63..1625.62 rows=13354 width=16)
  Hash Cond: (a.id = p.address_id)
  ->  Seq Scan on addresses a  (cost=0.00..613.80 rows=21780 width=16)
  ->  Hash  (cost=787.71..787.71 rows=13354 width=32)
        ->  Bitmap Heap Scan on properties p  (cost=207.78..787.71 rows=13354 width=32)
              Recheck Cond: (price >= 300)
              ->  Bitmap Index Scan on property_price_search_index  (cost=0.00..204.44 rows=13354 width=0)
                    Index Cond: (price >= 300)

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

Query:

SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s);


Variables: {'city': 'dr', 'min_price': None, 'max_price': None}

Explain:
Hash Join  (cost=924.80..1842.81 rows=2464 width=16)
  Hash Cond: (p.address_id = a.id)
  ->  Seq Scan on properties p  (cost=0.00..813.00 rows=40000 width=32)
  ->  Hash  (cost=894.00..894.00 rows=2464 width=16)
        ->  Seq Scan on addresses a  (cost=0.00..894.00 rows=2464 width=16)
              Filter: ((city)::text ~~* '%dr%'::text)

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

Query:

SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE (%(city)s IS NULL OR a.city ILIKE '%%' || %(city)s || '%%')
  AND (%(min_price)s IS NULL OR p.price >= %(min_price)s)
  AND (%(max_price)s IS NULL OR p.price <= %(max_price)s);


Variables: {'city': 'dr', 'min_price': None, 'max_price': None}

Explain:
Hash Join  (cost=825.07..1743.09 rows=2476 width=16)
  Hash Cond: (p.address_id = a.id)
  ->  Seq Scan on properties p  (cost=0.00..813.00 rows=40000 width=32)
  ->  Hash  (cost=794.12..794.12 rows=2476 width=16)
        ->  Bitmap Heap Scan on addresses a  (cost=369.17..794.12 rows=2476 width=16)
              Recheck Cond: ((city)::text ~~* '%dr%'::text)
              ->  Bitmap Index Scan on address_city_search_index  (cost=0.00..368.56 rows=2476 width=0)
                    Index Cond: ((city)::text ~~* '%dr%'::text)

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

### Получение просмотров пользователя

```
Query:

SELECT id, property_id, viewing_date
FROM viewings
WHERE user_id = %(user_id)s;


Variables: {'user_id': '8402d32c-94b6-4fe8-ad5f-75b15441661b'}

Explain:
Seq Scan on viewings  (cost=0.00..913.00 rows=2 width=36)
  Filter: (user_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)

Analyze:
Seq Scan on viewings  (cost=0.00..913.00 rows=2 width=36) (actual time=1.218..2.964 rows=2.00 loops=1)
  Filter: (user_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)
  Rows Removed by Filter: 39998
  Buffers: shared hit=413
Planning Time: 0.062 ms
Execution Time: 2.980 ms
```

Как видим, тут запрос очень долго выполняется. Поможет индекс. Но учитываем что используется только операция `=`. Попробуем
добавить BTree индекс.

```sql
CREATE INDEX viewing_user_id_search_index on viewings(user_id);
```

```
Query:

SELECT id, property_id, viewing_date
FROM viewings
WHERE user_id = %(user_id)s;


Variables: {'user_id': '8402d32c-94b6-4fe8-ad5f-75b15441661b'}

Explain:
Bitmap Heap Scan on viewings  (cost=4.31..11.91 rows=2 width=36)
  Recheck Cond: (user_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)
  ->  Bitmap Index Scan on viewing_user_id_search_index  (cost=0.00..4.30 rows=2 width=0)
        Index Cond: (user_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)

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

Все работает нормально.

### Получение недвижимости пользователя

```
Query:

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
WHERE p.owner_id = %(user_id)s;


Variables: {'user_id': '8402d32c-94b6-4fe8-ad5f-75b15441661b'}

Explain:
Nested Loop  (cost=0.29..921.31 rows=1 width=57)
  ->  Seq Scan on properties p  (cost=0.00..913.00 rows=1 width=40)
        Filter: (owner_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)
  ->  Index Scan using addresses_pkey on addresses a  (cost=0.29..8.31 rows=1 width=49)
        Index Cond: (id = p.address_id)

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

Тут можем аналогично с получением просмотров пользователя. Добавляем индекс

```sql
CREATE INDEX property_owner_id_search_index on properties(owner_id);
```

Теперь все работает

```
Query:

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
WHERE p.owner_id = %(user_id)s;


Variables: {'user_id': '8402d32c-94b6-4fe8-ad5f-75b15441661b'}

Explain:
Nested Loop  (cost=0.58..16.62 rows=1 width=57)
  ->  Index Scan using property_owner_id_search_index on properties p  (cost=0.29..8.31 rows=1 width=40)
        Index Cond: (owner_id = '8402d32c-94b6-4fe8-ad5f-75b15441661b'::uuid)
  ->  Index Scan using addresses_pkey on addresses a  (cost=0.29..8.31 rows=1 width=49)
        Index Cond: (id = p.address_id)

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
