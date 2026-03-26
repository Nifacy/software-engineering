import pytest_userver.client


async def test_can_login_after_registration(service_client: pytest_userver.client.Client):
    response = await service_client.post('/register', json={'login': 'foo', 'password': 'foo', 'firstName': 'Bob', 'lastName': 'Smith'})
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
