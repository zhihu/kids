# 生产环境部署

我们在每台服务器上安装 kids agent，用于转发日志到中心的 kids server，部署后，可以使用任何一种语言的 redis 客户端 publish 日志到本地的 kids agent，而日志处理程序都从 kids server 订阅日志来消费。

## 制作 deb 包

如果不需要把 kids 的配置文件包含在 deb 里，比如可以用 puppet 或 saltstack 管理 kids 的配置，那么，不需要自己编译，从 [kids 的 Github release page](https://github.com/zhihu/kids/releases) 下载 deb 包，部署时覆盖`/etc/kids.conf`就可以了。如果想把配置包含在 deb 包里，需要自己生成 deb 包。

前提：

* [fpm](https://github.com/jordansissel/fpm)

下载 [kids 源码发布版](https://github.com/zhihu/kids/releases)，然后：

	tar xzf kids-VERSION.tar.gz
	cd kids-VERSION
    cp samples/agent.conf debian/kids.conf
    # EDIT kids.conf, minimally fill in server address
	make deb

Server 端，用同样的 deb 包，部署时用 server 端的配置文件覆盖 /etc/kids.conf 就好了。

## 从源码编译

下载 [源码发布包](https://github.com/zhihu/kids/releases)（文件名为 kids-VERSION.tar.gz），运行：
	
	tar xzf kids-VERSION.tar.gz
	cd kids-VERSION
    ./configure
    make
    make test  # optional
    make install

默认情况下，kids 会被安装至 `/usr/local/bin/kids`，使用 `./configure --prefix` 设置指定的安装位置，运行 `./configure --help` 获取更多设置选项。

## 使用 Docker 简化部署和 deb 包的制作

你可以用 docker 来生成可部署到生产环境的 kids container 或者用它来生成 kids deb 包。

首先：
	
	git clone https://github.com/zhihu/kids.git
	cd kids
	# or samples/server.conf
    copy samples/agent.conf debian/kids.conf
    # Edit kids.conf, minimally logfile should be set to stdout 
    # to make `docker logs` work if you run kids in a container.

### 用 docker 生成 kids 镜像

在项目根目录，运行：
	  
	docker build -t zhihu/kids .

现在，你可以这样使用它：
	
	docker run -d -p 3388:3388 zhihu/kids

### 用 docker 制作 deb 包

确保你已经生成了 `zhihu/kids` 镜像，因为 `zhihu/kids-deb` 依赖它，运行：

    cd debian
    docker build -t zhihu/kid-deb .

现在，你可以使用这个镜像来生成 deb 包：
	
	docker run -v /path/to/save/deb:/deb zhihu/kids-deb

### 配置

配置文件的具体选项详见 [配置](config.zh_CN.md)

