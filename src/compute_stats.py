import sys
import json

from collections import defaultdict

objs = []
lines = []

groups = defaultdict(list)
data = defaultdict(dict)

for line in sys.stdin:
    quotefix = '"'.join('"'.join(line.split("'")).split('"'))
    for item in quotefix.split('}'):
        if len(item.strip()) > 0:
            item = item + '}'
            lines.append(item)
            objs.append(json.loads(item))

for obj in objs:
    if 'gid' in obj and 'tid' in obj:
        groups[(obj['gid'], obj['tid'])].append(obj)

for k, v in groups.items():
    for t in v:
        for tk, tv in t.items():
            if tk not in ['msg']:
                data[k][tk] = tv

print('[')
print(','.join([json.dumps(v) for v in data.values()]))
print(']')
    