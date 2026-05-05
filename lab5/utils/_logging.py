import sys

import loguru
import typing


if typing.TYPE_CHECKING:
    Logger: typing.TypeAlias = loguru.Logger
else:
    Logger = typing.Any


def setup_logging() -> None:
    loguru.logger.remove()
    loguru.logger.add(
        sys.stderr,
        colorize=True,
        format="<green>{time:HH:mm:ss}</green> <level>[{level}]</level> [{extra[namespace]}] {message}"
    )


def get_logger(namespace: str = "main") -> Logger:
    return loguru.logger.bind(namespace=namespace)
