INSERT INTO addresses (country, city, street, building, apartment)
VALUES ($1, $2, $3, $4, $5)
RETURNING (id);
