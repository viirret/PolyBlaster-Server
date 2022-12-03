FROM ubuntu:jammy
EXPOSE 8080

ENV DEBIAN_FRONTEND=noninteractive
ENV DEBCONFIG_NONINTERACTIVE_SEEN=true
RUN apt-get update && apt-get upgrade -y && apt-get install -y build-essential libboost-all-dev cmake 

RUN mkdir -p /app/tmp/build

COPY CMakeLists.txt /app/tmp/
COPY src /app/tmp/src
COPY websocketpp /app/tmp/websocketpp

WORKDIR /app/tmp/build

RUN ls -l ..
RUN cmake .. && make
RUN cp ./x /app/polyblaster-server
COPY entrypoint.sh /app/entrypoint.sh

WORKDIR /app
RUN rm -rf /app/tmp

ENTRYPOINT [ "/app/entrypoint.sh" ]