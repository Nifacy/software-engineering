import dataclasses
import logging
from typing import Awaitable
from typing import Callable
from typing import Final
from typing import final

import faststream.rabbit

_EXCHANGE_NAME: Final = "document-indexer-events"
_QUEUE_NAME_PREFIX: Final = "document-indexer-events-"


@final
@dataclasses.dataclass(slots=True, frozen=True, kw_only=True)
class DocumentUpdatedEvent:
    document_id: str


type EventHandler = Callable[[DocumentUpdatedEvent], Awaitable[None]]


def create_event_subscription(
    broker: faststream.rabbit.RabbitBroker,
    pool_name: str,
    handler: EventHandler,
) -> faststream.rabbit.RabbitRoute:
    log = logging.getLogger("event-handler")

    exchange = faststream.rabbit.RabbitExchange(
        name=_EXCHANGE_NAME,
        type=faststream.rabbit.ExchangeType.TOPIC,
    )

    queue = faststream.rabbit.RabbitQueue(
        name=f"{_QUEUE_NAME_PREFIX}{pool_name}",
        durable=True,
        routing_key=pool_name,
    )

    async def _handle_event(event: DocumentUpdatedEvent) -> None:
        log.info("Received document indexer event: %s", event)
        await handler(event)

    return faststream.rabbit.RabbitRoute(
        queue=queue,
        exchange=exchange,
        call=_handle_event,
    )
