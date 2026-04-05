import datetime

import pytest_userver.client
from . import utils


async def test_returns_empty_list_by_default(service_client: pytest_userver.client.Client):
    token, _ = await utils.create_user(service_client)
    property = await utils.create_property(service_client, token)
    property_id = property['id']

    response = await service_client.get(f'/api/v1/properties/{property_id}/viewings')
    assert response.status == 200
    assert response.json() == {'viewings': []}


async def test_returns_error_if_property_not_exists(service_client: pytest_userver.client.Client):
    response = await service_client.get(f'/api/v1/properties/unknown/viewings')
    assert response.status == 404


async def test_updates_list(service_client: pytest_userver.client.Client):
    token1, _ = await utils.create_user(service_client)
    token2, user_id = await utils.create_user(service_client)

    property = await utils.create_property(service_client, token1)
    property_id = property['id']

    payload = utils.get_schedule_viewing_payload(datetime.date(year=2024, month=2, day=20))
    viewing_id = await utils.schedule_viewing(service_client, token2, property_id, payload)

    response = await service_client.get(f'/api/v1/properties/{property_id}/viewings')
    assert response.status == 200
    assert response.json() == {'viewings': [{'id': viewing_id, 'user_id': user_id, 'date': payload['date']}]}

    await utils.delete_viewing(service_client, token2, property_id, viewing_id)

    response = await service_client.get(f'/api/v1/properties/{property_id}/viewings')
    assert response.status == 200
    assert response.json() == {'viewings': []}


async def test_filters_viewings(service_client: pytest_userver.client.Client):
    token1, _ = await utils.create_user(service_client)
    token2, _ = await utils.create_user(service_client)

    property1 = await utils.create_property(service_client, token1)
    property1_id = property1['id']

    property2 = await utils.create_property(service_client, token1)
    property2_id = property2['id']

    viewing_id = await utils.schedule_viewing(service_client, token2, property1_id)

    await utils.schedule_viewing(service_client, token2, property2_id)

    response = await service_client.get(f'/api/v1/properties/{property1_id}/viewings')
    assert response.status == 200

    property_viewing_ids = {viewing['id'] for viewing in response.json()['viewings']}
    assert property_viewing_ids == {viewing_id}
