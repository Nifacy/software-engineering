# 🏢 Property Service API

REST API для системы по управлению недвижимостью

|          |                |
|----------|----------------|
| Вариант  | 24             |
| Выполнил | Гришин Алексей |
| Группа   | М8О-106СВ-25   |

## Getting started

### 📦 Запуск приложения

TODO

### 📍 API Endpoints

Для ознакомления с набором доступных эндпоинтов сервис предоставляет endpoint
`/api/v1/docs` по которому открывается Swagger UI.

TODO: Add picture

Для получения схемы API по спецификации OpenAPI 3.0 сервис предоставляет endpoint
`/api/v1/docs/openapi.yaml`.

```bash
curl "http://localhost:8080/api/v1/docs/openapi.yaml"
```

### 🔑 Авторизация

Авторизация сервиса основана на JWT токенах. При регистрации или аутентификации
возвращается JSON объект, содержащий 2 токена: access-токен и refresh-токен.

```json
{
    "accessToken": "<access-token>",
    "refreshToken": "<refresh-token>"
}
```

Access-токен необходим для авторизации запросов и передается в заголовке `Authorization`.
Сервис использует тип авторизации `Bearer`. Ниже предоставлен пример отправки авторизованного
запроса в сервис.

```bash
curl -X POST -H "Authorization: Bearer <access-token>" "http://localhost:8080/api/v1/example"
```

Access-токен имеет ограниченное время жизни и со временем может стать невалидным. Для перевыпуска
access-токена сервис предоставляет endpoint `/api/v1/auth/refresh`, куда в качестве тела запроса
передается refresh-токен. Ниже предоставлен пример отправки запроса на перевыпуск токена.

```bash
curl -X POST "http://localhost:8080/api/v1/auth/refresh" \
    -d '{
        "refreshToken": "<refresh-token>"
    }'
```

## Примеры запросов

### Создание пользователя

TODO

### Создание объекта недвижимости

TODO

### Запись на просмотр объекта недвижимости

TODO
