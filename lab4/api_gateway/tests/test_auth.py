import pytest_userver.client
from . import utils


# TODO: Use constants instead of hard coded values
async def test_can_login_after_registration(service_client: pytest_userver.client.Client):
    payload = utils.get_register_payload()

    response = await service_client.post('/api/v1/auth/register', json=payload)
    assert response.status == 201

    response = await service_client.post('/api/v1/auth/login', json={'login': payload['login'], 'password': payload['password']})
    assert response.status == 200


async def test_unable_to_register_if_login_already_taken(service_client: pytest_userver.client.Client):
    payload = utils.get_register_payload()
    await utils.create_user(service_client, payload=payload)

    response = await service_client.post('/api/v1/auth/register', json=payload)
    assert response.status == 409
    assert response.content.decode() == "User already exists"


async def test_validates_registration_payload(service_client: pytest_userver.client.Client):
    response = await service_client.post('/api/v1/auth/register', json={'unknown': 'field'})
    assert response.status == 400
    assert response.content.decode() == "Invalid Payload"


async def test_validates_login_payload(service_client: pytest_userver.client.Client):
    response = await service_client.post('/api/v1/auth/login', json={'unknown': 'field'})
    assert response.status == 400
    assert response.content.decode() == "Invalid Payload"



async def test_validates_payload_on_login(service_client: pytest_userver.client.Client):
    response = await service_client.post('/api/v1/auth/login', json={'credentials': 'unknown'})
    assert response.status == 400
    assert response.content.decode() == "Invalid Payload"


async def test_validates_password_on_login(service_client: pytest_userver.client.Client):
    payload = utils.get_register_payload()
    await utils.create_user(service_client, payload=payload)
    response = await service_client.post('/api/v1/auth/login', json={'login': payload['login'], 'password': 'unknown'})
    assert response.status == 401
    assert response.content.decode() == "Bad credentials"


async def test_unable_to_login_if_user_not_exists(service_client: pytest_userver.client.Client):
    response = await service_client.post('/api/v1/auth/login', json={'login': 'unknown', 'password': 'unknown'})
    assert response.status == 400
    assert response.content.decode() == "User with specified login not found"
