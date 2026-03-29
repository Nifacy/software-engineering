from typing import Any
from unittest.mock import ANY
import pytest_userver.client
import pytest

from . import utils


def _get_update_payload(price: int | None = None, status: str | None = None) -> dict[str, Any]:
    payload = {}

    if price is not None:
        payload['price'] = price

    if status is not None:
        payload['status'] = status

    return payload


def _get_filter_params(min_price: int | None = None, max_price: int | None = None, city: str | None = None) -> dict[str, Any]:
    params = {'minPrice': min_price, 'maxPrice': max_price, 'city': city}
    return {key: value for key, value in params.items() if value is not None}



async def test_creates_property(service_client: pytest_userver.client.Client):
    token, user_id = await utils.create_user(service_client)
    creation_payload = utils.get_property_creation_payload()

    response = await service_client.post('/properties', json=creation_payload, headers=utils.get_auth_headers(token))
    response.status == 201

    created_property = response.json()
    assert created_property == {
        'id': ANY,
        'address': creation_payload['address'],
        'ownerId': user_id,
        'price': creation_payload['price'],
        'status': 'active',
    }


async def test_unable_to_create_if_unauthorized(service_client: pytest_userver.client.Client):
    creation_payload = utils.get_property_creation_payload()
    response = await service_client.post('/properties', json=creation_payload)
    response.status == 401


@pytest.mark.parametrize(
    'payload',
    [
        pytest.param({'unknown': 'field'}, id='wrong_payload_structure'),
        pytest.param(utils.get_property_creation_payload(price=-1), id='invalid_price_value'),
    ],
)
async def test_validates_creation_payload(service_client: pytest_userver.client.Client, payload: dict[str, Any]):
    token, _ = await utils.create_user(service_client)
    response = await service_client.post('/properties', json=payload, headers=utils.get_auth_headers(token))
    response.status == 400


# Get poroperty


async def test_raises_if_property_not_exists(service_client: pytest_userver.client.Client):
    response = await service_client.get('/properties/unknown')
    response.status == 404


async def test_returns_found_property(service_client: pytest_userver.client.Client):
    token, _ = await utils.create_user(service_client)
    property = await utils.create_property(service_client, token)
    property_id = property['id']

    response = await service_client.get(f'/properties/{property_id}')
    response.status == 200

    found_property = response.json()
    assert found_property == property


# Update property


@pytest.mark.parametrize(
    'payload',
    [
        pytest.param(_get_update_payload(), id='nothing_change'),
        pytest.param(_get_update_payload(price=6767), id='change_price'),
        pytest.param(_get_update_payload(status='sold'), id='change_status'),
        pytest.param(_get_update_payload(price=6767, status='sold'), id='change_status_and_price'),
    ]
)
async def test_updates_property(service_client: pytest_userver.client.Client, payload: dict[str, Any]):
    token, _ = await utils.create_user(service_client)
    property = await utils.create_property(service_client, token)
    property_id = property['id']

    response = await service_client.patch(f'/properties/{property_id}', json=payload, headers=utils.get_auth_headers(token))
    response.status == 200

    assert response.json() == {**property, **payload}


async def test_update_not_authorized(service_client: pytest_userver.client.Client):
    token, _ = await utils.create_user(service_client)
    property = await utils.create_property(service_client, token)
    property_id = property['id']

    response = await service_client.patch(f'/properties/{property_id}', json=_get_update_payload())
    assert response.status == 400

async def test_raises_if_update_unknown_property(service_client: pytest_userver.client.Client):
    token, _ = await utils.create_user(service_client)
    response = await service_client.patch('/properties/unknown', json=_get_update_payload(), headers=utils.get_auth_headers(token))
    response.status == 404


@pytest.mark.parametrize(
    'payload',
    [
        pytest.param({'unknown': 'field'}, id='invalid_payload'),
        pytest.param(_get_update_payload(price=-2), id='invalid_price_value'),
        pytest.param(_get_update_payload(status='unknown'), id='invalid_status_value'),
    ]
)
async def test_validates_update_payload(service_client: pytest_userver.client.Client, payload: dict[str, Any]):
    token, _ = await utils.create_user(service_client)
    property = await utils.create_property(service_client, token)
    property_id = property['id']

    response = await service_client.patch(f'/properties/{property_id}', json=payload, headers=utils.get_auth_headers(token))
    response.status == 400


