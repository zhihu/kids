FROM debian:wheezy

MAINTAINER Li Yichao <liyichao.good@gmail.com>

COPY ./docker/sources.list /etc/apt/sources.list
RUN	apt-get update && \
	apt-get install -y --no-install-recommends \
	build-essential \
	libtool \
	automake

WORKDIR /kids

COPY . /kids
RUN ./autogen.sh && ./configure && make

EXPOSE :3388

CMD ["src/kids", "-c", "/kids/debian/kids.conf"]
