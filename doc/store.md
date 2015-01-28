## *Basic Storage*

### FileStore

Using FileStore, kids will save log to file, and divide the file into several files according to the value of `rotate`.  
Mostly used in persistent storage for server and the buffer storage for agent  

Example:

    store file {
      path /path/to/logs/[topic]/[date];
      name [time].log;
      rotate 1hour; # both min and hour are supported
    }

Using config above, kids will save log to path like `/topic/date/01-00-00.log`.

You can also add things like `flush 5s` to make sure buffered data written into log files in time.
This would be helpful when you dealing with logs with small amount but high priority.
In most cases, you can safely ignore `flush`, or use real-time subscribe instead.


### NetworkStore

Using NetworkStore, kids will use `log` command to resend  log to specific host, `log` is treated as `publish`.  

Example:

    store network {
      host kidsserver;
      port 3388;
    }


### NullStore

Using NullStore, kids will not store specified log in any way,   
NullStore can filter the specified log in a combination of other storage methods.  


Example:

    store null {
      topic kids.ignoretopic;
    }

## *Combined Storage*

### PriorityStore  

Using PriorityStore, kids will try to store log according to the order of storage methods in config file, if stored successfully using one method,  kids will ignore the rest methods in config file.

Example:

    store priority {
      store null {
        topic kids.ignoretopic1;
        topic test.debug;
        topic debug.*;
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

In the example above, using `PriorityStore`, kids will ignore the storage of `kids.ignoretopic1`and `kids.ignoretopic2`,  as for other topic, kids will resend log to kidsserver:3388,  if resend failed, log will be saved to specified file.


### BufferStore

BufferStore use Primary, Secondary two-level storage.
When exceptions occur in Primary storage,  kids will temporarily store log to Secondary storage.  When Primary restore,  buffered log will be stored back to Primary storage again.

Example:

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

Using MultipleStore, kids will store log file in multiple methods, the success or failure of MultipleStorage depends on value of  `success`.
`success` can be  `any` or `all`.

Example:

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
