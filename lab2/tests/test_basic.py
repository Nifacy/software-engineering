# import sys
# import pytest_userver

# # Start via `make test-debug` or `make test-release`


# async def test_basic(service_client):
#     assert pytest_userver.__file__ == ""
#     response = await service_client.post('/hello', params={'name': 'Tester'})
#     assert response.status == 200
#     assert response.text == 'Hello, Tester!\n'
