import contextlib
import dataclasses
import time
from typing import Any, Iterator
import urllib.parse
import _logging
import zapros


@dataclasses.dataclass(frozen=True, slots=True, kw_only=True)
class UserCredentials:
    login: str
    password: str
    first_name: str
    last_name: str


@dataclasses.dataclass(frozen=True, slots=True, kw_only=True)
class Address:
    country: str
    city: str
    street: str
    building: int


@dataclasses.dataclass(frozen=True, slots=True, kw_only=True)
class Property:
    address: Address
    price: int


@dataclasses.dataclass(frozen=True, slots=True, kw_only=True)
class SearchParams:
    city: str | None
    min_price: int | None
    max_price: int | None


class ApiClient:
    def __init__(self, url: str, client: zapros.AsyncClient):
        self.__client = client
        self.__url = url
        self.__logger = _logging.get_logger("ApiClient")

    async def register_user(self, creds: UserCredentials) -> str:
        with self.__log_action("Register user"):
            response = await self.__client.post(
                urllib.parse.urljoin(self.__url, "api/v1/auth/register"),
                json={
                    "login": creds.login,
                    "password": creds.password,
                    "firstName": creds.first_name,
                    "lastName": creds.last_name,
                }
            )

            response.raise_for_status()
            return response.json["accessToken"]

    async def login(self, login: str, password: str) -> str:
        response = await self.__client.post(
            urllib.parse.urljoin(self.__url, "api/v1/auth/login"),
            json={
                "login": login,
                "password": password,
            }
        )

        response.raise_for_status()
        return response.json["accessToken"]

    async def create_property(self, token: str, property: Property) -> str:
        with self.__log_action("Create property"):
            response = await self.__client.post(
                urllib.parse.urljoin(self.__url, "api/v1/properties"),
                headers={
                    "Authorization": f"Bearer {token}",
                },
                json={
                    "address": {
                        "country": property.address.country,
                        "city": property.address.city,
                        "street": property.address.street,
                        "building": property.address.building,
                    },
                    "price": property.price,
                },
            )

            response.raise_for_status()
            return response.json["id"]

    async def find_properties(self, search_params: SearchParams) -> list[str]:
        with self.__log_action("Find properties"):
            params: dict[str, str] = {}
            if search_params.city is not None:
                params["city"] = search_params.city
            if search_params.min_price is not None:
                params["minPrice"] = str(search_params.min_price)
            if search_params.max_price is not None:
                params["maxPrice"] = str(search_params.max_price)

            response = await self.__client.get(
                urllib.parse.urljoin(self.__url, "api/v1/properties"),
                params=params,
            )

            response.raise_for_status()
            return response.json["propertyIds"]

    async def schedule_viewing(self, token: str, property_id: str, date: str) -> str:
        with self.__log_action("Schedule viewing"):
            response = await self.__client.post(
                urllib.parse.urljoin(self.__url, f"api/v1/properties/{property_id}/viewings"),
                headers={"Authorization": f"Bearer {token}"},
                json={"date": date},
            )

            response.raise_for_status()
            return response.json["id"]

    async def find_property_viewings(self, property_id: str) -> list[Any]:
        with self.__log_action("Find property viewings"):
            response = await self.__client.get(
                urllib.parse.urljoin(self.__url, f"api/v1/properties/{property_id}/viewings"),
            )

            response.raise_for_status()
            return response.json

    @contextlib.contextmanager
    def __log_action(self, action: str) -> Iterator[None]:
        start_time = time.time()

        self.__logger.debug("{action} ...", action=action)

        try:
            yield
        finally:
            finish_time = time.time()
            self.__logger.debug("{action} ... ok ({delta:.2f}s)", action=action, delta=finish_time - start_time)
