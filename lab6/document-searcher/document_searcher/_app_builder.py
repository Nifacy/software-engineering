import http
from typing import Any
from typing import TypedDict

import litestar
import litestar.logging
import litestar.status_codes
import meilisearch

from . import _config
from . import _document_searcher
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

    app = litestar.Litestar(
        route_handlers=[_handlers.search_documents(searcher)],
        exception_handlers={
            litestar.status_codes.HTTP_500_INTERNAL_SERVER_ERROR: _log_unexpected_error,
        },
        logging_config=logging_config,
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
