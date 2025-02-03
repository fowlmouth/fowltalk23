FROM docker.io/library/debian:12 AS build

RUN apt-get update \
  && apt-get install -y curl wget git make build-essential gdb

WORKDIR /build

ADD Makefile config.mk ./

ADD ./src ./src

RUN make -j"$(nproc)" compiler vm

FROM docker.io/library/debian:12 AS main

WORKDIR /app
COPY --from=build /build/vm /build/compiler /build/libfowl.so ./

ENTRYPOINT ["./compiler"]
CMD ["--help"]
