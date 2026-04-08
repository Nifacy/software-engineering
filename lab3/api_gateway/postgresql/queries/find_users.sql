SELECT id
FROM users
WHERE ($1 IS NULL OR login ILIKE '%%' || $1 || '%%')
  AND ($2 IS NULL OR first_name ILIKE '%%' || $2 || '%%')
  AND ($3 IS NULL OR last_name ILIKE '%%' || $3 || '%%');
