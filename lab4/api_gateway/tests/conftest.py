import pytest
from testsuite.databases.pgsql import discover


pytest_plugins = (
    'pytest_userver.plugins.core',
    'pytest_userver.plugins.mongo',
)



@pytest.fixture
def mongo_schema_directory(service_source_dir) -> str:
    return service_source_dir.joinpath("mongo/")
