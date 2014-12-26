var kids = require('../kids');
var client = kids.createClient();

client.on('data', function(data) {
  console.log('topic:', data.topic, 'message:', data.message);
});

client.subscribe('topic', function(err, data) {
  if (err) {
    console.log(err.message);
  } else {
    console.log('subscribe success');
  }
});
