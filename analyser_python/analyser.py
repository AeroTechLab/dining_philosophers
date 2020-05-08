import re
import numpy as np

file_pre_name = 'logPhil'

n_philosofers = 9
n_forks = n_philosofers
n_lines = 9500

file_indexes = [i for i in range(n_philosofers)]

file_extension = '.txt'

file = [0] * n_philosofers

logs = []
logs_times = []

for l in range(n_philosofers):
    logs.append([])
    logs_times.append([])
    for l_idx in range(n_lines):
        logs[l].append('')
        logs_times[l].append('')

final_logs = []
final_times = []
forks_owner = []

for l in range(n_lines * n_philosofers):
    final_logs.append([])
    final_times.append([])
    forks_owner.append([])
    for l_idx in range(n_philosofers):
        final_logs[l].append('')
        forks_owner[l].append([])

line_counter = 0

time_regexp = re.compile('\d+')
phil_regexp = re.compile('p\d+')
fork_regexp = re.compile('f(\d+)')
got_regexp = re.compile('got')
drop_regexp = re.compile('drop')

max_line_idx = [0] * n_philosofers

for f_idx in file_indexes:
    file_name = file_pre_name + str(f_idx) + file_extension
    file = open(file_name, 'r')
    for l_idx, line in enumerate(file):
        if not line =='\n':
            time_slice = time_regexp.match(line)
            logs_times[f_idx][l_idx] = int(line[slice(*time_slice.span())])
            logs[f_idx][l_idx] = line[time_slice.span()[1]:].strip()
            if l_idx == (n_lines-1):
                break
        else:
            max_line_idx[f_idx] = l_idx
            break


line_indexes = []
live_phil_logs_idx = []

for philosofer_log in logs:
    if len(philosofer_log) > 0:
        live_phil_logs_idx.append(True)
        line_indexes.append(0)

current_forks_owners = [''] * n_forks

line_indexes = np.asarray(line_indexes)

while not np.sum(live_phil_logs_idx) == 0:
    current_lines = []
    current_times = []

    for phil_idx in range(len(live_phil_logs_idx)):
        if live_phil_logs_idx[phil_idx]:
            current_lines.append(logs[phil_idx][line_indexes[phil_idx]])
            current_times.append(logs_times[phil_idx][line_indexes[phil_idx]])
        else:
            current_lines.append('nadegas')
            current_times.append(np.inf)

    current_times = np.asarray(current_times)
    current_phil_idx = np.argmin(current_times)

    current_fork_idx = -1

    result_got = got_regexp.search(current_lines[current_phil_idx])
    if result_got:
        current_fork_idx = int(fork_regexp.search(current_lines[current_phil_idx]).groups()[0])
        current_forks_owners[current_fork_idx] = 'p' + str(current_phil_idx)

    result_drop = drop_regexp.search(current_lines[current_phil_idx])
    if result_drop:
        current_fork_idx = int(fork_regexp.search(current_lines[current_phil_idx]).groups()[0])
        current_forks_owners[current_fork_idx] = '-p' + str(current_phil_idx)

    forks_owner[np.sum(line_indexes)] = current_forks_owners[:]

    if result_drop:
        current_forks_owners[current_fork_idx] = ''

    final_logs[np.sum(line_indexes)][current_phil_idx] = current_lines[current_phil_idx]
    final_times[np.sum(line_indexes)] = int(current_times[current_phil_idx])
    line_indexes[current_phil_idx] = line_indexes[current_phil_idx] + 1

    if (line_indexes[current_phil_idx] == n_lines or line_indexes[current_phil_idx] == max_line_idx[current_phil_idx] ):
        live_phil_logs_idx[current_phil_idx] = False
        print(live_phil_logs_idx)

header = ' '*20 + ' '*15* n_philosofers + '|' + ''.join(['{:>6}'.format('f'+str(fork_idx)) for fork_idx in range(n_forks)])

with open('final_log.txt','w') as f_final_logs:
    f_final_logs.write(header + '\n')
    for t,l,fo in zip(final_times, final_logs, forks_owner):
        f_final_logs.write('{:20}'.format(str(t)))
        for e in l:
            f_final_logs.write('{:>15}'.format(e))

        f_final_logs.write('|')
        for o in fo:
            f_final_logs.write('{:>6}'.format('' if o == [] else o))

        f_final_logs.write('\n')
