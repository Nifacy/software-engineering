-- Active: 1775407683391@@0.0.0.0@5432
INSERT INTO viewings (user_id, property_id, viewing_date)
VALUES ($1, $2, $3)
RETURNING id;
