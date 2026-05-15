import datetime
from typing import Final

import _client
import uuid
import random


CITIES: Final = (
    "Moscow",
    "Berlin",
    "Paris",
    "Minsk",
)


def get_random_user_creds() -> _client.UserCredentials:
    return _client.UserCredentials(
        login=str(uuid.uuid4()),
        password=str(uuid.uuid4()),
        first_name="Test",
        last_name="User",
    )


def get_random_address() -> _client.Address:
    return _client.Address(
        country="Example",
        city=random.choice(CITIES),
        street="Example",
        building=12,
    )


def get_random_property() -> _client.Property:
    return _client.Property(
        address=get_random_address(),
        price=random.randint(1, 500),
    )


def get_random_city_substr() -> str:
    city = random.choice(CITIES)
    index = random.randint(0, len(city))
    finish = random.randint(index, len(city)) + 1
    return city[index:finish]


def get_random_search_params() -> _client.SearchParams:
    return _client.SearchParams(
        city=random.choice((None, get_random_city_substr())),
        min_price=random.choice((None, random.randint(0, 500))),
        max_price=random.choice((None, random.randint(0, 500))),
    )


def get_random_date() -> str:
    start_date = datetime.date(year=2000, month=1, day=1)
    days = random.randint(1, 10000)
    return (start_date + datetime.timedelta(days=days)).strftime("%Y-%m-%d")
