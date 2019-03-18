#!/bin/bash

pubtest_num=(0 1 2 3 4 5 6 7 8)
## Start public testing
echo "Start public testing ..."
for tno in ${pubtest_num[*]}
do
cp "test_inputs/test$tno.in" "./events.txt"

if cmp -s <(./idispatcher < "test_inputs/test$tno.in") <(cat "test_outputs/test$tno.out"); then
   echo "Test $tno passed"
else
    echo "Test $tno failed"
fi
done
## Public testing is done!
echo "Public testing is done!"

pritest_num=(9 10 11)
## Start Secret testing
echo "Start secret testing ..."
for tno in ${pritest_num[*]}
do
cp "pri_test_inputs/test$tno.in" "./events.txt"

if cmp -s <(./idispatcher < "pri_test_inputs/test$tno.in") <(cat "pri_test_outputs/test$tno.out"); then
   echo "Test $tno passed"
else
    echo "Test $tno failed"
fi
done
## Secret testing is done!
echo "Secret testing is done!"

