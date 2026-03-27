from unittest.mock import ANY

import pytest
import pytest_userver.client
from . import utils


def _get_filter_params(login: str | None = None, first_name: str | None = None, last_name: str | None = None) -> dict[str, str]:
    filters = {'login': login, 'firstName': first_name, 'lastName': last_name}
    return {key: value for key, value in filters.items() if value is not None}


# Get current user

async def test_returns_current_user(service_client: pytest_userver.client.Client):
    payload = utils.get_register_payload()
    token = await utils.register(service_client, payload)

    response = await service_client.get('/users/me', headers=utils.get_auth_headers(token))
    assert response.status == 200

    user_info = response.json()
    assert user_info == {
        'id': ANY,
        'login': payload['login'],
        'firstName': payload['firstName'],
        'lastName': payload['lastName'],
    }


async def test_returns_current_user_not_authorized(service_client: pytest_userver.client.Client):
    response = await service_client.get('/users/me')
    assert response.status == 400


# Get user

async def test_returns_user_by_id(service_client: pytest_userver.client.Client):
    payload = utils.get_register_payload()
    _, user_id = await utils.create_user(service_client, payload)

    response = await service_client.get(f'/users/{user_id}')
    assert response.status == 200

    user_info = response.json()
    assert user_info == {
        'id': user_id,
        'login': payload['login'],
        'firstName': payload['firstName'],
        'lastName': payload['lastName'],
    }


async def test_gets_unknown_user(service_client: pytest_userver.client.Client):
    response = await service_client.get('/users/unknown')
    assert response.status == 404


# Find users

@pytest.mark.parametrize(
    'filters,found_user_indexes',
    [
        pytest.param(_get_filter_params(), [0, 1, 2], id='all_users'),
        pytest.param(_get_filter_params(login='foo'), [0], id='filter_by_login'),
        pytest.param(_get_filter_params(first_name='A'), [1], id='filter_by_first_name'),
        pytest.param(_get_filter_params(last_name='B'), [2], id='filter_by_last_name'),
    ]
)
async def test_finds_users(service_client: pytest_userver.client.Client, filters: dict[str, str], found_user_indexes: list[int]):
    register_payloads = [
        utils.get_register_payload(login=utils.get_random_login('foo'), first_name='C', last_name='D'),
        utils.get_register_payload(login=utils.get_random_login('bar'), first_name='A', last_name='D'),
        utils.get_register_payload(login=utils.get_random_login('buzz'), first_name='C', last_name='B'),
    ]

    user_ids = [
        (await utils.create_user(service_client, payload))[1]
        for payload in register_payloads
    ]

    response = await service_client.get('/users', params=filters)
    assert response.status == 200

    found_user_ids = {user_id for user_id in response.json()['userIds'] if user_id in user_ids}
    expected_user_ids = {user_ids[index] for index in found_user_indexes}

    assert found_user_ids == expected_user_ids
