import re
import sys

filename = sys.argv[1]
f = open(filename, 'r')
content = f.read()
f.close()

totalTimes = re.findall(r'Protocol Total:\s+([0-9.]+)s', content)
print(max(totalTimes))