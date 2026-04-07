INSERT INTO properties (owner_id, address_id, status, price)
VALUES ($1, $2, $3::property_status, $4)
RETURNING (id);
