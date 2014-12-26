var kids = require('../kids');
var client = kids.createClient();


for (var i = 0; i < 1000; i ++) {
  client.publish('topic', 'the message no %d', i);
}
