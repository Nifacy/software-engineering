import http
from typing import Any
from unittest.mock import ANY

import pytest
import zapros

from . import utils


def _get_update_payload(
    price: int | None = None,
    status: str | None = None,
) -> dict[str, Any]:
    payload: dict[str, Any] = {}

    if price is not None:
        payload["price"] = price

    if status is not None:
        payload["status"] = status

    return payload


def _get_filter_params(
    min_price: int | None = None,
    max_price: int | None = None,
    city: str | None = None,
) -> dict[str, str]:
    params: dict[str, int | str | None] = {
        "minPrice": min_price,
        "maxPrice": max_price,
        "city": city,
    }

    return {key: str(value) for key, value in params.items() if value is not None}


@pytest.mark.asyncio
async def test_creates_property(http_client: zapros.AsyncClient):
    token, user_id = await utils.create_user(http_client)
    creation_payload = utils.get_property_creation_payload()

    response = await http_client.post(
        "/api/v1/properties",
        json=creation_payload,
        headers=utils.get_auth_headers(token),
    )

    utils.assert_status(response, http.HTTPStatus.CREATED)
    await utils.assert_json(
        response,
        {
            "id": ANY,
            "address": creation_payload["address"],
            "ownerId": user_id,
            "price": creation_payload["price"],
            "status": "active",
        },
    )


@pytest.mark.asyncio
async def test_unable_to_create_if_unauthorized(http_client: zapros.AsyncClient):
    creation_payload = utils.get_property_creation_payload()
    response = await http_client.post("/api/v1/properties", json=creation_payload)
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


@pytest.mark.parametrize(
    "payload",
    [
        pytest.param({"unknown": "field"}, id="wrong_payload_structure"),
        pytest.param(
            utils.get_property_creation_payload(price=-1), id="invalid_price_value"
        ),
    ],
)
@pytest.mark.asyncio
async def test_validates_creation_payload(
    http_client: zapros.AsyncClient, payload: dict[str, Any]
):
    token, _ = await utils.create_user(http_client)
    response = await http_client.post(
        "/api/v1/properties",
        json=payload,
        headers=utils.get_auth_headers(token),
    )
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


# Get poroperty


@pytest.mark.asyncio
async def test_raises_if_property_not_exists(http_client: zapros.AsyncClient):
    response = await http_client.get("/api/v1/properties/unknown")
    utils.assert_status(response, http.HTTPStatus.NOT_FOUND)


@pytest.mark.asyncio
async def test_returns_found_property(http_client: zapros.AsyncClient):
    token, _ = await utils.create_user(http_client)
    property = await utils.create_property(http_client, token)
    property_id = property["id"]

    response = await http_client.get(f"/api/v1/properties/{property_id}")
    utils.assert_status(response, http.HTTPStatus.OK)
    await utils.assert_json(response, property)


# Update property


@pytest.mark.parametrize(
    "payload",
    [
        pytest.param(_get_update_payload(), id="nothing_change"),
        pytest.param(_get_update_payload(price=6767), id="change_price"),
        pytest.param(_get_update_payload(status="sold"), id="change_status"),
        pytest.param(
            _get_update_payload(price=6767, status="sold"), id="change_status_and_price"
        ),
    ],
)
@pytest.mark.asyncio
async def test_updates_property(
    http_client: zapros.AsyncClient, payload: dict[str, Any]
):
    token, _ = await utils.create_user(http_client)
    property = await utils.create_property(http_client, token)
    property_id = property["id"]

    response = await http_client.patch(
        f"/api/v1/properties/{property_id}",
        json=payload,
        headers=utils.get_auth_headers(token),
    )
    utils.assert_status(response, http.HTTPStatus.OK)
    await utils.assert_json(response, {**property, **payload})


@pytest.mark.asyncio
async def test_update_not_authorized(http_client: zapros.AsyncClient):
    token, _ = await utils.create_user(http_client)
    property = await utils.create_property(http_client, token)
    property_id = property["id"]

    response = await http_client.patch(
        f"/api/v1/properties/{property_id}",
        json=_get_update_payload(),
    )

    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


@pytest.mark.asyncio
async def test_raises_if_update_unknown_property(http_client: zapros.AsyncClient):
    token, _ = await utils.create_user(http_client)
    response = await http_client.patch(
        "/api/v1/properties/unknown",
        json=_get_update_payload(),
        headers=utils.get_auth_headers(token),
    )
    utils.assert_status(response, http.HTTPStatus.NOT_FOUND)


