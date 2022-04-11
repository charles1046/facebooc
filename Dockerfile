FROM debian:latest as build
LABEL Author="zxc25077667@protonmail.com"
WORKDIR /app

RUN apt-get update -y && \
    apt-get install -y libsqlite3-dev \
    build-essential
COPY ["Makefile", "/app/"]
COPY ["src", "/app/src/"]
COPY ["include", "/app/include/"]
RUN make release

FROM debian:latest
LABEL Author="zxc25077667@protonmail.com"
ENV port 8080

RUN apt-get update -y && apt-get install -y libsqlite3-dev
COPY ["static", "/app/static/"]
COPY ["templates", "/app/templates/"]

COPY --from=build /app/bin/facebooc /app/facebooc
USER 1000
CMD /app/facebooc ${port}