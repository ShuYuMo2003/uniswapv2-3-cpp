ret = []
with open('pool_events_test_', 'r') as f:
    for line in f.readlines():
        d = line.split()
        if(d[0] == 'swap'): ret.append([d[2], d[3]])

with open('pool_swap_events', 'w') as f:
    for d in ret:
        f.write(' '.join(d) + '\n')