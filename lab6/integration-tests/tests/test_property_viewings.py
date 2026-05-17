import datetime
import http

import pytest
import zapros

from . import utils


@pytest.mark.asyncio
async def test_returns_empty_list_by_default(http_client: zapros.AsyncClient):
    token, _ = await utils.create_user(http_client)
    property = await utils.create_property(http_client, token)
    property_id = property["id"]

    response = await http_client.get(f"/api/v1/properties/{property_id}/viewings")
    assert response.status == http.HTTPStatus.OK
    await utils.assert_json(response, {"viewings": []})


@pytest.mark.asyncio
async def test_returns_error_if_property_not_exists(http_client: zapros.AsyncClient):
    response = await http_client.get("/api/v1/properties/unknown/viewings")
    assert response.status == http.HTTPStatus.NOT_FOUND


@pytest.mark.asyncio
async def test_updates_list(http_client: zapros.AsyncClient):
    token1, _ = await utils.create_user(http_client)
    token2, user_id = await utils.create_user(http_client)

    property = await utils.create_property(http_client, token1)
    property_id = property["id"]

    payload = utils.get_schedule_viewing_payload(
        datetime.date(year=2024, month=2, day=20)
    )
    viewing_id = await utils.schedule_viewing(http_client, token2, property_id, payload)

    response = await http_client.get(f"/api/v1/properties/{property_id}/viewings")
    assert response.status == http.HTTPStatus.OK
    await utils.assert_json(
        response,
        {"viewings": [{"id": viewing_id, "user_id": user_id, "date": payload["date"]}]},
    )

    await utils.delete_viewing(http_client, token2, property_id, viewing_id)

    response = await http_client.get(f"/api/v1/properties/{property_id}/viewings")
    assert response.status == http.HTTPStatus.OK
    await utils.assert_json(response, {"viewings": []})


@pytest.mark.asyncio
async def test_filters_viewings(http_client: zapros.AsyncClient):
    token1, _ = await utils.create_user(http_client)
    token2, _ = await utils.create_user(http_client)

    property1 = await utils.create_property(http_client, token1)
    property1_id = property1["id"]

    property2 = await utils.create_property(http_client, token1)
    property2_id = property2["id"]

    viewing_id = await utils.schedule_viewing(http_client, token2, property1_id)

    await utils.schedule_viewing(http_client, token2, property2_id)

    response = await http_client.get(f"/api/v1/properties/{property1_id}/viewings")
    assert response.status == http.HTTPStatus.OK

    property_viewing_ids = {
        viewing["id"] for viewing in (await utils.get_json(response))["viewings"]
    }
    assert property_viewing_ids == {viewing_id}
