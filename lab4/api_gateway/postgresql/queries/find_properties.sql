SELECT p.id
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE ($1 IS NULL OR a.city ILIKE '%%' || $1 || '%%')
  AND ($2 IS NULL OR p.price >= $2)
  AND ($3 IS NULL OR p.price <= $3)
  AND ($4 IS NULL OR p.owner_id = $4);
