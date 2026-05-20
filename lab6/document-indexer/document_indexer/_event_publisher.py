import dataclasses
import logging
from typing import Final
from typing import final

import faststream.rabbit


@final
@dataclasses.dataclass(slots=True, frozen=True, kw_only=True)
class DocumentUpdatedEvent:
    document_id: str


class EventPublisher:
    __LOGGING_NAMESPACE: Final = "event-publisher"
    __EXCHANGE_NAME: Final = "document-indexer-events"

    def __init__(
        self,
        broker: faststream.rabbit.RabbitBroker,
        pool_name: str,
    ):
        self.__log = logging.getLogger(self.__LOGGING_NAMESPACE)
        self.__broker = broker
        self.__pool_name = pool_name
        self.__exchange = faststream.rabbit.RabbitExchange(
            name=self.__EXCHANGE_NAME,
            type=faststream.rabbit.ExchangeType.TOPIC,
        )

    async def async_init(self) -> None:
        self.__log.info("Declare exchange ...")
        await self.__broker.declare_exchange(self.__exchange)
        self.__log.info("Exchange declared")

    async def publish(self, event: DocumentUpdatedEvent) -> None:
        self.__log.info("Publish event: %s", event)
        await self.__broker.publish(
            exchange=self.__exchange,
            message=event,
            routing_key=self.__pool_name,
        )
