FROM debian:latest as build
LABEL Author="zxc25077667@protonmail.com"
WORKDIR /app

RUN apt-get update -y && \
    apt-get install -y libsqlite3-dev libssl-dev \
    build-essential sassc
COPY . .
RUN make release

FROM alpine:latest
LABEL Author="zxc25077667@protonmail.com"
ENV port 8080
ENV DB_PATH "/app/data/db.sqlite3"
WORKDIR /app

RUN apk add sqlite-dev libc6-compat libressl-dev --no-cache && \
    mkdir -p /app/data && touch ${DB_PATH} && chmod 600 ${DB_PATH} && chown -R 1000:1000 /app/data
COPY ["static/image", "/app/static/image/"]
COPY ["templates", "/app/templates/"]

COPY --from=build --chown=1000:1000 /app/bin/facebooc /app/facebooc
COPY --from=build --chown=1000:1000 /app/templates/version.html /app/templates/
COPY --from=build --chown=1000:1000 /app/static/css /app/static/css

USER 1000
CMD /app/facebooc ${port}