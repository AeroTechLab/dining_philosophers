import re
import numpy as np

# begin of each filename
file_pre_name = 'logPhil'

# amount of philosofers that was dining
n_philosofers = int(input("How much philosofers?\n"))
# same amount of forks
n_forks = n_philosofers

# amount of lines in each philosofer log file
n_lines = int(input("What is the maximum amount of lines in each of the {} files?\n".format(n_philosofers)))

# files index
file_indexes = [i for i in range(n_philosofers)]

# expected extension for the logs
file_extension = '.txt'

# preallocating for the file list
file = [0] * n_philosofers

# pre allocating for log (text) and log_times (duh)
logs_event = []
logs_times = []

for event in range(n_philosofers):
    logs_event.append([])
    logs_times.append([])
    for l_idx in range(n_lines):
        logs_event[event].append('')
        logs_times[event].append(0)

# pre allocating for log (text) and log_times (duh)
final_events = []     # this is a timeline: stores in each element the event of each of the final_log.txt
final_times = []    # this is a timeline: stores in each element the time of each of the final_log.txt
forks_owners = []    # this is a timeline: stores in each element the list of the owners of each fork

for event in range(n_lines * n_philosofers):
    final_events.append([])
    final_times.append([])
    forks_owners.append([])
    for l_idx in range(n_philosofers):
        final_events[event].append('') # preallocated with empty string
        forks_owners[event].append([])

time_regexp = re.compile('\d+')     # regexp to find at start of each line of logPhil\d+.txt
phil_regexp = re.compile('p\d+')    # regexp to find philosofer
fork_regexp = re.compile('f(\d+)')  # regexp to find fork
got_regexp = re.compile('got')      # regexp to find got
drop_regexp = re.compile('drop')    # regexp to find drop

max_line_idx = [0] * n_philosofers  # stores the last line index of each logPhil log

for f_idx in file_indexes:
    file_name = file_pre_name + str(f_idx) + file_extension                     # creating the filename string
    with open(file_name, 'r') as file:                                          # opening the file
        for l_idx, line in enumerate(file):                                     # for each line index and line itself
            if not line =='\n':                                                 # if not empty line
                time_slice = time_regexp.match(line).span()                     # find time slice position
                logs_times[f_idx][l_idx] = int(line[slice(*time_slice)])        # using slice to extract the int(time)
                logs_event[f_idx][l_idx] = line[time_slice[1]:].strip()         # stores remaining text (stripped) in log
                if l_idx == (n_lines-1): # if this is the last line, break the analysis of this file
                    break
            else:                                                               # if empyt string
                max_line_idx[f_idx] = l_idx                                     # mark end of file
                break

# each file progrides differently, so a list of current line indexes: a index for each file
line_indexes = []
# as each file progrides differently, one file may end before others, so keep track of which files are still "alive"
live_phil_logs_idx = []

# preallocating this stuff
for philosofer_log in logs_event:
    if len(philosofer_log) > 0:
        live_phil_logs_idx.append(True)
        line_indexes.append(0)

# converting to ndarray (to apply np.sum)
line_indexes = np.asarray(line_indexes)

# current owner of each fork
current_forks_owners = [''] * n_forks

# start of  time
time_zero = 0

# mark if is first iteration or not (to get the time_zero of the logs)
first_iteration = True

last_time = []
last_events = []
last_fowners = []
last_phil_idx = []

