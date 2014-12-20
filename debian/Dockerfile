FROM zhihu/kids

RUN apt-get update && apt-get install -yq ruby-dev
RUN gem install fpm

WORKDIR /kids

COPY kids.conf /kids/debian/kids.conf
RUN cd /kids/debian && ./make_deb.sh

VOLUME ["/deb"]

CMD ["bash", "-c", "cp /kids/debian/*.deb /deb"]
