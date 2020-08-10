FROM debian:10

MAINTAINER Li Yichao <liyichao.good@gmail.com>


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
