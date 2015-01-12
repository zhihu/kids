# kids

[![Build Status]][Travis CI]


Kids 是一个日志收集系统。

采用 [Scribe](https://github.com/facebookarchive/scribe) 的消息聚合模型和 [Redis](http://redis.io/) 的 pub/sub 模型。


## 特性

* 实时订阅
* 分布式收集，集中存储
* 多线程模型
* 使用 Redis 协议
* 无第三方依赖


## 快速开始

### 从源码编译

编译 kids 需要 C++11 支持，如 GCC 4.7 或更高版本或 [Clang](http://clang.llvm.org)

下载 [源码发布包](https://github.com/zhihu/kids/releases)（文件名为 kids-VERSION.tar.gz），运行：
	
	tar xzf kids-VERSION.tar.gz
	cd kids-VERSION
    ./configure
    make

Kids samples/ 文件夹带了一些示例配置，编译好后，运行：

	kids -c samples/dev.conf

kids 使用 redis 协议，现在你可以用 `redis-cli` 和它通信：

	$ redis-cli -p 3388
	$ 127.0.0.1:3388> PSUBSCRIBE *

在另一个终端：

	$ redis-cli -p 3388
    $ 127.0.0.1:3388> PUBLISH test message

`redis-cli` 命令需要安装 redis。在 MAC 上，可以用 `brew install redis` 安装它。
配置文件的具体选项详见 [配置](doc/config.zh_CN.md)。

执行 `kids --help` 查看更多选项。

### 使用 Docker

如果已经装了 Docker，使用 Docker 是最快速开始的方式。
    
    git clone https://github.com/zhihu/kids.git
    cd kids    
    cp samples/dev.conf debian/kids.conf
    docker build -t zhihu/kids .

现在，你可以这样运行：
    
    docker run -d -p 3388:3388 zhihu/kids

你也可以指定配置文件：
    
    docker run -d -v /path/to/kids.conf:/etc/kids.conf -p 3388:3388 zhihu/kids

## 配置

请看[配置](doc/config.zh_CN.md)。

## 生产环境部署

请看[生产环境部署](doc/deploy.zh_CN.md)。

## 开发者

你需要以下东西来从源码编译 kids：

* build-essential
* libtool
* automake
* c++ compiler with c++ 11 support like gcc4.7+ or [Clang](http://clang.llvm.org)

运行以下命令编译 kids：

	./autogen.sh
	./configure
	make

            
## 开源协议


Kids 使用 BSD-3 协议，具体内容详见 LICENSE 文件。


## FAQ

Q: 为什么叫「kids」?  
A: 「kids」是「Kids Is Data Stream」的递归缩写。


## 架构图

![image](doc/image/arch.jpg)

[Build Status]: https://img.shields.io/travis/zhihu/kids/master.svg?style=flat
[Travis CI]:	https://travis-ci.org/zhihu/kids

