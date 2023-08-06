FROM docker.io/library/debian:12

RUN apt-get update \
  && apt-get install -y curl wget git make build-essential

WORKDIR /build

ADD Makefile config.mk ./

ADD ./src ./src


RUN make -j"$(nproc)" vm
