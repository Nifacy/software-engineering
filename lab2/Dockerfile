FROM ghcr.io/userver-framework/ubuntu-24.04-userver:v2.15 AS builder

WORKDIR /build

COPY . .

RUN make build-release -j$(nproc)

FROM ghcr.io/userver-framework/ubuntu-24.04-userver:v2.15

RUN groupadd -g 1500 api_gateway
RUN useradd -u 1500 -g api_gateway -m -s /bin/bash api_gateway

COPY --from=builder /build/build-release/api_gateway /usr/bin/api_gateway

RUN mkdir -p /etc/api_gateway
RUN mkdir -p /etc/api_gateway/config
COPY --from=builder /build/configs/static_config.yaml /etc/api_gateway/config/static_config.yaml
COPY --from=builder /build/configs/config_vars.yaml /etc/api_gateway/config/config_vars.yaml

COPY --from=builder /build/build-release/static /etc/api_gateway/static
WORKDIR /etc/api_gateway
USER api_gateway
EXPOSE 8080

CMD [ "CPU_LIMIT=$(nproc)c", "/usr/bin/api_gateway", "--config", "/etc/api_gateway/config/static_config.yaml", "--config_vars", "/etc/api_gateway/config/config_vars.yaml" ]
