SELECT id
FROM viewings
WHERE ($1 IS NULL OR user_id = $1)
  AND ($2 IS NULL OR property_id = $2);
