import pytest
from testsuite.databases.pgsql import discover


pytest_plugins = (
    'pytest_userver.plugins.core',
    'pytest_userver.plugins.postgresql',
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
def pgsql_local(service_source_dir, pgsql_local_create):
    databases = discover.find_schemas(
        'admin',
        [service_source_dir.joinpath('postgresql/schemas')],
    )

    return pgsql_local_create(list(databases.values()))


@pytest.fixture(scope='session')
def userver_pg_config(pgsql_local):
    """
    Returns a function that adjusts the static configuration file for
    the testsuite.
    Sets the `dbconnection` to the testsuite started PostgreSQL credentials
    if there's only one `dbconnection` in static config.
 
    @ingroup userver_testsuite_fixtures
    """
 
    if not pgsql_local:
        raise ValueError(
            'Override the "pgsql_local" fixture so that testsuite knowns how to start the PostgreSQL database',
        )
 
    if len(pgsql_local) > 1:
        raise ValueError(
            f'Found more than one entry in "pgsql_local": '
            f'{list(pgsql_local.keys())}. '
            f'The "userver_pg_config" fixture supports '
            f'only one entry in "pgsql_local" fixture. The '
            f'"userver_pg_config" fixture should be overridden and '
            f'the "dbconnection" for the components::Postgres '
            f'components should be adjusted via the overridden fixture.',
        )
 
    uri = list(pgsql_local.values())[0].get_uri()
 
    def _patch_config(config_yaml, config_vars):
        components = config_yaml['components_manager']['components']
        postgre_dbs = {"database": components["database"]}
 
        if len(postgre_dbs) > 1:
            raise ValueError(
                f'Found more than one components with "dbconnection": '
                f'{list(postgre_dbs.keys())}. '
                f'The "userver_pg_config" fixture should be overridden and '
                f'the "dbconnection" for the components::Postgres '
                f'components should be adjusted via the overridden fixture.',
            )
 
        for config in postgre_dbs.values():
            config['dbconnection'] = uri
            config.pop('dbalias', None)
 
    return _patch_config


@pytest.fixture(scope='session')
def mongodb_settings():
    return MONGO_COLLECTIONS