# kids

[![Build Status]][Travis CI]

Kids 是一个日志聚合系统。

采用 [Scribe](https://github.com/facebookarchive/scribe) 的消息聚合模型和 [Redis](http://redis.io/) 的 pub/sub 模型。


## 特性

* 实时订阅
* 分布式收集，集中存储
* 多线程模型
* 使用 Redis 协议
* 无第三方依赖


## 安装

### 二进制包

kids 的二进制包支持基于 Debian 的 Linux 64位发行版 [获取 release 版本](https://github.com/zhihu/kids/releases)   
在 Debian 文件夹中的示例配置不适合用于生产环境，[在生产环境中部署](#production)

### 从源码安装

编译 kids 需要 C++11 支持，如 GCC 4.7 或更高版本或 [Clang](http://clang.llvm.org)

请从 [release](https://github.com/zhihu/kids/releases) 中下载最新 Release 版本，执行：

	tar xzf kids-VERSION_source.tar.gz
	cd kids-VERSION
	./configure
	make
	make test  # optional
	make install

默认情况下，kids 会被安装至 `/usr/local/bin/kids`，使用 `--prefix` 选项设置指定的安装位置，运行 `./configure --help` 获取更多设置选项。


## 快速开始

Kids 在 `samples` 文件夹中有示例的配置文件，编译安装完成后运行：

	kids -c samples/dev.conf
	
因为 kids 使用 Redis 协议，所以你可以使用 `redis-cli` 去连接 kids, 打开终端，运行：
		
	$ redis-cli -p 3888
	$ 127.0.0.1:3388> PSUBSCRIBE *

在另一个终端中运行：
	
	$ redis-cli -p 3388
	$ 127.0.0.1:3388> PUBLISH kids.testtopic message

在 Linux 环境下使用 `redis-cli` 需要安装 `redis-tools`

	$ sudo apt-get install redis-tools
	
在 Mac 环境下使用 `redis-cli` 仅需要安装 `redis`

	$ brew install redis
	
配置文件的具体选项详见 [配置](doc/config.zh_CN.md)。

执行 `kids --help` 查看更多选项。

<a name="production"></a>
## 生产环境

在生产环境中，需要将 agent 部署至每一台需要记录日志的主机，将 server 部署至足够强大的主机

使用打包或者 docker container 可以简化部署流程

### 创建打包文件

前提条件：

* 需要 build-essential, libtool, automake 去构建项目
* 使用 [fpm](https://github.com/jordansissel/fpm) 进行打包

流程：

	cp samples/agent.conf debian/kids.conf
	# 修改 kids.conf, 更新 server 地址
	make deb

对于 server, 使用相同的 deb 文件，使用 server 对应的配置覆盖 `/etc/kids.conf` 下的配置

### 使用 Docker

你可以使用 Docker 去创建 kids 的 container 运行，或者使用它去创建 deb 包在 container 外运行

首先准备好你的配置文件：

	# 修改 samples/agent.conf 或者 samples/server.conf
	copy samples/agent.conf debian/kids.conf
	# 修改 kids.conf, file 需要修改至 stdout 以使 docker 日志生效

#### 创建 kids container

在项目的根目录，创建 kids 的 image：

	docker build -t zhihu/kids .
	
然后就可以运行在 container 里了

	docker run -d -p 3388:3388 zhihu/kids
	
#### 创建 deb 包

确认你已经创建了 zhihu/kids 的 image，kids-deb 依赖于它，然后：

	cd debian
	docker build -t zhihu/kids-deb .
	
使用 image 创建 deb 包

	docker run -v /path/to/save/deb:/deb zhihu/kids-deb


## 开源协议

Kids 使用 BSD 协议，具体内容详见 LICENSE 文件。


## FAQ

Q: 为什么叫「kids」?  
A: 「kids」是「Kids Is Data Stream」的递归缩写。


## 架构图

![image](doc/image/arch.jpg)

[Build Status]: https://img.shields.io/travis/zhihu/kids/master.svg?style=flat
[Travis CI]:	https://travis-ci.org/zhihu/kids

