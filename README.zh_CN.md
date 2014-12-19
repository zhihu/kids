kids
====

Kids 是一个日志聚合系统。

采用 [Scribe](https://github.com/facebookarchive/scribe) 的消息聚合模型和 [Redis](http://redis.io/) 的 pub/sub 模型。


特性
----

* 实时订阅
* 分布式收集，集中存储
* 多线程模型
* 使用 Redis 协议
* 无第三方依赖


安装
----

编译 kids 需要 C++11 支持，如 GCC 4.7 或更高版本或 [Clang](http://clang.llvm.org)

    ./autogen.sh
    ./configure
    make
    make test  # optional
    make install

默认情况下，kids 会被安装至 `/usr/local/bin/kids`，使用 `--prefix` 选项设置指定的安装位置，运行 `./configure --help` 获取更多设置选项。


快速开始
--------

在分布式模式下，首先使用 `server.conf` 启动 kids server：

    kids -c sample/server.conf

然后，修改 `networkstore` 中的 `host` 与 `port`：

    store network primary {
      host kidsserver;
      port 3388;
    }

然后使用修改后的 `agent.conf` 启动 kids agent：

    kids -c sample/agent.conf

最后，将需要收集的日志以 Redis 协议的 `publish` 命令发送到 kids agent，agent 会将它们转发至 kids server，server 端会对其进行持久化存储，同时，可以在 server 端直接使用 `subscribe` 或者 `psubscribe` 命令实时订阅日志。

配置文件的具体选项详见 [配置](doc/config.zh_CN.md)。

你也可以不使用 kids agent 而直接运行 `kids -c sample/server.conf` 使用单服务器模式，但是这是不被推荐的行为。

开源协议
--------

Kids 使用 BSD 协议，具体内容详见 LICENSE 文件。

FAQ
---

Q: 为什么叫「kids」?  
A: 「kids」是「Kids Is Data Stream」的递归缩写。

架构图
------

![image](doc/image/arch.jpg)

[English Version](README.md)
