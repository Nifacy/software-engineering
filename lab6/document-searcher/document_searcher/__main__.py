from pathlib import Path
from typing import Annotated

import typer
import uvicorn

from . import _app_builder
from . import _config


def main(
    config: Annotated[Path, typer.Option(help="Path to app configuration YAML file")],
) -> None:
    app_config = _config.load_from_config_file(config)
    uvicorn.run(
        app=_app_builder.build_app(app_config),
        host=app_config.host,
        port=app_config.port,
    )


if __name__ == "__main__":
    typer.run(main)
