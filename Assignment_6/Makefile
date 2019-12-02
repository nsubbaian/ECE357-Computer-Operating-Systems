ALL: testSetSpinlock testFifo

testSetSpinlock: testSetSpinlock.c  spinlock.h spinlock.c tas64.s tas.h
	gcc -o testSetSpinlock spinlock.c testSetSpinlock.c tas64.s tas.h

testFifo:testFifo.c sem.c sem.h fifo.c fifo.h tas64.s spinlock.c
	gcc -o testFifo testFifo.c sem.c fifo.c  tas64.s spinlock.c
