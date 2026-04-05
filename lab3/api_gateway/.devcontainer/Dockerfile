FROM ghcr.io/userver-framework/ubuntu-24.04-userver:v2.15

RUN usermod -aG sudo ubuntu
RUN echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/ubuntu
RUN chmod 0440 /etc/sudoers.d/ubuntu

USER ubuntu
