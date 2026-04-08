SELECT (user_id, property_id, viewing_date)
FROM viewings
WHERE id = $1;
