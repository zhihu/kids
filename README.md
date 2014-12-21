# kids

[![Build Status]][Travis CI]

Kids is a log aggregation system.

It aggregates messages like [Scribe](https://github.com/facebookarchive/scribe) and its pub/sub pattern is ported from [Redis](http://redis.io/).


## Features

* Real-time subscription
* Distributed collection
* Message persistence
* Multithreading
* Redis protocol
* No third-party dependencies


## Installation

### Binaries

kids [releases are available on the Github project releases page](https://github.com/zhihu/kids/releases).
Binaries are available for Linux, with package for Debian based distributions.

There is an example conf in Debian packages, but it is not useful in production, to deploy in production, see [Run in production](#production).

### From Source

You need a complier with C++11 support like GCC 4.7 (or later) or [Clang](http://clang.llvm.org).

Download a [source release](https://github.com/zhihu/kids/releases), then:
	
	tar xzf kids-VERSION_source.tar.gz
	cd kids-VERSION
    ./configure
    make
    make test  # optional
    make install

By default, it will be installed to `/usr/local/bin/kids`.
You can use the `--prefix` option to specify the installation location.
Run `./configure --help` for more config options.


## Quickstart

Kids comes with some sample config files in `samples/`, after building, simply run:

    kids -c samples/dev.conf

Because kids uses redis protocol, so you can use `redis-cli` to play with it, open another terminal:
    
    $ redis-cli -p 3888
    $ 127.0.0.1:3388> PSUBSCRIBE *

In yet another terminal:
    
    $ redis-cli -p 3388
    $ 127.0.0.1:3388> PUBLISH test message

`redis-cli` needs `redis` to be installed. On Mac, you can run `brew install redis` to install it.

Full explanation of config file, see [here](doc/config.md).

Run `kids --help` for more running options.

<a name="production"></a>
## Run in production

In production, we deploy kids agent at every host, and assign a powerful server to kids server,

To simplify deployment, use a package or a docker container.

If you do not have to include config in the package, this may happen, 
for example, you use puppet or saltstack to manage your configuration file, 
then you do not have to make the package yourself, just download it from
[kids's Github releases page](https://github.com/zhihu/kids/releases).

### Creating packages

Prerequisites:

* build-essential, libtool, automake for building the prject
* [fpm](https://github.com/jordansissel/fpm) for packaging

Instructions: 

    cp samples/agent.conf debian/kids.conf
    # EDIT kids.conf, minimally fill in server address
	make deb

For server, use the same deb package and overwrite /etc/kids.conf with server's config file.

### Using Docker

You can use docker to build a kids container to run or use it to make a deb package and run kids outside
a container.

First prepare your config file:

    # or samples/server.conf
    copy samples/agent.conf debian/kids.conf
    # Edit kids.conf, minimally logfile should be set to stdout 
    # to make `docker logs` work if you run kids in a container.

#### Build a kids container

In project root directory, build the kids image:

    docker build -t zhihu/kids .

Now you can run it like so:

    docker run -d -p 3388:3388 zhihu/kids

#### Make a deb package

Make sure you have built the zhihu/kids image, because the kids-deb depends on it.

    cd debian
    docker build -t zhihu/kid-deb .

You can now use the image to get a deb package.

    docker run -v /path/to/save/deb:/deb zhihu/kids-deb

## License

Kids is BSD-licensed, see LICENSE for more details.


## FAQ

Q: What is the meaning of "kids"?  
A: "kids" is the recursive acronym of "Kids Is Data Stream".


## Architecture

![image](doc/image/arch.jpg)

You can view the Chinese version README [here](README.zh_CN.md)


[Build Status]: https://img.shields.io/travis/zhihu/kids/master.svg?style=flat
[Travis CI]:    https://travis-ci.org/zhihu/kids
