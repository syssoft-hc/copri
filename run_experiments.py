# Execute a given set of experiments

import os
import sys
import time

from datetime import datetime

limits = [ f*10**e for e in range(3,9) for f in (1,2,5) ] + [ 1000000000 ]

def buildFilename ( c ):
    return datetime.now().strftime("%Y%m%d-%H%M%S")+"_"+c+".csv"

def cli ( c, *args ):
    l = "./" + c
    for a in args:
        l += " " + str(a)
    return l

def runExperiment ( c, limits ):
    fname = buildFilename(c)
    with open(fname, 'w') as f:
        for l in limits:
            command = cli(c,l)
            print(command)
            s = os.popen(command)
            lines = s.read()
            print(lines)
            f.write(lines)
            f.flush()
            time.sleep(5)

if __name__ == "__main__":
    for c in sys.argv[1:]:
        if os.path.isfile(c):
            runExperiment(c,limits)
        else:
            print(f'No file <{c}>, skipping ...')

