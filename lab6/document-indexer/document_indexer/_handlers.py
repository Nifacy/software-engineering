from typing import Any
from typing import Awaitable
from typing import Callable

from ._document_indexer import DocumentIndexer


def index_document(
    document_indexer: DocumentIndexer,
) -> Callable[[dict[str, Any]], Awaitable[None]]:
    async def handler(raw_data: dict[str, Any]) -> None:
        await document_indexer(raw_data)

    return handler
