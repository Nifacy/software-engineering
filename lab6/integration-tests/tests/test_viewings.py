import datetime
import http
from unittest.mock import ANY

import pytest
import zapros

from . import utils


async def _create_property(
    http_client: zapros.AsyncClient,
    status: utils.PropertyStatus | None = None,
) -> str:
    token, _ = await utils.create_user(http_client)
    property = await utils.create_property(http_client, token)

    if status is not None:
        await utils.update_property(
            http_client,
            token,
            property["id"],
            utils.get_property_update_payload(status),
        )

    return property["id"]


@pytest.mark.asyncio
async def test_schedules_viewing(http_client: zapros.AsyncClient):
    property_id = await _create_property(http_client)
    token, user_id = await utils.create_user(http_client)
    payload = utils.get_schedule_viewing_payload(
        datetime.date(year=2025, month=10, day=30)
    )

    response = await http_client.post(
        f"/api/v1/properties/{property_id}/viewings",
        headers=utils.get_auth_headers(token),
        json=payload,
    )

    utils.assert_status(response, http.HTTPStatus.CREATED)
    await utils.assert_json(
        response,
        {
            "id": ANY,
            "user_id": user_id,
            "date": payload["date"],
        },
    )


@pytest.mark.asyncio
async def test_unable_to_schedule_without_authorization(
    http_client: zapros.AsyncClient,
):
    property_id = await _create_property(http_client)
    payload = utils.get_schedule_viewing_payload(
        datetime.date(year=2025, month=10, day=30)
    )

    response = await http_client.post(
        f"/api/v1/properties/{property_id}/viewings",
        json=payload,
    )
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


@pytest.mark.asyncio
async def test_unable_to_schedule_viewing_for_unknown_property(
    http_client: zapros.AsyncClient,
):
    token, _ = await utils.create_user(http_client)
    payload = utils.get_schedule_viewing_payload(
        datetime.date(year=2025, month=10, day=30)
    )

    response = await http_client.post(
        "/api/v1/properties/unknown/viewings",
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    utils.assert_status(response, http.HTTPStatus.NOT_FOUND)


@pytest.mark.asyncio
async def test_not_schedules_viewing_on_the_same_date(http_client: zapros.AsyncClient):
    property_id = await _create_property(http_client)
    token, _ = await utils.create_user(http_client)
    payload = utils.get_schedule_viewing_payload(
        datetime.date(year=2025, month=10, day=30)
    )

    response = await http_client.post(
        f"/api/v1/properties/{property_id}/viewings",
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    utils.assert_status(response, http.HTTPStatus.CREATED)

    response = await http_client.post(
        f"/api/v1/properties/{property_id}/viewings",
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    utils.assert_status(response, http.HTTPStatus.NOT_MODIFIED)


@pytest.mark.asyncio
async def test_unable_to_schedule_on_the_same_date(http_client: zapros.AsyncClient):
    property_id = await _create_property(http_client)
    token1, _ = await utils.create_user(http_client)
    token2, _ = await utils.create_user(http_client)
    payload = utils.get_schedule_viewing_payload(
        datetime.date(year=2025, month=10, day=30)
    )

    response = await http_client.post(
        f"/api/v1/properties/{property_id}/viewings",
        headers=utils.get_auth_headers(token1),
        json=payload,
    )
    utils.assert_status(response, http.HTTPStatus.CREATED)

    response = await http_client.post(
        f"/api/v1/properties/{property_id}/viewings",
        headers=utils.get_auth_headers(token2),
        json=payload,
    )
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


@pytest.mark.asyncio
async def test_unable_to_schedule_if_property_sold(http_client: zapros.AsyncClient):
    property_id = await _create_property(http_client, status="sold")
    token, _ = await utils.create_user(http_client)
    payload = utils.get_schedule_viewing_payload(
        datetime.date(year=2025, month=10, day=30)
    )

    response = await http_client.post(
        f"/api/v1/properties/{property_id}/viewings",
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


@pytest.mark.parametrize(
    "payload",
    [
        pytest.param({"unknown": "field"}, id="invalid_payload"),
        pytest.param({"date": "foo"}, id="invalid_date_format"),
    ],
)
@pytest.mark.asyncio
async def test_validates_viewing_payload(
    http_client: zapros.AsyncClient, payload: dict[str, str]
):
    property_id = await _create_property(http_client)
    token, _ = await utils.create_user(http_client)

    response = await http_client.post(
        f"/api/v1/properties/{property_id}/viewings",
        headers=utils.get_auth_headers(token),
        json=payload,
    )
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)


# Delete viewing


@pytest.mark.asyncio
async def test_deletes_viewing(http_client: zapros.AsyncClient):
    property_id = await _create_property(http_client)
    token, _ = await utils.create_user(http_client)
    viewing_id = await utils.schedule_viewing(http_client, token, property_id)

    response = await http_client.delete(
        f"/api/v1/properties/{property_id}/viewings/{viewing_id}",
        headers=utils.get_auth_headers(token),
    )
    utils.assert_status(response, http.HTTPStatus.NO_CONTENT)


@pytest.mark.asyncio
async def test_unable_to_delete_not_scheduled_viewing(http_client: zapros.AsyncClient):
    property_id = await _create_property(http_client)
    token, _ = await utils.create_user(http_client)

    response = await http_client.delete(
        f"/api/v1/properties/{property_id}/viewings/unknown",
        headers=utils.get_auth_headers(token),
    )
    utils.assert_status(response, http.HTTPStatus.NOT_FOUND)


@pytest.mark.asyncio
async def test_can_schedule_viewing_on_the_same_date_after_delete(
    http_client: zapros.AsyncClient,
):
    property_id = await _create_property(http_client)
    token, _ = await utils.create_user(http_client)
    payload = utils.get_schedule_viewing_payload(
        datetime.date(year=2024, month=2, day=20)
    )
    viewing_id = await utils.schedule_viewing(http_client, token, property_id, payload)

    response = await http_client.delete(
        f"/api/v1/properties/{property_id}/viewings/{viewing_id}",
        headers=utils.get_auth_headers(token),
    )
    utils.assert_status(response, http.HTTPStatus.NO_CONTENT)
    await utils.schedule_viewing(http_client, token, property_id, payload)


@pytest.mark.asyncio
async def test_unable_to_delete_viewing_of_other_user(http_client: zapros.AsyncClient):
    property_id = await _create_property(http_client)
    token1, _ = await utils.create_user(http_client)
    token2, _ = await utils.create_user(http_client)
    viewing_id = await utils.schedule_viewing(http_client, token1, property_id)

    response = await http_client.delete(
        f"/api/v1/properties/{property_id}/viewings/{viewing_id}",
        headers=utils.get_auth_headers(token2),
    )
    utils.assert_status(response, http.HTTPStatus.FORBIDDEN)
