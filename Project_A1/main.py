import subprocess
import sys

TOTAL = 1
INITIAL = 100
args = list()
for arg in sys.argv:
    if '-t' in arg:
        arg = arg.split('=')[-1]
        TOTAL = int(arg)
    elif '-h' in arg:
        arg = arg.split('=')[-1]
        INITIAL = int(arg)
    else:
        args.append(arg.replace('main', 'simulate'))

codes = [*range(INITIAL, INITIAL + TOTAL)]
command = ''
for code in codes:
    command += 'python3 ' + ' '.join(args) + f' -h={code} & ' 

subprocess.run(command[:-3] + ' -m=1', shell = True)