import asyncio
import dataclasses
from typing import Any
from typing import Awaitable
from typing import Callable
from typing import Iterable

import meilisearch
import meilisearch.index

from . import _document_schema

type RawData = dict[str, object]
type DocumentIndexer = Callable[[RawData], Awaitable[None]]


@dataclasses.dataclass(frozen=True, slots=True, kw_only=True)
class Document:
    id: str
    fields: dict[str, Any]


async def create_document_indexer(
    client: meilisearch.Client,
    schema: _document_schema.DocumentSchema,
    index_name: str,
) -> DocumentIndexer:
    index = await _get_index(client, index_name)
    await _update_filterable_attributes(
        index,
        (field.name for field in schema.fields if _is_filterable(field)),
    )

    async def indexer(raw_data: RawData) -> None:
        document = schema.validate_document(raw_data)
        await _create_document(index, document)

    return indexer


def _is_filterable(field: _document_schema.SchemaField) -> bool:
    match field:
        case _document_schema.TextField():
            return False
        case _document_schema.IntegerField():
            return field.filterable


async def _get_index(
    client: meilisearch.Client,
    index_name: str,
) -> meilisearch.index.Index:
    return await asyncio.to_thread(
        client.index,
        uid=index_name,
    )


async def _update_filterable_attributes(
    meilisearch_index: meilisearch.index.Index,
    attributes: Iterable[str],
) -> None:
    await asyncio.to_thread(
        meilisearch_index.update_filterable_attributes,
        body=list(attributes),
    )


async def _create_document(
    index: meilisearch.index.Index,
    document: _document_schema.Document,
) -> None:
    task_info = await asyncio.to_thread(
        index.add_documents,
        documents=[{"id": document.id, **document.fields}],
    )

    await asyncio.to_thread(
        index.wait_for_task,
        uid=task_info.task_uid,
    )
