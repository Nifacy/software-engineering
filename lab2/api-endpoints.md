- `/v1/auth/register` - регистрация пользователя

  Payload:
  ```json
  {
    "login": "{login}",
    "password": "{password}",
    "firstName": "{firstName}",
    "lastName": "{lastName}"
  }
  ```

- `/v1/auth/login`

  Payload:
  ```json
  {
    "login": "{login}",
    "password": "{password}"
  }
  ```

- `/v1/auth/refresh`

  Payload:
  ```json
  {
    "refreshToken": "{refreshToken}"
  }
  ```

- `[GET] /users?login={login}`

  Response:
  ```json
  {
    "user_ids": ["{user-id}", "{user-id}"]
  }
  ```

- `[GET] /users?firstName={firstName}&secondName={secondName}`

  Response:
  ```json
  {
    "user_ids": ["{user-id}", "{user-id}"]
  }
  ```

- `[GET] /users/{id}`

  Response: user object

- `[GET] /users/me` (auth)

  Response: user object

- `[POST] /properties` (auth)

  Response: property object

- `[GET] /properties?city={city}`

  Response:
  ```json
  {
    "property_ids": ["{property-id}", "{property-id}"]
  }
  ```

- `[GET] /properties?minPrices={minPrice}&maxPrices={maxPrice}`

  Response:
  ```json
  {
    "property_ids": ["{property-id}", "{property-id}"]
  }
  ```

- `[GET] /properties/{id}`

  Response: property object

- `[PATCH] /properties/{id}`

  Response: property object

- `[POST] /viewings`

  Response: viewing object

- `[GET] /viewings/{id}`

  Response: viewing object

- `[DELETE] /viewings/{id}`

  Response: None

- `[PATCH] /viewings/{id}`

  Response: viewing object

## Response objects

### User

```json
{
    "id": "{id}",
    "login": "{login}",
    "firstName": "{firstName}",
    "secondName": "{secondName}"
}
```

### Current User

```json
{
    "id": "{id}",
    "login": "{login}",
    "firstName": "{firstName}",
    "secondName": "{secondName}",
    "viewings": ["{view-id}"],
    "properties": ["{property}"]
}
```

### Property

```json
{
    "id": "{property-id}",
    "ownerId": "{owner-id}",
    "address": {
        "country": "{country}",
        "city": "{city}",
        "street": "{street}",
        "building": "{building}",
        "apartment": "{apartment}"
    },
    "viewingIds": ["{viewing-id}"],
    "status": ["ACTIVE", "SOLD"]
}
```

