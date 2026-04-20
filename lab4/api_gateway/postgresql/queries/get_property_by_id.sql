SELECT 
  p.owner_id,
  p.status,
  p.price,
  a.country as "address__country",
  a.city as "address__city",
  a.street as "address__street",
  a.building as "address__building",
  a.apartment as "address__apartment"
FROM properties p
JOIN addresses a ON p.address_id = a.id
WHERE p.id = $1;
