import asyncio
from pathlib import Path
from typing import Annotated

import typer

from . import _app_builder


async def async_main(config: Path) -> None:
    app = await _app_builder.create_app(config)
    await app.run()


def main(
    config: Annotated[Path, typer.Option(help="Path to app configuration YAML file")],
) -> None:
    asyncio.run(async_main(config))


if __name__ == "__main__":
    typer.run(main)
