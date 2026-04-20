UPDATE properties
SET
    status = COALESCE($2, status),
    price = COALESCE($3, price)
WHERE id = $1
RETURNING (id);
