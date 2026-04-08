CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE TABLE users (
  id UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  login VARCHAR UNIQUE NOT NULL,
  first_name VARCHAR NOT NULL,
  last_name VARCHAR NOT NULL
);

CREATE INDEX user_login_search_index ON users USING gin (login gin_trgm_ops);
CREATE INDEX user_first_name_search_index ON users USING gin (first_name gin_trgm_ops);
CREATE INDEX user_last_name_search_index ON users USING gin (last_name gin_trgm_ops);

CREATE TABLE credentials (
  key VARCHAR PRIMARY KEY NOT NULL,
  verify_secret VARCHAR NOT NULL,
  user_id UUID UNIQUE NOT NULL REFERENCES users(id)
);

CREATE TABLE addresses (
  id UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  country VARCHAR NOT NULL,
  city VARCHAR NOT NULL,
  street VARCHAR NOT NULL,
  building INTEGER NOT NULL CHECK (building > 0),
  apartment INTEGER CHECK (apartment > 0)
);

CREATE TYPE property_status AS ENUM ('active', 'sold');

CREATE TABLE properties (
  id UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  owner_id UUID NOT NULL REFERENCES users(id),
  address_id UUID UNIQUE NOT NULL REFERENCES addresses(id),
  status property_status NOT NULL,
  price INTEGER NOT NULL CHECK (price > 0)
);

CREATE INDEX property_price_search_index on properties(price);
CREATE INDEX address_city_search_index ON addresses USING gin (city gin_trgm_ops);

CREATE TABLE viewings (
  id UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  user_id UUID NOT NULL REFERENCES users(id),
  property_id UUID NOT NULL REFERENCES properties(id),
  viewing_date DATE NOT NULL,

  CONSTRAINT unique_date_per_property UNIQUE (property_id, viewing_date)
);
