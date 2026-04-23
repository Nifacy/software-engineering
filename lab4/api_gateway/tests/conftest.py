import pytest
from testsuite.databases.pgsql import discover


pytest_plugins = (
    'pytest_userver.plugins.core',
    'pytest_userver.plugins.mongo',
)


MONGO_COLLECTIONS = {
    'viewings': {
        'settings': {
            'collection': 'viewings',
            'connection': 'admin',
            'database': 'admin',
        },
        'indexes': [
            {
                'key': [
                    {'name': 'property_id', 'type': 'ascending'},
                    {'name': 'viewing_date', 'type': 'ascending'}
                ],
                'unique': True,
                'name': 'index_date_unique_per_property',
            }
        ],
    },
    'users': {
        'settings': {
            'collection': 'users',
            'connection': 'admin',
            'database': 'admin',
        },
        'indexes': [
            {
                'key': 'login',
                'unique': True,
                'name': 'index_user_login_unique',
            }
        ],
    }
}


@pytest.fixture(scope='session')
def mongodb_settings():
    return MONGO_COLLECTIONS