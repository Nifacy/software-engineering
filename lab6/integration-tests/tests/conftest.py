import contextlib
import logging
import shutil
from pathlib import Path
from typing import AsyncGenerator
from typing import Final
from typing import Iterator

import pytest
import pytest_asyncio
import zapros
from testcontainers.compose import DockerCompose

_CURRENT_DIR: Final = Path(__file__).parent
_SETUP_ENV_DIR: Final = _CURRENT_DIR / ".." / "setup-env"
_DOCKER_COMPOSE_FILE_PATH: Final = _SETUP_ENV_DIR / "docker-compose.yml"
_ENV_FILE: Final = _SETUP_ENV_DIR / "variables.env"
_SERVICE_LOGS_DIR: Final = Path(".service_logs")


@contextlib.contextmanager
def _log_action(log: logging.Logger, action: str) -> Iterator[None]:
    log.info("%s ...", action)

    try:
        yield
    except Exception:
        log.error("%s ... error", action)
        raise
    else:
        log.info("%s ... ok", action)


def _save_logs(compose: DockerCompose, log: logging.Logger) -> None:
    if _SERVICE_LOGS_DIR.exists():
        shutil.rmtree(_SERVICE_LOGS_DIR)
    _SERVICE_LOGS_DIR.mkdir(parents=True, exist_ok=True)

    for container in compose.get_containers():
        container.get_logs()

    with _log_action(log, "Save service logs"):
        for container in compose.get_containers():
            service_name = container.Service or container.Name
            if service_name is None:
                continue

            with _log_action(log, f"Save logs for service {service_name}"):
                stdout, stderr = container.get_logs()

                stdout_logs_file = _SERVICE_LOGS_DIR / f"{service_name}.stdout.log"
                stdout_logs_file.write_bytes(stdout)

                stderr_logs_file = _SERVICE_LOGS_DIR / f"{service_name}.stderr.log"
                stderr_logs_file.write_bytes(stderr)


@pytest.fixture(scope="session", autouse=True)
def compose():
    logger = logging.getLogger("docker-compose-fixture")

    with _log_action(logger, "Set up docker compose file"):
        context_path = str(_SETUP_ENV_DIR.resolve().absolute())
        docker_compose_file = str(_DOCKER_COMPOSE_FILE_PATH.resolve().absolute())
        environment_file = str(_ENV_FILE.resolve().absolute())

        logger.info("Docker compose file: %s", docker_compose_file)
        logger.info("Environment file: %s", environment_file)

        docker_compose = DockerCompose(
            context=context_path,
            compose_file_name=docker_compose_file,
            env_file=environment_file,
        )

        docker_compose.start()

    yield docker_compose

    _save_logs(docker_compose, logger)

    with _log_action(logger, "Stop docker compose"):
        docker_compose.stop()


@pytest_asyncio.fixture
async def http_client() -> AsyncGenerator[zapros.AsyncClient]:
    handler = zapros.RetryMiddleware(
        next_handler=zapros.AsyncStdNetworkHandler(),
        max_attempts=3,
        backoff_factor=1.0,
        backoff_max=120.0,
        backoff_jitter=0.5,
    )

    async with zapros.AsyncClient(
        handler=handler,
        base_url="http://localhost:8080",
    ) as http_client:
        yield http_client
