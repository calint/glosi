#!/bin/bash
set -e
cd $(dirname "$0")
DIR="$(pwd)"

slptm=${1:-60}

cd ../.. && ./glos 1> /dev/null 2> "$DIR/metrics.txt" &

echo "running $slptm seconds" &&
sleep $slptm &&
echo done waiting $slptm &&

pid=$(ps -u | grep glos | head -n1 | awk '{print $2;}') &&
kill $pid &&

cd "$DIR" &&

glxinfo | grep -i version > metrics.meta &&

./fps.plot
./update.plot
