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
INSERT INTO users (id, login, first_name, last_name) VALUES (md5('user1')::uuid, md5('user1')::uuid, 'Alexey', 'Grishin');

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
