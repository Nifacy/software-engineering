import http
from typing import Any
from typing import Literal

import litestar
import litestar.exceptions
import litestar.handlers

from . import _document_searcher


def search_documents(
    searcher: _document_searcher.DocumentSearcher,
) -> litestar.handlers.HTTPRouteHandler:
    @litestar.get("/documents")
    async def search_documents(query: dict[str, Any]) -> list[str]:
        return await searcher(_parse_args(query))

    return search_documents


def _parse_args(
    query_params: dict[str, str],
) -> _document_searcher.Filters:
    range_filters: dict[str, _document_searcher.RangeFilter] = {}
    text_filter = query_params.pop("text", None)

    if text_filter is None:
        raise litestar.exceptions.HTTPException(
            status_code=http.HTTPStatus.BAD_REQUEST,
            detail="Missed required field 'text'",
        )

    for key, value in query_params.items():
        filter_type, field_name, filter_value = _parse_filter_parameter(key, value)
        range_filter = range_filters.setdefault(
            field_name,
            _document_searcher.RangeFilter(field=field_name),
        )

        match filter_type:
            case "max":
                range_filter.max = filter_value
            case "min":
                range_filter.min = filter_value

    return _document_searcher.Filters(
        text=text_filter,
        range_filters=list(range_filters.values()),
    )


def _parse_filter_parameter(
    name: str,
    value: str,
) -> tuple[Literal["max", "min"], str, int]:
    if name.endswith("_min"):
        field_name = name.removesuffix("_min")
        return "min", field_name, _parse_integer_value(name, value)

    elif name.endswith("_max"):
        field_name = name.removesuffix("_max")
        return "max", field_name, _parse_integer_value(name, value)

    else:
        raise litestar.exceptions.HTTPException(
            status_code=http.HTTPStatus.BAD_REQUEST,
            detail=f"Extra query parameter '{name}'",
        )


def _parse_integer_value(field_name: str, raw_value: str) -> int:
    if not raw_value.isdigit():
        raise litestar.exceptions.HTTPException(
            status_code=http.HTTPStatus.BAD_REQUEST,
            detail=f"Value for field '{field_name}' is not integer",
        )

    return int(raw_value)
