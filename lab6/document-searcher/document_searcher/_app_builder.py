import http
import logging
from typing import Any
from typing import TypedDict

import faststream
import faststream.rabbit
import litestar
import litestar.logging
import litestar.status_codes
import meilisearch

from . import _config
from . import _document_searcher
from . import _event_handler
from . import _handlers


class _ErrorDetails(TypedDict):
    detail: str


def build_app(app_config: _config.AppConfiguration) -> litestar.Litestar:
    meillisearch_client = _create_meilisearch_client(app_config)
    searcher = _create_document_searcher(meillisearch_client, app_config)

    logging_config = litestar.logging.LoggingConfig(
        formatters={
            "standard": {
                "format": "%(asctime)s [%(levelname)s] [%(name)s] %(message)s",
            }
        },
        log_exceptions="always",
    )

    faststream_app = _create_faststream_app(app_config)

    async def _start_faststream_app() -> None:
        await faststream_app.start()

    async def _stop_faststream_app() -> None:
        await faststream_app.stop()

    app = litestar.Litestar(
        route_handlers=[_handlers.search_documents(searcher)],
        exception_handlers={
            litestar.status_codes.HTTP_500_INTERNAL_SERVER_ERROR: _log_unexpected_error,
        },
        logging_config=logging_config,
        on_startup=[_start_faststream_app],
        on_shutdown=[_stop_faststream_app],
    )

    return app


def _log_unexpected_error(
    request: litestar.Request[Any, Any, Any],
    exc: Exception,
) -> litestar.Response[_ErrorDetails]:
    log = request.logger
    log.error(f"Unexpected error occurred during request {request.url.path}: {exc}")

    return litestar.Response(
        content={"detail": "Internal Server Error"},
        status_code=http.HTTPStatus.INTERNAL_SERVER_ERROR,
    )


def _create_document_searcher(
    client: meilisearch.Client,
    app_config: _config.AppConfiguration,
) -> _document_searcher.DocumentSearcher:
    return _document_searcher.create_document_searcher(
        client=client,
        index_name=app_config.index_name,
    )


def _create_meilisearch_client(
    app_config: _config.AppConfiguration,
) -> meilisearch.Client:
    return meilisearch.Client(
        url=app_config.meilisearch.url,
        api_key=app_config.meilisearch.api_key,
    )


def _create_faststream_app(
    app_config: _config.AppConfiguration,
) -> faststream.FastStream:
    broker = _create_broker(app_config)
    broker.include_router(
        faststream.rabbit.RabbitRouter(
            handlers=(
                _create_event_subscription(
                    broker=broker,
                    app_config=app_config,
                ),
            )
        )
    )

    app = faststream.FastStream(
        broker,
        logger=logging.getLogger("faststream"),
    )
    return app


def _create_broker(
    app_config: _config.AppConfiguration,
) -> faststream.rabbit.RabbitBroker:
    return faststream.rabbit.RabbitBroker(
        url=app_config.rmq_connection_url,
        timeout=60.0,
        fail_fast=False,
    )


def _create_event_subscription(
    broker: faststream.rabbit.RabbitBroker,
    app_config: _config.AppConfiguration,
) -> faststream.rabbit.RabbitRoute:
    async def _print_message(event: _event_handler.DocumentUpdatedEvent) -> None:
        print("Event received")

    return _event_handler.create_event_subscription(
        broker=broker,
        pool_name=app_config.pool_name,
        handler=_print_message,
    )