async def test_validates_if_update_owned_property(service_client: pytest_userver.client.Client):
    token1, _ = await utils.create_user(service_client)
    token2, _ = await utils.create_user(service_client)

    property = await utils.create_property(service_client, token1)
    property_id = property['id']

    response = await service_client.patch(f'/properties/{property_id}', json=_get_update_payload(), headers=utils.get_auth_headers(token2))
    assert response.status == 403


# User properties


async def test_gets_properties_not_authorized(service_client: pytest_userver.client.Client):
    response = await service_client.get('/users/me/properties')
    assert response.status == 400


async def test_returns_empty_property_list_by_default(service_client: pytest_userver.client.Client):
    token, _ = await utils.create_user(service_client)
    response = await service_client.get('/users/me/properties', headers=utils.get_auth_headers(token))
    assert response.status == 200
    assert response.json() == {'properties': []}


async def test_adds_created_property_to_list(service_client: pytest_userver.client.Client):
    token, _ = await utils.create_user(service_client)
    property = await utils.create_property(service_client, token)

    response = await service_client.get('/users/me/properties', headers=utils.get_auth_headers(token))
    assert response.status == 200

    user_properties = response.json()
    assert user_properties == {'properties': [
        {
            'id': property['id'],
            'address': property['address'],
            'price': property['price'],
            'status': property['status'],            
        }
    ]}


async def test_filters_properties(service_client: pytest_userver.client.Client):
    token1, _ = await utils.create_user(service_client)
    token2, _ = await utils.create_user(service_client)

    await utils.create_property(service_client, token1)
    property = await utils.create_property(service_client, token2)

    response = await service_client.get('/users/me/properties', headers=utils.get_auth_headers(token2))
    assert response.status == 200

    user_properties = response.json()
    assert user_properties == {'properties': [
        {
            'id': property['id'],
            'address': property['address'],
            'price': property['price'],
            'status': property['status'],
        }
    ]}


# Find properties

@pytest.mark.parametrize(
    'params,found_property_indexes',
    [
        pytest.param(_get_filter_params(), [0, 1, 2], id='all_properties'),
        pytest.param(_get_filter_params(min_price=3), [1, 2], id='min_price'),
        pytest.param(_get_filter_params(max_price=4), [0, 2], id='max_price'),
        pytest.param(_get_filter_params(city='A'), [0, 1], id='city'),
    ]
)
async def test_filters_properties(service_client: pytest_userver.client.Client, params: dict[str, Any], found_property_indexes: list[int]):
    token, _ = await utils.create_user(service_client)

    creation_payloads = [
        utils.get_property_creation_payload(price=2, city='AA'),
        utils.get_property_creation_payload(price=5, city='AB'),
        utils.get_property_creation_payload(price=3, city='CD'),
    ]

    property_ids = [
        (await utils.create_property(service_client, token, payload))['id']
        for payload in creation_payloads
    ]

    response = await service_client.get('/properties', params=params)
    response.status == 200

    found_property_ids = response.json()['propertyIds']
    found_property_ids = {property_id for property_id in found_property_ids if property_id in property_ids}

    assert found_property_ids == {property_id for index, property_id in enumerate(property_ids) if index in found_property_indexes}


@pytest.mark.parametrize(
    'params',
    [
        pytest.param(_get_filter_params(min_price=-2), id='negative_min_price'),
        pytest.param(_get_filter_params(max_price=-2), id='negative_max_price'),
        pytest.param(_get_filter_params(min_price='foo'), id='not_integer_min_price'),
        pytest.param(_get_filter_params(max_price='bar'), id='not_integer_max_price'),
    ]
)
async def test_validates_price_filters(service_client: pytest_userver.client.Client, params: dict[str, Any]):
    response = await service_client.get('/properties', params=params)
    response.status == 400
