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
