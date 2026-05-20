from pathlib import Path

import pydantic
import pydantic_settings
import yaml


class MeiliSearchConfig(pydantic_settings.BaseSettings):
    model_config = pydantic_settings.SettingsConfigDict(env_prefix="MEILISEARCH_")

    url: str
    api_key: str


class AppConfiguration(pydantic_settings.BaseSettings):
    port: int = 8080
    host: str = "127.0.0.1"
    meilisearch: MeiliSearchConfig = pydantic.Field(default_factory=MeiliSearchConfig)
    index_name: str


def load_from_config_file(path: Path) -> AppConfiguration:
    path = path.resolve().absolute()
    raw_config = yaml.safe_load(path.read_text()) if path.exists() else {}
    return AppConfiguration(**raw_config)
