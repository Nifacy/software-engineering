import pytest_userver.client
from . import utils


async def test_returns_empty_list_by_default(service_client: pytest_userver.client.Client):
    token, _ = await utils.create_user(service_client)
    response = await service_client.get('/api/v1/users/me/viewings', headers=utils.get_auth_headers(token))
    assert response.status == 200
    assert response.json() == {'viewings': []}


async def test_updates_list(service_client: pytest_userver.client.Client):
    token, _ = await utils.create_user(service_client)
    property = await utils.create_property(service_client)
    payload = utils.get_schedule_viewing_payload()
    viewing_id = await utils.schedule_viewing(service_client, token, property['id'], payload)

    response = await service_client.get('/api/v1/users/me/viewings', headers=utils.get_auth_headers(token))
    assert response.status == 200
    assert response.json() == {'viewings': [{'propertyId': property['id'], 'id': viewing_id, 'date': payload['date']}]}

    await utils.delete_viewing(service_client, token, property['id'], viewing_id)
    response = await service_client.get('/api/v1/users/me/viewings', headers=utils.get_auth_headers(token))
    assert response.status == 200
    assert response.json() == {'viewings': []}


async def test_filters_viewings(service_client: pytest_userver.client.Client):
    property = await utils.create_property(service_client)

    token1, _ = await utils.create_user(service_client)
    viewing_id = await utils.schedule_viewing(service_client, token1, property['id'])

    token2, _ = await utils.create_user(service_client)
    await utils.schedule_viewing(service_client, token2, property['id'])

    response = await service_client.get('/api/v1/users/me/viewings', headers=utils.get_auth_headers(token1))
    assert response.status == 200

    viewing_ids = {viewing['id'] for viewing in response.json()['viewings']}
    assert viewing_ids == {viewing_id}



async def test_raises_if_not_authorized(service_client: pytest_userver.client.Client):
    response = await service_client.get('/api/v1/users/me/viewings')
    assert response.status == 400
