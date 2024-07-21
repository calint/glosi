#!/bin/sh
# enable prof for users
#  :: echo -1 | sudo tee /proc/sys/kernel/perf_event_paranoid
# -1
#  :: cat /proc/sys/kernel/perf_event_paranoid
# -1
#  :: echo 0 | sudo tee /proc/sys/kernel/kptr_restrict
# 0
#  :: cat /proc/sys/kernel/kptr_restrict
# 0

perf record --delay 3000 -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./glos
perf report
