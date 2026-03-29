import pytest
from unittest.mock import ANY
import datetime
import pytest_userver.client
from . import utils


async def _create_property(service_client: pytest_userver.client.Client, status: utils.PropertyStatus | None = None) -> str:
    token, _ = await utils.create_user(service_client)
    property = await utils.create_property(service_client, token)

    if status is not None:
        await utils.update_property(
            service_client,
            token,
            property['id'],
            utils.get_property_update_payload(status),
        )

    return property['id']


async def test_schedules_viewing(service_client: pytest_userver.client.Client):
    property_id = await _create_property(service_client)
    token, user_id = await utils.create_user(service_client)
    payload = utils.get_schedule_viewing_payload(datetime.date(year=2025, month=10, day=30))

    response = await service_client.post(
        f'/properties/{property_id}/viewings',
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    assert response.status == 201
    assert response.json() == {
        'id': ANY,
        'user_id': user_id,
        'date': payload['date'],
    }


async def test_unable_to_schedule_without_authorization(service_client: pytest_userver.client.Client):
    property_id = await _create_property(service_client)
    payload = utils.get_schedule_viewing_payload(datetime.date(year=2025, month=10, day=30))

    response = await service_client.post(f'/properties/{property_id}/viewings', json=payload)
    assert response.status == 400


async def test_unable_to_schedule_viewing_for_unknown_property(service_client: pytest_userver.client.Client):
    token, _ = await utils.create_user(service_client)
    payload = utils.get_schedule_viewing_payload(datetime.date(year=2025, month=10, day=30))

    response = await service_client.post(
        '/properties/unknown/viewings',
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    assert response.status == 404


async def test_not_schedules_viewing_on_the_same_date(service_client: pytest_userver.client.Client):
    property_id = await _create_property(service_client)
    token, _ = await utils.create_user(service_client)
    payload = utils.get_schedule_viewing_payload(datetime.date(year=2025, month=10, day=30))

    response = await service_client.post(
        f'/properties/{property_id}/viewings',
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    assert response.status == 201

    response = await service_client.post(
        f'/properties/{property_id}/viewings',
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    assert response.status == 304


async def test_unable_to_schedule_on_the_same_date(service_client: pytest_userver.client.Client):
    property_id = await _create_property(service_client)
    token1, _ = await utils.create_user(service_client)
    token2, _ = await utils.create_user(service_client)
    payload = utils.get_schedule_viewing_payload(datetime.date(year=2025, month=10, day=30))

    response = await service_client.post(
        f'/properties/{property_id}/viewings',
        headers=utils.get_auth_headers(token1),
        json=payload,
    )
    assert response.status == 201

    response = await service_client.post(
        f'/properties/{property_id}/viewings',
        headers=utils.get_auth_headers(token2),
        json=payload,
    )
    assert response.status == 400



async def test_unable_to_schedule_if_property_sold(service_client: pytest_userver.client.Client):
    property_id = await _create_property(service_client, status='sold')
    token, _ = await utils.create_user(service_client)
    payload = utils.get_schedule_viewing_payload(datetime.date(year=2025, month=10, day=30))

    response = await service_client.post(
        f'/properties/{property_id}/viewings',
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    assert response.status == 400


@pytest.mark.parametrize(
    'payload',
    [
        pytest.param({'unknown': 'field'}, id='invalid_payload'),
        pytest.param({'date': 'foo'}, id='invalid_date_format'),
    ]
)
async def test_validates_viewing_payload(service_client: pytest_userver.client.Client, payload: dict[str, str]):
    property_id = await _create_property(service_client)
    token, _ = await utils.create_user(service_client)

    response = await service_client.post(
        f'/properties/{property_id}/viewings',
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    assert response.status == 500


# Delete viewing


async def test_deletes_viewing(service_client: pytest_userver.client.Client):
    property_id = await _create_property(service_client)
    token, _ = await utils.create_user(service_client)
    viewing_id = await utils.schedule_viewing(service_client, token, property_id)

    response = await service_client.delete(
        f'/properties/{property_id}/viewings/{viewing_id}',
        headers=utils.get_auth_headers(token),
    )
    assert response.status == 204


async def test_unable_to_delete_not_scheduled_viewing(service_client: pytest_userver.client.Client):
    property_id = await _create_property(service_client)
    token, _ = await utils.create_user(service_client)

    response = await service_client.delete(
        f'/properties/{property_id}/viewings/unknown',
        headers=utils.get_auth_headers(token),
    )
    assert response.status == 404


async def test_can_schedule_viewing_on_the_same_date_after_delete(service_client: pytest_userver.client.Client):
    property_id = await _create_property(service_client)
    token, _ = await utils.create_user(service_client)
    payload = utils.get_schedule_viewing_payload(datetime.date(year=2024, month=2, day=20))
    viewing_id = await utils.schedule_viewing(service_client, token, property_id, payload)

    response = await service_client.delete(
        f'/properties/{property_id}/viewings/{viewing_id}',
        headers=utils.get_auth_headers(token),
    )
    assert response.status == 204

    await utils.schedule_viewing(service_client, token, property_id, payload)


async def test_unable_to_delete_viewing_of_other_user(service_client: pytest_userver.client.Client):
    property_id = await _create_property(service_client)
    token1, _ = await utils.create_user(service_client)
    token2, _ = await utils.create_user(service_client)
    viewing_id = await utils.schedule_viewing(service_client, token1, property_id)

    response = await service_client.delete(
        f'/properties/{property_id}/viewings/{viewing_id}',
        headers=utils.get_auth_headers(token2),
    )
    assert response.status == 403
