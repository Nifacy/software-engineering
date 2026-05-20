import asyncio
import dataclasses
import logging
from typing import Awaitable
from typing import Callable
from typing import Iterable

import meilisearch


@dataclasses.dataclass
class RangeFilter:
    field: str
    min: int | None = None
    max: int | None = None


@dataclasses.dataclass
class Filters:
    text: str
    range_filters: list[RangeFilter]


type DocumentSearcher = Callable[[Filters], Awaitable[list[str]]]


def create_document_searcher(
    client: meilisearch.Client,
    index_name: str,
) -> DocumentSearcher:
    log = logging.getLogger("document-searcher")

    log.info("Get index '%s' ...", index_name)
    index = client.index(index_name)

    async def search_documents(filters: Filters) -> list[str]:
        log.info("Search documents with filter: %s ...", filters)

        filter_string = _get_filter_string(filters.range_filters)
        log.info("Filter string: '%s'", filter_string)

        result = await asyncio.to_thread(
            index.search,
            filters.text,
            {
                "filter": filter_string,
                "attributesToRetrieve": ["id"],
            },
        )

        document_ids = [doc["id"] for doc in result["hits"]]
        log.info("Found document IDs: %s", document_ids)
        return document_ids

    return search_documents


def _get_filter_string(range_filters: Iterable[RangeFilter]) -> str:
    filter_components: list[str] = []

    for filter in range_filters:
        if filter.min is not None:
            filter_components.append(f"{filter.field} >= {filter.min}")
        if filter.max is not None:
            filter_components.append(f"{filter.field} <= {filter.max}")

    return " AND ".join(filter_components)
