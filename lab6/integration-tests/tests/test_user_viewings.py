import http

import pytest
import zapros

from . import utils


@pytest.mark.asyncio
async def test_returns_empty_list_by_default(http_client: zapros.AsyncClient):
    token, _ = await utils.create_user(http_client)
    response = await http_client.get(
        "/api/v1/users/me/viewings",
        headers=utils.get_auth_headers(token),
    )
    utils.assert_status(response, http.HTTPStatus.OK)
    await utils.assert_json(response, {"viewings": []})


@pytest.mark.asyncio
async def test_updates_list(http_client: zapros.AsyncClient):
    token, _ = await utils.create_user(http_client)
    property = await utils.create_property(http_client)
    payload = utils.get_schedule_viewing_payload()
    viewing_id = await utils.schedule_viewing(
        http_client, token, property["id"], payload
    )

    response = await http_client.get(
        "/api/v1/users/me/viewings",
        headers=utils.get_auth_headers(token),
    )

    utils.assert_status(response, http.HTTPStatus.OK)
    await utils.assert_json(
        response,
        {
            "viewings": [
                {
                    "propertyId": property["id"],
                    "id": viewing_id,
                    "date": payload["date"],
                }
            ]
        },
    )

    await utils.delete_viewing(http_client, token, property["id"], viewing_id)
    response = await http_client.get(
        "/api/v1/users/me/viewings",
        headers=utils.get_auth_headers(token),
    )

    utils.assert_status(response, http.HTTPStatus.OK)
    await utils.assert_json(response, {"viewings": []})


@pytest.mark.asyncio
async def test_filters_viewings(http_client: zapros.AsyncClient):
    property = await utils.create_property(http_client)

    token1, _ = await utils.create_user(http_client)
    viewing_id = await utils.schedule_viewing(http_client, token1, property["id"])

    token2, _ = await utils.create_user(http_client)
    await utils.schedule_viewing(http_client, token2, property["id"])

    response = await http_client.get(
        "/api/v1/users/me/viewings",
        headers=utils.get_auth_headers(token1),
    )
    utils.assert_status(response, http.HTTPStatus.OK)

    viewing_ids = {
        viewing["id"] for viewing in (await utils.get_json(response))["viewings"]
    }
    assert viewing_ids == {viewing_id}


@pytest.mark.asyncio
async def test_raises_if_not_authorized(http_client: zapros.AsyncClient):
    response = await http_client.get("/api/v1/users/me/viewings")
    utils.assert_status(response, http.HTTPStatus.BAD_REQUEST)
