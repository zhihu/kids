#! /usr/bin/env python
# coding=utf-8
import redis
from multiprocessing import Process, Queue
import unittest

KIDS_HOST = {'host': 'localhost', 'port': 3388}
num_process = 10

def subscriber(queue,topics):
    red = redis.StrictRedis(**KIDS_HOST)
    red = red.pubsub()
    for topic in topics:
        red.subscribe(topic)
        # skip the reply
        for msg in red.listen():
            break

    for reply in red.listen():
        queue.put(reply)

def subscribe(topics):
    subs = []
    sub_queues = []

    for i in range(0, num_process):
        queue = Queue()
        p = Process(target=subscriber, args=(queue, topics,))
        p.start()
        sub_queues.append(queue)
        subs.append(p)

    return (subs, sub_queues)

def psubscribe(pattern):
    red = redis.StrictRedis(**KIDS_HOST)
    red = red.pubsub()
    red.psubscribe(pattern)
    return red


def publisher(messages):
    red = redis.StrictRedis(**KIDS_HOST)
    for i in range(len(messages)):
        red.publish(messages[i][0], messages[i][1])


class TestKids(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_get_right_message(self):
        """Assert I do not get message I did not subscribe."""

        subs, sub_queues = subscribe(['topic 1'])

        messages = [('topic 2','data 2'), ('topic 1','data 1')]
        for i in range(0, num_process):
            p = Process(target=publisher, args=(messages,))
            p.start()
            p.join()

        for i in range(0, num_process):
            # there are num_process subscribers
            for j in range(0, num_process):
                # every subscriber receives num_process messages
                msg = sub_queues[i].get()
                self.assertEqual('topic 1', msg['channel'])
                self.assertEqual('data 1', msg['data'])

        for i in range(0, num_process):
            subs[i].terminate()

    def test_get_all_message(self):
        """Assert no message is lost."""

        subs, sub_queues = subscribe(['topic 1', 'topic 2'])
        messages = []
        for i in range(0, 100):
            messages.append(('topic 3', 'data 3'))
            messages.append(('topic 1', 'topic 1:'+str(i)))
            messages.append(('topic 2', 'topic 2:'+str(i)))

        for i in range(0, num_process):
            p = Process(target=publisher, args=(messages,))
            p.start()
            p.join()

        for i in range(0, num_process):
            for j in range(0, 100):
                msg1 = sub_queues[i].get()
                msg2 = sub_queues[i].get()
                self.assertEqual('topic 1', msg1['channel'])
                self.assertEqual('topic 1:'+str(j), msg1['data'])
                self.assertEqual('topic 2', msg2['channel'])
                self.assertEqual('topic 2:'+str(j), msg2['data'])

        for i in range(0, num_process):
            subs[i].terminate()

    def test_unsubscribe(self):
        red = redis.StrictRedis(**KIDS_HOST)
        red = red.pubsub()
        red.subscribe(['topic 1', 'topic 2'])

        red.unsubscribe(['topic 1'])
        i = 0
        for msg in red.listen():
            i += 1
            if i == 3:
                break

        publisher([('topic 1', 'data 1'), ('topic 2', 'data 2')])
        for msg in red.listen():
            self.assertEqual('topic 2', msg['channel'])
            self.assertEqual('data 2', msg['data'])
            break


if __name__ == '__main__':
    unittest.main()
