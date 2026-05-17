import http

import pytest
import zapros

from . import utils


@pytest.mark.asyncio
async def test_can_login_after_registration(http_client: zapros.AsyncClient):
    payload = utils.get_register_payload()

    response = await http_client.post("/api/v1/auth/register", json=payload)
    assert response.status == http.HTTPStatus.CREATED

    response = await http_client.post(
        "/api/v1/auth/login",
        json={
            "login": payload["login"],
            "password": payload["password"],
        },
    )
    assert response.status == http.HTTPStatus.OK


@pytest.mark.asyncio
async def test_unable_to_register_if_login_already_taken(
    http_client: zapros.AsyncClient,
):
    payload = utils.get_register_payload()
    await utils.create_user(http_client, payload=payload)

    response = await http_client.post("/api/v1/auth/register", json=payload)
    assert response.status == http.HTTPStatus.CONFLICT
    await utils.assert_content(response, "User already exists")


@pytest.mark.asyncio
async def test_validates_registration_payload(
    http_client: zapros.AsyncClient,
):
    response = await http_client.post(
        "/api/v1/auth/register", json={"unknown": "field"}
    )
    assert response.status == http.HTTPStatus.BAD_REQUEST
    await utils.assert_content(response, "Invalid Payload")


@pytest.mark.asyncio
async def test_validates_login_payload(http_client: zapros.AsyncClient):
    response = await http_client.post("/api/v1/auth/login", json={"unknown": "field"})
    assert response.status == http.HTTPStatus.BAD_REQUEST
    await utils.assert_content(response, "Invalid Payload")


@pytest.mark.asyncio
async def test_validates_payload_on_login(http_client: zapros.AsyncClient):
    response = await http_client.post(
        "/api/v1/auth/login", json={"credentials": "unknown"}
    )
    assert response.status == http.HTTPStatus.BAD_REQUEST
    await utils.assert_content(response, "Invalid Payload")


@pytest.mark.asyncio
async def test_validates_password_on_login(
    http_client: zapros.AsyncClient,
):
    payload = utils.get_register_payload()
    await utils.create_user(http_client, payload=payload)
    response = await http_client.post(
        "/api/v1/auth/login",
        json={"login": payload["login"], "password": "unknown"},
    )
    assert response.status == http.HTTPStatus.UNAUTHORIZED
    await utils.assert_content(response, "Bad credentials")


@pytest.mark.asyncio
async def test_unable_to_login_if_user_not_exists(
    http_client: zapros.AsyncClient,
):
    response = await http_client.post(
        "/api/v1/auth/login",
        json={"login": "unknown", "password": "unknown"},
    )
    assert response.status == http.HTTPStatus.BAD_REQUEST
    await utils.assert_content(response, "User with specified login not found")
