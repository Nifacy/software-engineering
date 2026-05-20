from typing import Any
from typing import Awaitable
from typing import Callable

from ._document_indexer import DocumentIndexer
from ._event_publisher import DocumentUpdatedEvent
from ._event_publisher import EventPublisher


def index_document(
    document_indexer: DocumentIndexer,
    event_publisher: EventPublisher,
) -> Callable[[dict[str, Any]], Awaitable[None]]:
    async def handler(raw_data: dict[str, Any]) -> None:
        indexed_document = await document_indexer(raw_data)
        await event_publisher.publish(
            DocumentUpdatedEvent(
                document_id=indexed_document.id,
            )
        )

    return handler
