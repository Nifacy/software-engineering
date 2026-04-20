# 🏢 Property Service API

REST API для системы по управлению недвижимостью

|          |                |
|----------|----------------|
| Вариант  | 24             |
| Выполнил | Гришин Алексей |
| Группа   | М8О-106СВ-25   |

## Getting started

### 📦 Запуск приложения

Для запуска сервиса в Docker контейнере необходимо собрать образ через следующую команду

```bash
docker build . -t api_gateway:latest
```

Далее, для запуска Docker контейнера на основе собранного образа необходимо ввести следующую
команду.

```bash
docker run \
    -e 'ACCESS_SECRET=<access-secret>' \
    -e 'REFRESH_SCRET=<refresh-secret>' \
    -p '8080:8080' \
    api_gateway:1
```

Для упрощенного запуска проект предоставляет сборку и запуск Docker контейнера через docker compose.
Для этого необходимо создать `.env` файл, в котором необходимо прописать следующие переменные окружения

```bash
ACCESS_SECRET="<access-secret>"
REFRESH_SECRET="<refresh-secret>"
```

И запустить следующую команду.

```bash
docker-compose -f ./docker-compose.yaml --env-file "<enf-file-path>" up
```

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

```bash
curl -X POST "http://localhost:8080/api/v1/auth/register" \
    -d '{
        "login": "dmitro",
        "password": "foo",
        "firstName": "Dmitro",
        "lastName": "Soshnikov"
    }'
```

### Создание объекта недвижимости

```bash
curl -X POST http://localhost:8080/api/v1/properties \
    -H "Authorization: Bearer $ACCESS_TOKEN" \
    -d '{
        "address": {
            "country": "Russia",
            "city": "Moscow",
            "street": "Voykovskaya",
            "building": 4,
            "apartment": 221
        },
        "price": 67
    }'
```

### Запись на просмотр объекта недвижимости

```bash
curl -X POST "http://localhost:8080/api/v1/properties/$PROPERTY_ID/viewings" \
    -H "Authorization: Bearer $ACCESS_TOKEN" \
    -d '{"date": "2026-10-20"}'
```
