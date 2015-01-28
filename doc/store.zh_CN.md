## *基本存储类型*

### FileStore

文件存储，多用于 Server 的持久化和 Agent 的 Buffer 存储的文件缓存。

示例配置

    store file {
      path /path/to/logs/[topic]/[date];
      name [time].log;
      rotate 1hour; # 时间单位支持hour与min
    }

采用该配置后，kids 会将日志按照 `/topic/date/time.log` 的形式存储，每小时进行一次文件的切分。
此外，还可以在文件存储配置块中添加`flush 5s`之类的配置来确保内存中buffer的数据被及时写入到文件中。
这在处理量少但是较为关键的log类型时可能会带来帮助，但大部分时间你并不需要`flush`选项，或者使用订阅模式来获得真正实时的数据。

### NetworkStore
网络存储，将日志通过网络转发至指定主机。

示例配置

    store network {
      host kidsserver;
      port 3388;
    }

### NullStore
空存储，此时 kids 不会对指定形式的日志进行任何方式的存储，可与其他存储方式组合对指定日志进行过滤。

示例配置

    store null {
      topic kids.ignoretopic;
    }

## *组合存储方式*

### PriorityStore
日志按照配置文件中的顺序尝试存储，如果某一存储方式存储成功，则不再继续对该日志进行后续存储。

示例配置

    store priority {
      store null {
        topic kids.ignoretopic1;
        topic kids.ignoretopic2;
      }

      store network {
        host kidsserver;
        port 3388;
      }

      store file {
        path /path/to/logs/[topic]/[date];
        name [time].log;
        rotate 1hour;
      }
    }

采用该配置后 kids 会忽略 kids.ignoretopic1 与 kids.ignoretopic2 的存储，对于其他 topic，会转发至 kidsserver:3388，若转发失败，会保存到指定文件中。

### BufferStore
BufferStore 使用 Primary 与 Secondary 两级存储，
当 Primary 存储出现异常时，会将日志缓存至 Secondary 存储中，当 Primary 恢复后，会将 Secondary 中的缓存重新存储至 Primary。

示例配置

    store buffer {
      store network primary { # primary
        host kidsserver;
        port 3388;
      }

      store file secondary { # secondary
        path /data/kidsbuf;
        rotate 5min;
      }
    }

### MultipleStore
MultipleStore 会将日志存储至多个位置，通过定义 `success` 的类型，判断存储的成功与失败  
`success` 的值可选 `any` 或者 `all`。

示例配置

    store multiple {
      success any;
    # master
      store buffer {
        store network primary {
          host kidsserver;
          port 3388;
        }
        store file secondary {
          path /tmp/kidsbuf;
          rotate 10min;
        }
      }

    # backup
      store buffer {
        store network primary {
          host kidsbackup;
          port 3388;
        }
        store file secondary {
          path /tmp/kidsbuf2;
          rotate 10min;
        }
      }
    }
