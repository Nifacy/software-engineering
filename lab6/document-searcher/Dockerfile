FROM python:3.14.5-slim AS builder
WORKDIR /app
COPY --from=ghcr.io/astral-sh/uv:latest /uv /usr/local/bin/uv
COPY pyproject.toml uv.lock .python-version ./
ENV UV_COMPILE_BYTECODE=1 UV_LINK_MODE=copy
RUN --mount=type=cache,target=/root/.cache/uv \
    uv sync --frozen --no-dev

FROM python:3.14.5-slim
WORKDIR /app
COPY --from=builder /app/.venv /app/.venv
COPY document_searcher /app/document_searcher
ENV PATH="/app/.venv/bin:$PATH"
ENV HOST="0.0.0.0"
CMD ["python", "-m", "document_searcher", "--config", "/app/config.yml"]
