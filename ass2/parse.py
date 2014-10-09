#!/usr/bin/env python
import re
import sys

class Result(object):
    def __init__(self, db, rate, type_):
        super(Result, self).__init__()
        self.db = db
        self.rate = rate
        self.type_ = type_

        self.avg1 = -1
        self.count1 = -1
        self.time1 = -1
        self.pgreq1 = -1

        self.avg2 = -1
        self.count2 = -1
        self.time2 = -1

        self.count3 = -1
        self.time3 = -1

    def __str__(self):
        return '{},{},{}\n{},{},{}\n{},{}'.format(
                self.avg1, self.count1, self.time1,
                self.avg2, self.count2, self.time2,
                self.count3, self.time3)

    def query1(self):
        print '{},{},{},{},{},{},{}'.format(self.type_, self.rate, self.db, self.time1, self.pgreq1, self.count1, self.avg1)

print "type,rate,db#,time,#page requests,tuple count,avg"
for type_ in ('t', 'p'):
    for rate in range(10, 101, 10):

        for db in range(10):
            f = open('results/{}-{}-{}'.format(type_, rate, db), 'r')
            log = open('results/{}-{}-{}-log'.format(type_, rate, db), 'r')
            r = Result(db, rate, type_);
            for i, line in enumerate(f):
                if i == 4:
                    parts = line.split("|")
                    r.avg1 = float(parts[0])
                    r.count1 = int(parts[1])
                if i == 7:
                    m = re.match(r"Time: (.*) ms", line)
                    r.time1 = float(m.group(1))

                if i == 10:
                    parts = line.split("|")
                    r.avg2 = float(parts[0])
                    r.count2 = int(parts[1])
                if i == 13:
                    m = re.match(r"Time: (.*) ms", line)
                    r.time2 = float(m.group(1))

                if i == 16:
                    r.count3 = int(line.strip())
                if i == 19:
                    m = re.match(r"Time: (.*) ms", line)
                    r.time3 = float(m.group(1))

            for i, line in enumerate(log):
                if i == 0:
                    m = re.match(r".*rs_nblocks = (\d+).*", line)
                    if type_ == 't':
                        r.pgreq1 = int(m.group(1))
                if i == 1:
                    m = re.match(r".*used = (\d+).*", line)
                    if type_ == 'p':
                        r.pgreq1 = int(m.group(1))-1

            r.query1()
        print ""


