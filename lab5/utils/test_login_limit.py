import asyncio
import dataclasses
import json
from pathlib import Path
import time

import http
import aiofiles
import _client
import zapros
import _logging
import typer
import _generator



@dataclasses.dataclass(frozen=True, slots=True, kw_only=True)
class FindRequestStats:
    start_time: float
    finish_time: float


app = typer.Typer()


async def _worker(client: _client.ApiClient, login: str, password: str, queue: asyncio.Queue[FindRequestStats]):
    while True:
        try:
            start_time = time.time()
            await client.login(login, password)
            finish_time = time.time()
            await queue.put(FindRequestStats(start_time=start_time, finish_time=finish_time))
        except zapros.StatusCodeError as e:
            if e.response.status != http.HTTPStatus.TOO_MANY_REQUESTS:
                raise


async def _main_impl(workers: int, stats_file: Path) -> None:
    logger = _logging.get_logger()

    async with zapros.AsyncClient() as http_client:
        client = _client.ApiClient("http://localhost:8080", http_client)
        creds = _generator.get_random_user_creds()
        await client.register_user(creds)

        async with asyncio.TaskGroup() as group:
            stats_queue = asyncio.Queue[FindRequestStats]()

            for _ in range(workers):
                group.create_task(_worker(client, creds.login, creds.password, stats_queue))

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
def main(stats_file: Path, workers: int = 1) -> None:
    _logging.setup_logging()
    asyncio.run(_main_impl(workers, stats_file))

if __name__ == "__main__":
    app()
