from typing import AsyncGenerator, Awaitable, Callable, Iterable
import asyncio


async def _worker[T, R](
    func: Callable[[T], Awaitable[R]],
    input_queue: asyncio.Queue[T],
    result_queue: asyncio.Queue[R],
) -> None:
    while not input_queue.empty():
        param = await input_queue.get()
        result = await func(param)
        await result_queue.put(result)


async def task_group_map[T, R](
    func: Callable[[T], Awaitable[R]],
    params: Iterable[T],
    *,
    workers: int = 1,
) -> AsyncGenerator[R]:
    input_queue = asyncio.Queue[T]()
    output_queue = asyncio.Queue[R]()

    for item in params:
        await input_queue.put(item)

    async with asyncio.TaskGroup() as group:
        for _ in range(workers):
            group.create_task(_worker(func, input_queue, output_queue))

        while not input_queue.empty():
            yield await output_queue.get()
        
        while not output_queue.empty():
            yield await output_queue.get()
