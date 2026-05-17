import datetime
import http
import random
import string
import uuid
from typing import Any
from typing import Literal

import zapros

type PropertyStatus = Literal["active", "sold"]


def _generate_uuid() -> str:
    return str(uuid.uuid4())


def get_random_login(suffix: str, *, prefix_len: int = 8) -> str:
    random_prefix = "".join(
        random.choice(string.ascii_letters) for _ in range(prefix_len)
    )
    return f"{random_prefix}-{suffix}"


def get_register_payload(
    *,
    login: str | None = None,
    first_name: str | None = None,
    last_name: str | None = None,
) -> dict[str, Any]:
    return {
        "login": login or _generate_uuid(),
        "password": _generate_uuid(),
        "firstName": first_name or "Bob",
        "lastName": last_name or "Smith",
    }


def get_property_creation_payload(
    *,
    price: int | None = None,
    city: str | None = None,
) -> dict[str, Any]:
    return {
        "address": {
            "country": "Russia",
            "city": city or "Moscow",
            "street": "Babushkinskaya",
            "building": 1,
            "apartment": 67,
        },
        "price": price or 1,
    }


def get_property_update_payload(status: PropertyStatus) -> dict[str, str]:
    return {"status": status}


def get_schedule_viewing_payload(date: datetime.date | None = None) -> dict[str, str]:
    date = date or datetime.date(year=2024, month=2, day=20)
    return {"date": date.strftime("%Y-%m-%d")}


def get_auth_headers(token: str) -> dict[str, str]:
    return {"Authorization": f"Bearer {token}"}


async def register(
    http_client: zapros.AsyncClient, payload: dict[str, Any] | None = None
) -> str:
    response = await http_client.post(
        "/api/v1/auth/register",
        json=payload or get_register_payload(),
    )
    assert_status(response, http.HTTPStatus.CREATED)
    return (await get_json(response))["accessToken"]


async def create_user(
    http_client: zapros.AsyncClient,
    payload: dict[str, Any] | None = None,
) -> tuple[str, str]:
    token = await register(http_client, payload)
    response = await http_client.get(
        "/api/v1/users/me",
        headers=get_auth_headers(token),
    )
    assert_status(response, http.HTTPStatus.OK)
    return token, (await get_json(response))["id"]


async def create_property(
    http_client: zapros.AsyncClient,
    token: str | None = None,
    payload: dict[str, Any] | None = None,
) -> dict[str, Any]:
    if token is None:
        token, _ = await create_user(http_client)

    creation_payload = payload or get_property_creation_payload()
    response = await http_client.post(
        "/api/v1/properties",
        json=creation_payload,
        headers=get_auth_headers(token),
    )
    assert_status(response, http.HTTPStatus.CREATED)
    return await get_json(response)


async def update_property(
    http_client: zapros.AsyncClient,
    token: str,
    property_id: str,
    payload: dict[str, Any],
) -> None:
    response = await http_client.patch(
        f"/api/v1/properties/{property_id}",
        json=payload,
        headers=get_auth_headers(token),
    )
    assert_status(response, http.HTTPStatus.OK)


async def schedule_viewing(
    http_client: zapros.AsyncClient,
    token: str,
    property_id: str,
    payload: dict[str, Any] | None = None,
) -> str:
    payload = payload or get_schedule_viewing_payload(
        datetime.date(year=2026, month=2, day=24)
        + datetime.timedelta(days=random.randint(1, 1_000))
    )
    response = await http_client.post(
        f"/api/v1/properties/{property_id}/viewings",
        json=payload,
        headers=get_auth_headers(token),
    )
    assert_status(response, http.HTTPStatus.CREATED)
    return (await get_json(response))["id"]


async def delete_viewing(
    http_client: zapros.AsyncClient,
    token: str,
    property_id: str,
    viewing_id: str,
) -> None:
    response = await http_client.delete(
        f"/api/v1/properties/{property_id}/viewings/{viewing_id}",
        headers=get_auth_headers(token),
    )
    assert_status(response, http.HTTPStatus.NO_CONTENT)


async def assert_content(response: zapros.Response, expected_content: str) -> None:
    await response.aread()
    assert response.text == expected_content, (
        f"Content of response not equals to expected: '{response.text}' != '{expected_content}'"
    )


def assert_status(response: zapros.Response, expected_status: http.HTTPStatus) -> None:
    assert response.status == expected_status, (
        f"Status code of response not equals to expected: {response.status} != {expected_status.value}"
    )


async def get_json(response: zapros.Response) -> Any:
    await response.aread()
    return response.json


async def assert_json(response: zapros.Response, expected_data: Any) -> None:
    data = await get_json(response)
    assert data == expected_data, (
        f"JSON response not equals to expected: {response.json} != {expected_data}"
    )
