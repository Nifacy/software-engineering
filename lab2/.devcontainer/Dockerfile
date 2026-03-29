FROM ghcr.io/userver-framework/ubuntu-24.04-userver:v2.15

RUN groupadd -g 1500 user
RUN useradd -u 1500 -g user -m -s /bin/bash user

RUN mkdir -p /home/user/api_gateway
RUN chown -R user:user /home/user/api_gateway

RUN echo 'user ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers.d/user
RUN chmod 0440 /etc/sudoers.d/user

USER user
