from dataclasses import dataclass
import datetime
import enum
import random
import string
from typing import Final
import psycopg2
import uuid
import psycopg2.extras


PREFIXES: Final = ('foo', 'bar', 'buzz')
FIRST_NAMES: Final = ('Alexey', 'Maria', 'Dmitry', 'Elena', 'Ivan', 'Anna', 'Sergey', 'Olga', 'Nikita', 'Victoria')
LAST_NAMES: Final = ('Grishin', 'Petrova', 'Sokolov', 'Kuznetsoiva', 'Smirnov', 'Volkova', 'Popov', 'Fedorov', 'Orlova')
COUNTRIES: Final = ('Russia', 'Germany', 'France', 'Armenia', 'Belarus', 'Kazahstan')
CITIES: Final = (
    'druihginia',
    'klummont',
    'flothull',
    'stashire',
    'chehville',
    'klock',
    'grul',
    'obririe',
    'inevine',
    'encenard',
)


class PropertyStatus(enum.StrEnum):
    ACTIVE = "active"
    SOLD = "sold"


@dataclass
class User:
    id: uuid.UUID
    login: str
    first_name: str
    last_name: str


@dataclass
class Credentials:
    key: str
    secret: str
    user_id: uuid.UUID


@dataclass
class Address:
    id: uuid.UUID
    country: str
    city: str
    street: str
    building: int
    apartment: int | None = None


@dataclass
class Property:
    id: uuid.UUID
    owner_id: uuid.UUID
    address: Address
    status: PropertyStatus
    price: int


@dataclass
class Viewing:
    id: uuid.UUID
    user_id: uuid.UUID
    property_id: uuid.UUID
    date: datetime.date


def generate_user() -> User:
    random_id = uuid.uuid4()
    random_prefix = random.choice(PREFIXES)

    return User(
        id=random_id,
        login=f'user_{random_prefix}_{random_id}',
        first_name=random.choice(FIRST_NAMES),
        last_name=random.choice(LAST_NAMES),
    )


def get_random_string(len: int = 8) -> str:
    return ''.join(random.choice(string.ascii_letters) for _ in range(len))


def generate_credentials(user: User) -> Credentials:
    return Credentials(
        key=user.login,
        secret=str(uuid.uuid4()),
        user_id=user.id,
    )


def generate_address() -> Address:
    return Address(
        id=uuid.uuid4(),
        country=random.choice(COUNTRIES),
        city=random.choice(CITIES),
        street=get_random_string(),
        building=random.randint(10, 300),
        apartment=random.choice((random.randint(10, 300), None)),
    )


def generate_property(owner: User) -> Property:
    return Property(
        id=uuid.uuid4(),
        owner_id=owner.id,
        address=generate_address(),
        status=random.choice((PropertyStatus.ACTIVE, PropertyStatus.SOLD)),
        price=random.randint(200, 10_000),
    )


def generate_viewing(users: list[User], properties: list[Property]) -> Viewing:
    start_date = datetime.date(year=2024, month=1, day=1)
    view_date = start_date + datetime.timedelta(days=random.randint(1, 365 * 200))

    return Viewing(
        id=uuid.uuid4(),
        user_id=random.choice(users).id,
        property_id=random.choice(properties).id,
        date=view_date,
    )


with psycopg2.connect('dbname=postgres user=postgres password=postgres host=localhost port=5433') as conn:
    SIZE = 40_000
    users = [generate_user() for _ in range(SIZE)]
    credentials = [generate_credentials(user) for user in users]
    properties = [generate_property(user) for user in users]
    viewings = [generate_viewing(users, properties) for _ in range(SIZE)]

    with conn.cursor() as cursor:
        psycopg2.extras.execute_values(
            cursor,
            "INSERT INTO users (id, login, first_name, last_name) VALUES %s",
            [
                (str(user.id), user.login, user.first_name, user.last_name)
                for user in users
            ]
        )

    with conn.cursor() as cursor:
        psycopg2.extras.execute_values(
            cursor,
            "INSERT INTO credentials (key, verify_secret, user_id) VALUES %s",
            [
                (el.key, el.secret, str(el.user_id))
                for el in credentials
            ]
        )

    with conn.cursor() as cursor:
        psycopg2.extras.execute_values(
            cursor,
            "INSERT INTO addresses (id, country, city, street, building, apartment) VALUES %s",
            [
                (str(address.id), address.country, address.city, address.street, address.building, address.apartment)
                for address in map(lambda p: p.address, properties)
            ]
        )

    with conn.cursor() as cursor:
        psycopg2.extras.execute_values(
            cursor,
            "INSERT INTO properties (id, owner_id, address_id, status, price) VALUES %s",
            [
                (str(property.id), str(property.owner_id), str(property.address.id), property.status.value, property.price)
                for property in properties
            ]
        )

    with conn.cursor() as cursor:
        psycopg2.extras.execute_values(
            cursor,
            "INSERT INTO viewings (id, user_id, property_id, viewing_date) VALUES %s",
            [
                (str(viewing.id), str(viewing.user_id), str(viewing.property_id), viewing.date.strftime('%Y-%m-%d'))
                for viewing in viewings
            ]
        )
