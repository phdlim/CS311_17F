#!/bin/bash
PRJ_TEST_DIR="testdir"

if [ ! -f ./runfile ]; then
  make
fi

if [ ! -f $PRJ_TEST_DIR ]; then
  mkdir $PRJ_TEST_DIR
fi

for i in 1 2 3 4 5
do
  echo "$PRJ_TEST_DIR/example$i.s"
  cp Project1_examples/example$i*.s $PRJ_TEST_DIR/example$i.s
  cp Project1_examples/example$i*.o $PRJ_TEST_DIR/rexample$i.o
  if [ -f $PRJ_TEST_DIR/example$i.o ]; then
    rm $PRJ_TEST_DIR/example$i.o
  fi
  ./runfile $PRJ_TEST_DIR/example$i.s
  RESULT=`diff --ignore-all-space $PRJ_TEST_DIR/example$i.o $PRJ_TEST_DIR/rexample$i.o`
  if [ "$RESULT" == "" ]; then
    echo "Good!"
  else
    echo "BAD!"
  fi
done

rm -rf "$PRJ_TEST_DIR"
make clean
