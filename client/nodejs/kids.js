/**
 * Nodejs client for github.com/zhihu/kids.git
 * 2014 <hit9 hit9@icloud.com>
 *
 * Requires:
 *
 *   - Nodejs 0.8+
 *   - hiredis-node
 *
 * Subscribe:
 *
 *   var client = kids.createClient();
 *   client.subscribe('topic');
 *   client.on('data', function(error, data) {
 *     console.log(data);  // data: {topic:.., message: ..}
 *   });
 *
 * Publish (or Log):
 *
 *   var client = kids.createClient();
 *   client.log('topic', 'hello %s!', 'Mr.Developer');
 */

var events = require('events');
var net = require('net');
var util = require('util');
var hiredis = require('hiredis');


var PUBLISH      = 'PUBLISH';
var SUBSCRIBE    = 'SUBSCRIBE';
var PSUBSCRIBE   = 'PSUBSCRIBE';
var UNSUBSCRIBE  = 'UNSUBSCRIBE';
var PUNSUBSCRIBE = 'PUNSUBSCRIBE';

var MESSAGE      = 'MESSAGE';
var PMESSAGE     = 'PMESSAGE';


/**
 * @constructor
 * @param {Object} options
 */
function Connection(options) {
  this.host = options.host || '127.0.0.1';
  this.port = options.port || 3388;
  this.timeout = options.timeout || 0;

  this.sock = null;
  this.commands = [];
  this.callbacks = [];
  this._hrreader = new hiredis.Reader();
}
util.inherits(Connection, events.EventEmitter);


/**
 * Connect to kids
 *
 * @param {Function} callback
 */
Connection.prototype.connect = function(callback) {
  this.sock = net.Socket();
  this.sock.setTimeout(this.timeout);
  this.sock.setEncoding('utf8');
  this.sock.setNoDelay(true);
  this.sock.setKeepAlive(true);
  this.sock.connect(this.port, this.host, callback);

  // on data
  var self = this;
  this.sock.on('data', function(buf) {
    return self.recv(buf);
  });
};


/**
 * Send request to kids server over redis protocol (v2)
 *
 * @param {Array} args
 * @param {Function} callback
 */
Connection.prototype.send = function(args, callback) {
  var self = this;

  if (!this.sock) {
    // lazy connect
    return this.connect(function() {
      // resend on frist-connected
      self.send(args, callback);
    });
  } else {
    var args_ = [util.format('*%d\r\n', args.length)];

    for (var i = 0; i < args.length; i++) {
      var arg = util.format('%s', args[i]);
      var len = Buffer.byteLength(arg);
      args_.push(util.format('$%d\r\n', len));
      args_.push(arg);
      args_.push('\r\n');
    }
    var buf = new Buffer(args_.join(''));
    return this.sock.write(buf, function(){
      self.commands.push(args[0]);
      self.callbacks.push(callback);
    });
  }
};


/**
 * Recv response from kids server over redis protocol
 *
 * @param {Buffer/String} buf
 */
Connection.prototype.recv = function(buf) {
  this._hrreader.feed(buf);

  var res;

  while ((res = this._hrreader.get()) !== undefined) {
    this._routeResp(res);
  }
};


/**
 * Routing recv data to sub/pub
 * @private
 * @param {String} res
 */
Connection.prototype._routeResp = function(res) {
  var cmd = this.commands.shift();

  if (typeof cmd !== 'undefined') {
    switch(cmd) {
      /* jshint ignore: start */
      case PUBLISH:
      case SUBSCRIBE:
      case PSUBSCRIBE:
      case UNSUBSCRIBE:
      case PUNSUBSCRIBE:
        var err;
        var data;
        var callback = this.callbacks.shift();
        if (res.constructor === Error) err = res;
        else data = res;
        callback && callback(err, data);
        break;
      /* jshint ignore: end */
    }
  } else if (util.isArray(res) && res.length) {
    switch(res[0].toUpperCase()) {
      case MESSAGE:
      case PMESSAGE:
        this.emit('data', {topic: res[1], message: res[2]});
        break;
      default:
        // drop
    }
  }
};


/**
 * @constructor
 * @param {Object} options
 *
 * options
 *
 *   host    string, kids server host, default: '127.0.0.1'
 *   port    number, kids server port, default: 3388
 *   timeout number, ms timeout to connect
 */
function Client(options) {
  this.conn = new Connection(options);
  // register event listener
  var self = this;
  this.conn.on('data', function(data) {
    return self.emit('data', data);
  });
}
util.inherits(Client, events.EventEmitter);


/**
 * Send signle log message to kids server
 *
 * @public
 * @param {String} topic
 * @param {Mixed} args..
 * @param {Function} callback // error, data (0 or 1)
 */
Client.prototype.publish = function() {
  var callback;
  var topic = arguments[0];
  var args_ = [].slice.call(arguments, 1, -1);
  var argsl = [].slice.call(arguments, -1)[0];

  if (typeof argsl === 'function') {
    callback = argsl;
  } else {
    args_.push(argsl);
  }

  var data = util.format.apply(null, args_);
  return this.conn.send([PUBLISH, topic, data], callback);
};


/**
 * Log Alias
 */
Client.prototype.log = Client.prototype.publish;


/**
 * Subscribe topic from kids server
 *
 * @public
 * @param {String} topic
 * @param {Function} callback  // error, data (0 or 1)
 */
Client.prototype.subscribe = function(topic, callback) {
  return this.conn.send([SUBSCRIBE, topic], callback);
};


/**
 * PSubscribe topic from kids server
 *
 * @public
 * @param {String} topic
 * @param {Function} callback  // error, data (0 or 1)
 */
Client.prototype.psubscribe = function(topic, callback) {
  return this.conn.send([PSUBSCRIBE, topic], callback);
};


/**
 * UnSubscribe topic from kids server
 *
 * @public
 * @param {String} topic
 * @param {Function} callback
 */
Client.prototype.unsubscribe = function(topic, callback) {
  return this.conn.send([UNSUBSCRIBE, topic], callback);
};


/**
 * PUnSubscribe topic from kids server
 *
 * @public
 * @param {String} topic
 * @param {Function} callback
 */
Client.prototype.punsubscribe = function(topic, callback) {
  return this.conn.send([PUNSUBSCRIBE, topic], callback);
};


/**
 * @exports
 * @param {Object} options
 * @return {Client}
 */
exports.createClient = function(options) {
  return new Client(options || {});
};
