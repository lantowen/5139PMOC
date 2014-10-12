#!/usr/bin/env python
import re
import sys
import numpy as np

if len(sys.argv) != 2:
    sys.stderr.write("Usage: python parse.py query#\n");
    exit()

query_num = int(sys.argv[1])

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
        self.pgreqr3 = -1
        self.pgreqs3 = -1

    def __str__(self):
        return '{},{},{}\n{},{},{}\n{},{}'.format(
                self.avg1, self.count1, self.time1,
                self.avg2, self.count2, self.time2,
                self.count3, self.time3)

    def query1(self):
        typenum = 1
        if self.type_ == "t":
            typenum = 0
        print '{},{},{},{},{},{},{}'.format(typenum, self.rate, self.db, self.time1, self.pgreq1, self.count1, self.avg1)

    def query3(self):
        typenum = 1
        if self.type_ == "t":
            typenum = 0
        print '{},{},{},{},{},{},{}'.format(typenum, self.rate, self.db, self.time3, self.pgreqr3, self.pgreqs3, self.count3)

if query_num == 1:
    print "type,rate,db#,time,#page requests,tuple count,avg"
elif query_num == 3:
    print "type,rate,db#,time,#page requests R, #page requests S,tuple count"
for type_ in ('t', 'p'):
    for rate in range(10, 101, 10):

        dbresults = list()

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

                if i == 4:
                    m = re.match(r".*rs_nblocks = (\d+).*", line)
                    if type_ == 't':
                        r.pgreqr3 = int(m.group(1))
                if i == 5:
                    m = re.match(r".*used = (\d+).*", line)
                    if type_ == 'p':
                        r.pgreqr3 = int(m.group(1))-1

                if i == 6:
                    m = re.match(r".*rs_nblocks = (\d+).*", line)
                    if type_ == 't':
                        r.pgreqs3 = int(m.group(1))
                if i == 7:
                    m = re.match(r".*used = (\d+).*", line)
                    if type_ == 'p':
                        r.pgreqs3 = int(m.group(1))-1

            dbresults.append(r)
            #if query_num == 1:
                #r.query1()
            #elif query_num == 3:
                #r.query3()
        #print ""
        res = list()
        if query_num == 1:
            res.append(dbresults[0].type_)
            res.append(str(dbresults[0].rate))
            res.append(str(np.median([x.time1 for x in dbresults])))
            res.append(str(np.mean([x.pgreq1 for x in dbresults])))
            res.append(str(np.mean([x.count1 for x in dbresults])))
            res.append(str(np.mean([x.avg1 for x in dbresults])))

        elif query_num == 3:
            res.append(dbresults[0].type_)
            res.append(str(dbresults[0].rate))
            res.append(str(np.median([x.time3 for x in dbresults])))
            res.append(str(np.mean([x.pgreqr3 for x in dbresults])))
            res.append(str(np.mean([x.pgreqs3 for x in dbresults])))
            res.append(str(np.mean([x.count3 for x in dbresults])))
        print '\t'.join(res)



