import http
from unittest.mock import ANY

import pytest
import zapros

from . import utils


def _get_filter_params(
    login: str | None = None,
    first_name: str | None = None,
    last_name: str | None = None,
) -> dict[str, str]:
    filters = {"login": login, "firstName": first_name, "lastName": last_name}
    return {key: value for key, value in filters.items() if value is not None}


# Get current user


@pytest.mark.asyncio
async def test_returns_current_user(http_client: zapros.AsyncClient):
    payload = utils.get_register_payload()
    token = await utils.register(http_client, payload)

    response = await http_client.get(
        "/api/v1/users/me",
        headers=utils.get_auth_headers(token),
    )

    utils.assert_status(response, http.HTTPStatus.OK)
    await utils.assert_json(
        response,
        {
            "id": ANY,
            "login": payload["login"],
            "firstName": payload["firstName"],
            "lastName": payload["lastName"],
        },
    )


@pytest.mark.asyncio
async def test_returns_current_user_not_authorized(http_client: zapros.AsyncClient):
    response = await http_client.get("/api/v1/users/me")
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


# Get user


@pytest.mark.asyncio
async def test_returns_user_by_id(http_client: zapros.AsyncClient):
    payload = utils.get_register_payload()
    _, user_id = await utils.create_user(http_client, payload)

    response = await http_client.get(f"/api/v1/users/{user_id}")
    utils.assert_status(response, http.HTTPStatus.OK)
    await utils.assert_json(
        response,
        {
            "id": user_id,
            "login": payload["login"],
            "firstName": payload["firstName"],
            "lastName": payload["lastName"],
        },
    )


@pytest.mark.asyncio
async def test_gets_unknown_user(http_client: zapros.AsyncClient):
    response = await http_client.get("/api/v1/users/unknown")
    utils.assert_status(response, http.HTTPStatus.NOT_FOUND)


# Find users


@pytest.mark.parametrize(
    "filters,found_user_indexes",
    [
        pytest.param(_get_filter_params(), [0, 1, 2], id="all_users"),
        pytest.param(_get_filter_params(login="foo"), [0], id="filter_by_login"),
        pytest.param(
            _get_filter_params(first_name="A"), [1], id="filter_by_first_name"
        ),
        pytest.param(_get_filter_params(last_name="B"), [2], id="filter_by_last_name"),
    ],
)
@pytest.mark.asyncio
async def test_finds_users(
    http_client: zapros.AsyncClient,
    filters: dict[str, str],
    found_user_indexes: list[int],
):
    register_payloads = [
        utils.get_register_payload(
            login=utils.get_random_login("foo"), first_name="C", last_name="D"
        ),
        utils.get_register_payload(
            login=utils.get_random_login("bar"), first_name="A", last_name="D"
        ),
        utils.get_register_payload(
            login=utils.get_random_login("buzz"), first_name="C", last_name="B"
        ),
    ]

    user_ids = [
        (await utils.create_user(http_client, payload))[1]
        for payload in register_payloads
    ]

    response = await http_client.get("/api/v1/users", params=filters)
    utils.assert_status(response, http.HTTPStatus.OK)

    found_user_ids = {
        user_id
        for user_id in (await utils.get_json(response))["userIds"]
        if user_id in user_ids
    }
    expected_user_ids = {user_ids[index] for index in found_user_indexes}
    assert found_user_ids == expected_user_ids
