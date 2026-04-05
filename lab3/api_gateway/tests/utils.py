from typing import Any, Literal, TypeAlias

import pytest_userver.client
import uuid
import random
import string
import datetime


PropertyStatus: TypeAlias = Literal['active', 'sold']


def _generate_uuid() -> str:
    return str(uuid.uuid4())


def get_random_login(suffix: str, *, prefix_len: int = 8) -> str:
    random_prefix = ''.join(random.choice(string.ascii_letters) for _ in range(prefix_len))
    return f'{random_prefix}-{suffix}'


# TODO: use keyword only arguments for optional values
def get_register_payload(login: str | None = None, first_name: str | None = None, last_name: str | None = None) -> dict[str, Any]:
    return {
        'login': login or _generate_uuid(),
        'password': _generate_uuid(),
        'firstName': first_name or 'Bob',
        'lastName': last_name or 'Smith',
    }


def get_property_creation_payload(price: int | None = None, city: str | None = None) -> dict[str, Any]:
    return {
        'address': {
            'country': 'Russia',
            'city': city or 'Moscow',
            'street': 'Babushkinskaya',
            'building': 1,
            'apartment': 67,
        },
        'price': price or 1,
    }


def get_property_update_payload(status: PropertyStatus) -> dict[str, str]:
    return {'status': status}


def get_schedule_viewing_payload(date: datetime.date | None = None) -> dict[str, str]:
    date = date or datetime.date(year=2024, month=2, day=20)
    return {'date': date.strftime('%Y-%m-%d')}


def get_auth_headers(token: str) -> dict[str, str]:
    return { 'Authorization': f'Bearer {token}' }


async def register(service_client: pytest_userver.client.Client, payload: dict[str, Any] | None = None) -> str:
    response = await service_client.post('/api/v1/auth/register', json=payload or get_register_payload())
    assert response.status == 201

    return response.json()['accessToken']


async def create_user(service_client: pytest_userver.client.Client, payload: dict[str, Any] | None = None) -> tuple[str, str]:
    token = await register(service_client, payload)
    response = await service_client.get('/api/v1/users/me', headers=get_auth_headers(token))
    assert response.status == 200

    return token, response.json()['id']


async def create_property(service_client: pytest_userver.client.Client, token: str | None = None, payload: dict[str, Any] | None = None) -> dict[str, Any]:
    if token is None:
        token, _ = await create_user(service_client)

    creation_payload = payload or get_property_creation_payload()
    response = await service_client.post('/api/v1/properties', json=creation_payload, headers=get_auth_headers(token))
    assert response.status == 201

    return response.json()


async def update_property(service_client: pytest_userver.client.Client, token: str, property_id: str, payload: dict[str, Any]) -> None:
    response = await service_client.patch(f'/api/v1/properties/{property_id}', json=payload, headers=get_auth_headers(token))
    assert response.status == 200


async def schedule_viewing(service_client: pytest_userver.client.Client, token: str, property_id: str, payload: dict[str, Any] | None = None) -> str:
    payload = payload or get_schedule_viewing_payload(datetime.date(year=2026, month=2, day=24) + datetime.timedelta(days=random.randint(1, 1_000)))
    response = await service_client.post(f'/api/v1/properties/{property_id}/viewings', json=payload, headers=get_auth_headers(token))
    assert response.status == 201
    return response.json()['id']


async def delete_viewing(service_client: pytest_userver.client.Client, token: str, property_id: str, viewing_id: str) -> None:
    response = await service_client.delete(f'/api/v1/properties/{property_id}/viewings/{viewing_id}', headers=get_auth_headers(token))
    assert response.status == 204
