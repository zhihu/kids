## `worker_threads`

Worker 线程的数量

Kids 运行时会生成 n 个 worker 线程，除此之外会有 1 个负责调度的 master 线程和 1 个 storer 线程，对于 agent 一般只需要一个工作线程，而对于 server，由于处理能力和工作线程的个数、CPU 核数相关，将其设为 `CPU 核数 - 1` 能最大化 server 的处理能力。


## `nlimit`

缓冲区大小限制

格式 `nlimit mode hard_limit soft_limit soft_limit_time`

`mode` 目前仅支持 `pubsub`  
当 Client 缓冲区大小大于 hard_limit 或持续 soft_limit_time 都在 soft_limit 以上时会主动关闭客户端。


## `ignore_case`

`PSUBSCRIBE` 模式下大小写匹配开关

ignore_case 设置为 on 后，`PSUBSCRIBE` 命令将忽略大小写。

## `max_clients`

每个 host 的最大连接数

当 host 的连接数超过该值后，新建立的连接会被强制 close 。


## `log`

#### `level`
Kids 自身的 log 等级

Level 可以根据以下状态设置

	debug, info, warning, error, critical, fatal
	
Debug 模式需要在编译时定义 LOGDEBUG。


#### `file`
日志文件的存放位置, 设置为 `stderr` 会直接输出至命令行。


## `listen`
Kids 的监听设置

`host` 和 `port` 为 kids 监听的域名和端口，`socket` 为 kids 监听的 sock 文件。


## `store`

日志的存储方式

详见 [store](store.zh_CN.md)
