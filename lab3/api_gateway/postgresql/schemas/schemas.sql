CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE TABLE users (
  id UUID PRIMARY KEY NOT NULL DEFAULT gen_random_uuid(),
  login VARCHAR UNIQUE NOT NULL,
  first_name VARCHAR NOT NULL,
  last_name VARCHAR NOT NULL
);

CREATE INDEX idx_users_all_trgm ON users USING gin (
    login gin_trgm_ops,
    first_name gin_trgm_ops,
    last_name gin_trgm_ops
);

CREATE TABLE credentials (
  key VARCHAR PRIMARY KEY NOT NULL,
  verify_secret VARCHAR NOT NULL,
  user_id UUID UNIQUE NOT NULL REFERENCES users(id)
);
