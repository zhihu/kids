FROM debian:wheezy

MAINTAINER Li Yichao <liyichao.good@gmail.com>

RUN	apt-get update && \
	apt-get install -yq --no-install-recommends \
	build-essential \
	libtool \
	automake

WORKDIR /kids

COPY . /kids
RUN ./configure && make

EXPOSE :3388

CMD ["src/kids", "-c", "/kids/debian/kids.conf"]