@pytest.mark.parametrize(
    "payload",
    [
        pytest.param({"unknown": "field"}, id="invalid_payload"),
        pytest.param(_get_update_payload(price=-2), id="invalid_price_value"),
        pytest.param(_get_update_payload(status="unknown"), id="invalid_status_value"),
    ],
)
@pytest.mark.asyncio
async def test_validates_update_payload(
    http_client: zapros.AsyncClient, payload: dict[str, Any]
):
    token, _ = await utils.create_user(http_client)
    property = await utils.create_property(http_client, token)
    property_id = property["id"]

    response = await http_client.patch(
        f"/api/v1/properties/{property_id}",
        json=payload,
        headers=utils.get_auth_headers(token),
    )
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


@pytest.mark.asyncio
async def test_validates_if_update_owned_property(http_client: zapros.AsyncClient):
    token1, _ = await utils.create_user(http_client)
    token2, _ = await utils.create_user(http_client)

    property = await utils.create_property(http_client, token1)
    property_id = property["id"]

    response = await http_client.patch(
        f"/api/v1/properties/{property_id}",
        json=_get_update_payload(),
        headers=utils.get_auth_headers(token2),
    )

    utils.assert_status(response, http.HTTPStatus.FORBIDDEN)


# User properties


@pytest.mark.asyncio
async def test_gets_properties_not_authorized(http_client: zapros.AsyncClient):
    response = await http_client.get("/api/v1/users/me/properties")
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


@pytest.mark.asyncio
async def test_returns_empty_property_list_by_default(http_client: zapros.AsyncClient):
    token, _ = await utils.create_user(http_client)
    response = await http_client.get(
        "/api/v1/users/me/properties",
        headers=utils.get_auth_headers(token),
    )
    utils.assert_status(response, http.HTTPStatus.OK)
    await utils.assert_json(response, {"properties": []})


@pytest.mark.asyncio
async def test_adds_created_property_to_list(http_client: zapros.AsyncClient):
    token, _ = await utils.create_user(http_client)
    property = await utils.create_property(http_client, token)

    response = await http_client.get(
        "/api/v1/users/me/properties",
        headers=utils.get_auth_headers(token),
    )

    utils.assert_status(response, http.HTTPStatus.OK)
    await utils.assert_json(
        response,
        {
            "properties": [
                {
                    "id": property["id"],
                    "address": property["address"],
                    "price": property["price"],
                    "status": property["status"],
                }
            ]
        },
    )


# Find properties


@pytest.mark.parametrize(
    "params,found_property_indexes",
    [
        pytest.param(_get_filter_params(), [0, 1, 2], id="all_properties"),
        pytest.param(_get_filter_params(min_price=3), [1, 2], id="min_price"),
        pytest.param(_get_filter_params(max_price=4), [0, 2], id="max_price"),
        pytest.param(_get_filter_params(city="A"), [0, 1], id="city"),
    ],
)
@pytest.mark.asyncio
async def test_filters_properties(
    http_client: zapros.AsyncClient,
    params: dict[str, str],
    found_property_indexes: list[int],
):
    token, _ = await utils.create_user(http_client)

    creation_payloads = [
        utils.get_property_creation_payload(price=2, city="AA"),
        utils.get_property_creation_payload(price=5, city="AB"),
        utils.get_property_creation_payload(price=3, city="CD"),
    ]

    property_ids = [
        (await utils.create_property(http_client, token, payload))["id"]
        for payload in creation_payloads
    ]

    response = await http_client.get("/api/v1/properties", params=params)
    utils.assert_status(response, http.HTTPStatus.OK)

    found_property_ids = (await utils.get_json(response))["propertyIds"]
    found_property_ids = {
        property_id for property_id in found_property_ids if property_id in property_ids
    }

    assert found_property_ids == {
        property_id
        for index, property_id in enumerate(property_ids)
        if index in found_property_indexes
    }


@pytest.mark.parametrize(
    "params",
    [
        pytest.param(_get_filter_params(min_price=-2), id="negative_min_price"),
        pytest.param(_get_filter_params(max_price=-2), id="negative_max_price"),
        pytest.param(_get_filter_params(min_price="foo"), id="not_integer_min_price"),
        pytest.param(_get_filter_params(max_price="bar"), id="not_integer_max_price"),
    ],
)
@pytest.mark.asyncio
async def test_validates_price_filters(
    http_client: zapros.AsyncClient, params: dict[str, Any]
):
    response = await http_client.get("/api/v1/properties", params=params)
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)
