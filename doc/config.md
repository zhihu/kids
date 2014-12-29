## `worker_threads`

Number of worker threads.

Besides worker threads, there are a store thread and a master thread, but master thread is usually idle, so just set worker_threads to number of CPU cores - 1.

## `nlimit`

Limit of client's buffer size.

Set nlimit as follow:

    nlimit mode hard_limit soft_limit soft_limit_time

Mode can only be `pubsub` now.

Kids will close client when the buffer size reaches at the `hard_limit` or `soft_limit` for a duration of `soft_limit_time`.

## `ignore_case`

Case sensitive switch for `PSUBSCRIBE` command.

`PSUBSCRIBE` command will be case insensitive when ignore_case is 'on'.

## `max_clients`

Max number of connections for each host.

When the number of connections of host exceed the value, the new connection will be forced closed.

## `log`

#### `level`  
Log level.

Level can be configured as one of the following states:

    debug, info, warning, error, critical, fatal

You need to compile kids with LOGDEBUG for debug level.

#### `file`  
Log file name.

Setting the parameter to `stderr` will directly print log to terminal.

## `listen`

Listen config for kids.

Set `host` and `port` for network connections and set `socket` to socket file for kids.

## `store`

Open [store](./store.md) for store config.
