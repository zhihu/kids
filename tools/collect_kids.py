import collectd
import kids


#== Our Own Functions go here: ==#
def configer(ObjConfiguration):
    collectd.debug('Configuring Stuff')


def initer():
    collectd.debug('initing kids stuff')
    collectd.register_read(reader, data=kids.Kids('localhost', 3388))


def reader(data=None):
    '''{'server_hostname': 'kids', 'kids_version': '0.7.5 64bit', 'message_drop': 0, 'message_in_per_second': 587, 'uptime_in_days': 19, 'multiplexing_api': 'epoll', 'message_out_per_second': 689, 'message_in_queue': 0, 'connected_clients': 605, 'tcp_port': 3388, 'message_in': 927541610, 'used_memory_rss': 47280, 'pubsub_patterns': 1, 'used_cpu_sys': 205719.91, 'message_store': 883639995, 'message_out': 1071595818, 'process_id': 9121, 'used_memory_rss_human': '46.16M', 'unix_socket': '/tmp/kids.sock', 'used_cpu_user': 32181.06, 'uptime_in_seconds': 2426973, 'message_buffer': 0, 'pubsub_topics': 4}'''
    kids = data
    info = kids.info()
    v = collectd.Values(type='message_in_per_second')
    v.plugin = 'kids'
    v.values = [info['message_in_per_second']]
    v.dispatch()
    v = collectd.Values(type='message_out_per_second')
    v.plugin = 'kids'
    v.values = [info['message_out_per_second']]
    v.dispatch()
    v = collectd.Values(type='connected_clients')
    v.plugin = 'kids'
    v.values = [info['connected_clients']]
    v.dispatch()
    v = collectd.Values(type='pubsub_patterns')
    v.plugin = 'kids'
    v.values = [info['pubsub_patterns']]
    v.dispatch()
    v = collectd.Values(type='pubsub_topics')
    v.plugin = 'kids'
    v.values = [info['pubsub_topics']]
    v.dispatch()


#== Hook Callbacks, Order is important! ==#
collectd.register_config(configer)
collectd.register_init(initer)
