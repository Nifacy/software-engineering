import contextlib
from dataclasses import dataclass
import pathlib

from typing import Iterator
from typing import Any
from typing import Iterable

import psycopg2
import typer
import yaml
import apischema


app = typer.Typer(help="CLI утилита для анализа SQL запросов (EXPLAIN/ANALYZE)")


type _VariableValue = str | int | None


@dataclass
class _DatabaseConnectionConfig:
    host: str
    port: int
    user: str
    password: str
    database: str


class DBConfigError(Exception):
    def __init__(self, message: str) -> None:
        super().__init__(f"Invalid database connection config: {message}")


def _parse_variable(value: str) -> _VariableValue:
    if value.lower() == "null":
        return None
    if value.isdigit():
        return int(value)
    return value


def _parse_variables(var_list: list[str]) -> list[tuple[str, _VariableValue]]:
    return [
        (key, _parse_variable(value))
        for key, value in map(lambda v: v.split('=', 1), var_list)
    ]


def _load_connection_config(config_path: pathlib.Path) -> _DatabaseConnectionConfig:
    content = config_path.read_text()
    parsed_data = yaml.safe_load(content)
    return apischema.deserialize(_DatabaseConnectionConfig, parsed_data)


def _validate_file(path: pathlib.Path) -> None:
    if not path.exists():
        typer.echo(f"File '{path}' not exists", err=True)
        raise typer.Exit(code=1)


def _process_query(raw_query: str, variables: Iterable[str]) -> str:
    for number, variable in enumerate(variables, 1):
        raw_query = raw_query.replace(f'${number}', f'%({variable})s')
    return raw_query


@contextlib.contextmanager
def _create_connection(config: _DatabaseConnectionConfig) -> Iterator[Any]:
    conn = psycopg2.connect(
        host=config.host,
        port=config.port,
        user=config.user,
        password=config.password,
        dbname=config.database,
    )

    try:
        yield conn
    finally:
        conn.close()


@app.command(context_settings={"allow_extra_args": False})
def main(
    sql_file: pathlib.Path = typer.Argument(..., help="Path to SQL query script"),
    config: pathlib.Path = typer.Argument(..., help="Path to connection config file"),
    variables: list[str] | None = typer.Option(None, "--vars", "-v", help="SQL query script variables"),
):
    _validate_file(sql_file)
    _validate_file(config)

    query = sql_file.read_text(encoding='utf-8').strip()
    parsed_variables = _parse_variables(variables or [])
    connection_config = _load_connection_config(config)

    query = _process_query(query, (name for name, _ in parsed_variables))

    typer.echo(f"Query:\n{query}\n")
    typer.echo(f"Variables: {dict(parsed_variables)}\n")

    extended_query = f"EXPLAIN ANALYZE {query}"

    with _create_connection(connection_config) as conn:
        with conn.cursor() as cursor:
            cursor.execute(extended_query, {name: value for name, value in parsed_variables})
            rows = cursor.fetchall()
            output = "\n".join([row for (row,) in rows])
            typer.echo(f"Analyze:\n{output}")


if __name__ == "__main__":
    app()