# while there is at least one logPhil alive
while any(live_phil_logs_idx) == True:
    # emptying current stuff
    current_lines = []
    current_times = []
    current_final_line = np.sum(line_indexes)
    # for each philosofer
    for phil_idx in range(len(live_phil_logs_idx)):
        if live_phil_logs_idx[phil_idx]:                                        # if this philosofer's log is alive
            current_lines.append(logs_event[phil_idx][line_indexes[phil_idx]])  # get its event
            current_times.append(logs_times[phil_idx][line_indexes[phil_idx]])  # get its event's time
        else:                                                                       # if not alive
            current_lines.append('')                                            # get nothing
            current_times.append(np.inf)                                        # get infinite time (low time is important here)

    current_times = np.asarray(current_times)   # converts to ndarray
    current_phil_idx = np.argmin(current_times) # converts to ndarray

    if first_iteration:
        time_zero = np.min(current_times)       # stores the time zero
        first_iteration = False                 # unmarking first_iteration

    current_times = current_times - time_zero   # timing w.r.t first time

    # stores the currently affect fork. -1 means that no fork was affected
    current_fork_idx = -1

    # searching  for a gotten fork
    result_got = got_regexp.search(current_lines[current_phil_idx])

    # if a fork was gotten
    if result_got:
        # gets its id
        current_fork_idx = int(fork_regexp.search(current_lines[current_phil_idx]).groups()[0])
        # gets its owner
        current_forks_owners[current_fork_idx] = '+p' + str(current_phil_idx)

        result_drop = None
    else:
        # searching for a dropped fork
        result_drop = drop_regexp.search(current_lines[current_phil_idx])
        # if a fork was dropped
        if result_drop:
            # gets its id
            current_fork_idx = int(fork_regexp.search(current_lines[current_phil_idx]).groups()[0])
            # gets its dropper owner (marked with a minus sign)
            current_forks_owners[current_fork_idx] = '-p' + str(current_phil_idx)


    # writing down current fork owners
    forks_owners[current_final_line] = current_forks_owners[:]

    # write down to final_logs and final_times
    final_events[current_final_line][current_phil_idx] = current_lines[current_phil_idx]
    final_times[current_final_line] = int(current_times[current_phil_idx])

    # sometimes drops and gots are in inverted positions. this corrects it
    # if this is a drop AND
    # if last was a got AND
    # if same fork as before AND
    # if not first iteration AND
    # if same time as before
    if result_drop \
        and last_result_got_fork \
        and last_fork_idx == current_fork_idx \
        and not first_iteration \
        and final_times[current_final_line] == last_time:
            aux = (final_times[current_final_line], final_events[current_final_line][:], forks_owners[current_final_line][:])
            final_times[current_final_line], final_events[current_final_line], forks_owners[current_final_line] = last_time, last_events[:], last_fowners[:]
            final_times[current_final_line-1], final_events[current_final_line-1], forks_owners[current_final_line-1] = aux
            current_forks_owners = forks_owners[current_final_line][:]
            result_drop = None

    # after this, erase the dropped fork owners
    if result_drop:
        current_forks_owners[current_fork_idx] = ''

    if result_got:
        current_forks_owners[current_fork_idx] = current_forks_owners[current_fork_idx][1:]

    # storing interesting things from
    # this iteration (future's last iteration) for
    # the next (future's current iteration)
    last_time = final_times[current_final_line]
    last_events = final_events[current_final_line][:]
    last_fowners = forks_owners[current_final_line][:]
    last_phil_idx = current_phil_idx
    last_result_got_fork = result_got
    last_fork_idx = current_fork_idx

    # increment consumed line of the current philosofer's log
    line_indexes[current_phil_idx] = line_indexes[current_phil_idx] + 1

    # if current logPhil reached its maximum lines
    if (line_indexes[current_phil_idx] == n_lines or line_indexes[current_phil_idx] == max_line_idx[current_phil_idx] ):
        # mark logPhil as not alive
        live_phil_logs_idx[current_phil_idx] = False
        # prints it
        print(live_phil_logs_idx)

# header for the finel_log
header = ' '*20 + ' '*15* n_philosofers + '|' + ''.join(['{:>6}'.format('f'+str(fork_idx)) for fork_idx in range(n_forks)])

# text that will be writen in the final_log
final_log_text = []

# writes header to it
final_log_text.append(header)

# walking by time, event and forks' owners
for time, event, fo in zip(final_times, final_events, forks_owners):
    # writing time in the begining
    final_log_text.append('{:>20}'.format(str(time)))

    # writing event of this time
    for e in event:
        final_log_text[-1] = final_log_text[-1] + '{:>15}'.format(e)

    # spacing
    final_log_text[-1] = final_log_text[-1] + '|'

    # writing forks owners
    for o in fo:
        final_log_text[-1] = final_log_text[-1] + '{:>6}'.format('' if o == [] else o)

# writing to file for real
with open('final_log_novo.txt','w') as f_final_logs:
    # header first
    f_final_logs.write(final_log_text[0] + '\n')

    # others lines
    for i in range(1, len(final_log_text)):
        # if same event (like try) don't write it
        if not final_log_text[i][20:] == final_log_text[i-1][20:]:
            f_final_logs.write(final_log_text[i] + '\n')
