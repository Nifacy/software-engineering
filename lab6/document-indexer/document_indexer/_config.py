from pathlib import Path
from typing import Literal

import pydantic
import pydantic_settings
import yaml


class IntegerField(pydantic.BaseModel):
    type: Literal["integer"]
    filterable: bool = False


class TextField(pydantic.BaseModel):
    type: Literal["text"]


type DocumentField = IntegerField | TextField


class MeiliSearchConfig(pydantic_settings.BaseSettings):
    model_config = pydantic_settings.SettingsConfigDict(env_prefix="MEILISEARCH_")

    url: str
    api_key: str


class AppConfiguration(pydantic_settings.BaseSettings):
    rmq_connection_url: str
    meilisearch: MeiliSearchConfig = pydantic.Field(default_factory=MeiliSearchConfig)
    pool_name: str
    document_schema: dict[str, DocumentField]
    index_name: str


def parse_config(config_file: Path) -> AppConfiguration:
    config_file = config_file.resolve().absolute()

    if not config_file.exists():
        raise ValueError(f"Configuration file '{config_file}' not exists")

    return AppConfiguration(**yaml.safe_load(config_file.read_text()))
