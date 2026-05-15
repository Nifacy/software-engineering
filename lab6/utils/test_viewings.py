import asyncio
import dataclasses
import json
from pathlib import Path
import random
import time

import aiofiles
import _client
import zapros
import _logging
import typer
import _generator
import _common


@dataclasses.dataclass(frozen=True, slots=True, kw_only=True)
class FindRequestStats:
    start_time: float
    finish_time: float


app = typer.Typer()


async def _worker(client: _client.ApiClient, property_ids: list[str], queue: asyncio.Queue[FindRequestStats]):
    while True:
        start_time = time.time()
        await client.find_property_viewings(random.choice(property_ids))
        finish_time = time.time()
        await queue.put(FindRequestStats(start_time=start_time, finish_time=finish_time))


async def _main_impl(viewings: int, workers: int, stats_file: Path) -> None:
    logger = _logging.get_logger()

    async with zapros.AsyncClient() as http_client:
        client = _client.ApiClient("http://localhost:8080", http_client)
        token = await client.register_user(_generator.get_random_user_creds())

        user_tokens: list[str] = []

        async def create_random_user(_: int) -> str:
            return await client.register_user(_generator.get_random_user_creds())

        async for token in _common.task_group_map(create_random_user, range(viewings), workers=500):
            user_tokens.append(token)

        property_ids: list[str] = []

        async def create_random_property(_: int) -> str:
            return await client.create_property(
                token=random.choice(user_tokens),
                property=_generator.get_random_property(),
            )

        async for property_id in _common.task_group_map(create_random_property, range(viewings), workers=500):
            property_ids.append(property_id)

        async with asyncio.TaskGroup() as group:
            for _ in range(viewings):
                group.create_task(client.schedule_viewing(
                    token=random.choice(user_tokens),
                    property_id=random.choice(property_ids),
                    date=_generator.get_random_date(),
                ))

        async with asyncio.TaskGroup() as group:
            stats_queue = asyncio.Queue[FindRequestStats]()

            for _ in range(workers):
                group.create_task(_worker(client, property_ids, stats_queue))

            async with aiofiles.open(stats_file, "w") as file:
                while True:
                    stats = await stats_queue.get()
                    logger.info(
                        "Request: start={start_time} time={time}",
                        start_time=stats.start_time,
                        time=stats.finish_time - stats.start_time,
                    )
                    await file.write("{data}\n".format(data=json.dumps({"start": stats.start_time, "finish": stats.finish_time, "delta": stats.finish_time - stats.start_time})))


@app.command()
def main(stats_file: Path, viewings: int = 10, workers: int = 1) -> None:
    _logging.setup_logging()
    asyncio.run(_main_impl(viewings, workers, stats_file))

if __name__ == "__main__":
    app()
