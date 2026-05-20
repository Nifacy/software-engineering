from pathlib import Path

import faststream
import faststream.rabbit
import meilisearch

from . import _config
from . import _document_indexer
from . import _document_schema
from . import _event_publisher
from . import _handlers


def _get_schema_field(
    name: str, config_field: _config.DocumentField
) -> _document_schema.SchemaField:
    match config_field:
        case _config.IntegerField():
            return _document_schema.IntegerField(
                name=name,
                filterable=config_field.filterable,
            )
        case _config.TextField():
            return _document_schema.TextField(name=name)


def _create_document_schema(
    config: _config.AppConfiguration,
) -> _document_schema.DocumentSchema:
    return _document_schema.DocumentSchema(
        _get_schema_field(name, field) for name, field in config.document_schema.items()
    )


async def _create_document_indexer(
    config: _config.AppConfiguration,
) -> _document_indexer.DocumentIndexer:
    return await _document_indexer.create_document_indexer(
        client=meilisearch.Client(
            url=config.meilisearch.url,
            api_key=config.meilisearch.api_key,
        ),
        schema=_create_document_schema(config),
        index_name=config.index_name,
    )


def _create_broker(
    app_config: _config.AppConfiguration,
) -> faststream.rabbit.RabbitBroker:
    return faststream.rabbit.RabbitBroker(
        url=app_config.rmq_connection_url,
        timeout=60.0,
        fail_fast=False,
    )


def _create_event_publisher(
    broker: faststream.rabbit.RabbitBroker,
    app_config: _config.AppConfiguration,
) -> _event_publisher.EventPublisher:
    return _event_publisher.EventPublisher(
        broker=broker,
        pool_name=app_config.pool_name,
    )


def _create_router(
    app_config: _config.AppConfiguration,
    event_publisher: _event_publisher.EventPublisher,
    indexer: _document_indexer.DocumentIndexer,
) -> faststream.rabbit.RabbitRouter:
    return faststream.rabbit.RabbitRouter(
        handlers=(
            faststream.rabbit.RabbitRoute(
                call=_handlers.index_document(indexer, event_publisher),
                queue=f"document_indexer-{app_config.pool_name}",
            ),
        )
    )


async def create_app(config: Path) -> faststream.FastStream:
    app_config = _config.parse_config(config)
    broker = _create_broker(app_config)
    document_indexer = await _create_document_indexer(app_config)
    event_publisher = _create_event_publisher(broker, app_config)
    router = _create_router(app_config, event_publisher, document_indexer)

    broker.include_router(router)
    app = faststream.FastStream(broker, after_startup=[event_publisher.async_init])

    return app
