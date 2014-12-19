#! /usr/bin/env python
# coding=utf-8
import redis
from multiprocessing import Process, Queue
from time import sleep

KIDS_HOST = {'host': 't-infra', 'port': 3389}

def publisher(topics):
    red = redis.StrictRedis(**KIDS_HOST)
    i = 0
    while True:
        for topic in topics:
            red.publish(topic, topic)
            i += 1


def sub_msg(topic):
    red = redis.StrictRedis(**KIDS_HOST)
    red = red.pubsub()
    red.subscribe(topic)
    for item in red.listen():
        pass

def psub_msg(pattern):
    red = redis.StrictRedis(**KIDS_HOST)
    red = red.pubsub()
    red.psubscribe(pattern)
    for item in red.listen():
        pass


def main():
    pub_num = 300
    sub_num = 10
    topic_num = 10
    psub_num = 3
    topics = []

    for i in range(0, topic_num):
        t = 'topic' + str(i)
        topics.append(t)

    subers = []
    psubers = []
    pubers = []
    red = redis.StrictRedis(**KIDS_HOST)

    for i in range(0, sub_num):
        p = Process(target=sub_msg, args=(topics[i],))
        p.start()
        subers.append(p)

    for i in range(0, psub_num):
        p = Process(target=psub_msg, args=('*',))
        p.start()
        psubers.append(p)

    print("all subscribers have connected")
    for i in range(0, pub_num):
        p = Process(target=publisher, args=(topics,))
        p.start()
        pubers.append(p)

    print("all publishers have connected")

    binfo = red.info()

    while True:
        sleep(5)
        nowinfo = red.info()
        time_pass = int(nowinfo['uptime_in_seconds']) - int(binfo['uptime_in_seconds'])
        print 'time passed =%10d' % time_pass,
        print 'QPS = %10d' % ((int(nowinfo['message_in']) - int(binfo['message_in'])) / time_pass),
        print 'QPS from server =%10d' % nowinfo['message_in_per_second']


if __name__ == '__main__':
    main()
