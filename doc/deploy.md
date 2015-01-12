# Run in production

In production, we deploy kids agent at every host, which forwards log to a kids server, after deployment, you can use any redis client to publish log to local
kids agent, and subscribe to kids server to consume the log.

## Make a deb package

If you do not have to include config in the package, this may happen, 
for example, you use puppet or saltstack to manage your configuration file, 
then you do not have to make the package yourself, just download it from
[kids's Github releases page](https://github.com/zhihu/kids/releases).

Prerequisites:

* [fpm](https://github.com/jordansissel/fpm)

Download [kids source release](https://github.com/zhihu/kids/releases), then: 
	
	tar xzf kids-VERSION.tar.gz
	cd kids-VERSION
    cp samples/agent.conf debian/kids.conf
    # EDIT kids.conf, minimally fill in server address
	make deb

For server, use the same deb package and overwrite /etc/kids.conf with server's config file.


## From Source

Download [source release](https://github.com/zhihu/kids/releases)(with name kids-VERSION.tar.gz), run:
    
	tar xzf kids-VERSION.tar.gz
	cd kids-VERSION
    ./configure
    make
    make test  # optional
    make install

Kids will be installed to `/usr/local/bin/kids`, use `./configure --prefix` to specify another path, run `./configure --help` to see more options.

## Using Docker to simplify deployment and packaging

You can use docker to build a kids container or make a deb package

Firstly:

	git clone https://github.com/zhihu/kids.git
	cd kids
	# or samples/server.conf
    copy samples/agent.conf debian/kids.conf
    # Edit kids.conf, minimally logfile should be set to stdout 
    # to make `docker logs` work if you run kids in a container.

### Using Docker to make a kids container

In the project root directory, just run:
    
    docker build -t zhihu/kids .

Now, you can use it like this:
    
    docker run -d -p 3388:3388 zhihu/kids

### Using Docker to make a deb package

Make sure you have built `zhihu/kids`, then run:
    
    cd debian
    docker build -t zhihu/kids-deb .

Now, you can use it to build a deb package:
    
    docker run -v /path/to/save/deb:/deb zhihu/kids-deb

### Configuration

See [here](config.md).

    