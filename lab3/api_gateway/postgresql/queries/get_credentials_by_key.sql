SELECT (verify_secret, user_id)
FROM credentials
WHERE key = $1;