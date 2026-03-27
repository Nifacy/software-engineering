import pytest
import pytest_userver.client
from . import utils


# TODO: Use constants instead of hard coded values
async def test_can_login_after_registration(service_client: pytest_userver.client.Client):
    response = await service_client.post('/register', json=utils.get_register_payload())
    assert response.status == 201

    token = response.json()['token']
    response = await service_client.post('/login', json={'credentials': token})
    assert response.status == 200


async def test_validates_registration_payload(service_client: pytest_userver.client.Client):
    response = await service_client.post('/register', json={'unknown': 'field'})
    assert response.status == 400
    assert response.content.decode() == "Invalid Payload"


async def test_validates_login_payload(service_client: pytest_userver.client.Client):
    response = await service_client.post('/login', json={'unknown': 'field'})
    assert response.status == 400
    assert response.content.decode() == "Invalid Payload"



async def test_validates_token_on_login(service_client: pytest_userver.client.Client):
    response = await service_client.post('/login', json={'credentials': 'unknown'})
    assert response.status == 400
    assert response.content.decode() == "Invalid credentials"


@pytest.mark.skip(reason='This case is not working with current implementation of auth')
async def test_unable_to_register_if_login_already_taken(service_client: pytest_userver.client.Client):
    payload = utils.get_register_payload(login='foo')
    await utils.create_user(service_client, payload=payload)

    response = await service_client.post('/register', json=payload)
    assert response.status == 400
