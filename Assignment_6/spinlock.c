#include "spinlock.h"

void spin_lock(struct spinlock *lock_arg){
								while(tas(&(lock_arg->p_lock))!=0) ;
}

void spin_unlock(struct spinlock *lock_arg){
								lock_arg->p_lock=0;
}
