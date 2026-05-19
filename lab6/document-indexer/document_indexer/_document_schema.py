import dataclasses
from typing import Any
from typing import Final
from typing import Iterable


@dataclasses.dataclass
class Document:
    id: str
    fields: Any


@dataclasses.dataclass
class IntegerField:
    name: str
    filterable: bool = False
    VALUE_TYPE: Final = int


@dataclasses.dataclass
class TextField:
    name: str
    VALUE_TYPE: Final = str


type SchemaField = IntegerField | TextField


class ValidationError(Exception):
    pass


class MissedFieldError(ValidationError):
    def __init__(self, name: str) -> None:
        super().__init__(f"Field '{name}' missed in document")


class ExtraFieldError(ValidationError):
    def __init__(self, name: str) -> None:
        super().__init__(f"Field '{name}' not defined in schema")


class InvalidFieldValueType(ValidationError):
    def __init__(
        self, name: str, value_type: type[object], expected_type: type[object]
    ) -> None:
        super().__init__(
            f"Invalid type of value for field '{name}': expected '{expected_type}' but found '{value_type}'"
        )


class DocumentSchema:
    _ID_FIELD: Final = "id"

    def __init__(self, fields: Iterable[SchemaField]) -> None:
        self.__fields = {field.name: field for field in fields}

    def validate_document(self, raw_data: dict[str, object]) -> Document:
        document_fields, document_id = self.__get_document_id(raw_data)
        keys = {*document_fields.keys(), *self.__fields.keys()}

        for key in keys:
            raw_data_value = document_fields.get(key)
            schema_field = self.__fields.get(key)

            if raw_data_value is None:
                raise MissedFieldError(key)

            if schema_field is None:
                raise ExtraFieldError(key)

            if type(raw_data_value) is not schema_field.VALUE_TYPE:
                raise InvalidFieldValueType(
                    key,
                    type(raw_data_value),
                    schema_field.VALUE_TYPE,
                )

        return Document(id=document_id, fields=document_fields)

    @classmethod
    def __get_document_id(
        cls, raw_data: dict[str, object]
    ) -> tuple[dict[str, object], str]:
        data_copy = raw_data.copy()
        document_id = data_copy.pop(cls._ID_FIELD, None)
        if document_id is None:
            raise MissedFieldError(cls._ID_FIELD)
        if not isinstance(document_id, str):
            raise InvalidFieldValueType(cls._ID_FIELD, type(document_id), str)
        return data_copy, document_id

    @property
    def fields(self) -> Iterable[SchemaField]:
        return self.__fields.values()
