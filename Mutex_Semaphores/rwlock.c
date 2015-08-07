

#include"rwlock.h"

node_t *create_node( int type ){
  int rc;
  node_t *node = malloc( sizeof(node_t) );
  if( node == NULL ){ printf( "** create node failed\n" ); exit( -1 ); }
  rc = sem_init( &(node->waiting), 0, 0 );
  if( rc ){ printf( "** init node->sem failed\n" ); exit( -1 ); }
  node->is_writer = type;
  node->next = NULL;
  return node;
}

void destroy_node( node_t *node ){
  sem_destroy( &(node->waiting) );
  free( node );
}

fcfsrwlock_t *create_lock(){
  int rc;
  fcfsrwlock_t *lock = malloc( sizeof(fcfsrwlock_t) );
  if( lock == NULL ){ printf( "** create lock failed\n" ); exit( -1 ); }
  rc = sem_init( &(lock->mutex), 0, 1 );
  if( rc ){ printf( "** init lock->sem failed\n" ); exit( -1 ); }
  lock->is_writer = 0;
  lock->read_count = 0;
  lock->head = lock->tail = NULL;
  return lock;
}

void destroy_lock( fcfsrwlock_t *lock ){
  node_t *ptr;
  sem_destroy( &(lock->mutex) );
  while( lock->head != NULL ){
    ptr = lock->head;
    lock->head = lock->head->next;
    destroy_node( ptr );
  }
  free( lock );
}

// helper function called within a CS
// - assumes caller using mutex semaphore for mutual exclusion
//    (i.e., function is called from inside a wait/signal block)
// - creates a node with is-writer field set to type and waiting
//   semaphore initialized to zero, and places node on tail of
//   queue
node_t *add_node( fcfsrwlock_t *lock, int type ){
  node_t *new = create_node(type);
  if(lock->tail == NULL){
    lock->head = new;
    lock->tail = new;
  }
  else{
    lock->tail->next = new;
    lock->tail = new;
  }
  return new;
}

// helper function called within a CS
// - assumes caller using mutex semaphore for mutual exclusion
//    (i.e., function is called from inside a wait/signal block)
// - immediately returns if queue is empty, otherwise removes node
//   from head of list, sets is_writer field of lock to is_writer
//   value of node, increments read_count of lock if a reader node,
//   and signals the semaphore in the node to resume blocked thread
//   (note: the resumed thread has responsibility to destroy the node)
// - if the node was a reader and while there are additional reader
//   nodes at the head of the queue, repeat the process of removing
//    a reader node, incrementing read_count, and signaling
void wakeup_waiting_threads( fcfsrwlock_t *lock ){
  int opflag = 1;
  while(opflag == 1){
    opflag = 0;
    if(lock->head == NULL){
      return;
    }
    else{
      node_t *tmp = lock->head;
      lock->is_writer = tmp->is_writer;
      lock->head = lock->head->next;
      if(lock->is_writer == 0){
        opflag = 1;
        lock->read_count++;
      }
      if(lock->head == NULL){
        opflag = 0;
        lock->tail = NULL;
      }
      else{
        if(lock->head->is_writer != 0){
          opflag = 0;
        }
      }
      if(sem_post(&(tmp->waiting)) != 0){
        printf("Error using sem_post in wakeup_waiting_threads()\n");
        exit(0);
      }
    }
  }	
}

// protocol for starting a read
// - wait on mutex semaphore
// - if there is no current writer and nothing in queue, then
//   increment read_count of lock, signal mutex semaphore, and
//   return
// - otherwise, add a node to the end of the queue, signal mutex
//   semaphore, and wait on sempahore in node; when signaled,
//   destroy the node and return
void get_read_access( fcfsrwlock_t *lock ){
  if(lock->is_writer == 0 && lock->head == NULL){
    lock->read_count++;
    sem_post(&(lock->mutex));
  }
  else{
    node_t *new = add_node(lock, 0);
    sem_post(&(lock->mutex));
    sem_wait(&(new->waiting));
    destroy_node(new);
  }
}

// protocol for ending a read
// - wait on mutex semaphore
// - decrement read_count and if read_count is zero then call
//   wakeup_waiting_threads() to signal any waiting threads
// - signal mutex semaphore
void release_read_access( fcfsrwlock_t *lock ){
  sem_wait(&(lock->mutex));
  lock->read_count--;
  if(lock->read_count == 0){
     wakeup_waiting_threads(lock);
  }
  sem_post(&(lock->mutex));  
}

// protocol for starting a write
// - wait on mutex semaphore
// - if there is no current writer or current reader(s), then
//   set is_writer field of lock, signal mutex semaphore, and
//   return
// - otherwise, add a node to the end of the queue, signal mutex
//   semaphore, and block on sempahore in node; when signaled,
//   destroy the node and return
void get_write_access( fcfsrwlock_t *lock ){
  sem_wait(&(lock->mutex));
  if(lock->is_writer == 0 && lock->read_count == 0){
    lock->is_writer = 1;
    sem_post(&(lock->mutex));
  }
  else{
    node_t *new = add_node(lock, 1);
    sem_post(&(lock->mutex));
    sem_wait(&(new->waiting));
    destroy_node(new);
  }
}

// protocol for ending a write
// - wait on mutex semaphore
// - reset is_writer field of lock and call wakeup_waiting_threads()
//   to signal any waiting threads
// - signal mutex semaphore
void release_write_access( fcfsrwlock_t *lock ){
  sem_wait(&(lock->mutex));
  lock->is_writer = 0;
  wakeup_waiting_threads(lock);
  sem_post(&(lock->mutex)); 
}
