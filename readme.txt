Group Elements:
  - Daniel Filipe Santos Marques          - up201503822
  - João Francisco Barreiros de Almeida   - up201505866
  - João Nuno Fonseca Seixas              - up201505648

Implementation Details:
  The thread implementation was POSIX's (pthread) and to avoid race conditions between them we used its mutex API.
First, the mutex variable had to be declared and initialized: 'static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;'.
Then, every time there was the need to guarantee multiple exclusion we wrote the critical zone in between the lock call 'pthread_mutex_lock(&mutex);'
and the unlock call 'pthread_mutex_unlock(&mutex);'.
  Since when a user from the same gender as the current users' had to wait for a free spot when there was none, there was a need to constantly know whether
or not there was already a free spot. To avoid busy waiting we decided to implement a condition variable.
First, the condition variable had to be declared and initialized: 'static pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;'.
Then, when this situation happened we locked the mutex, waited for the condition variable to be signaled using the wait call
'pthread_cond_wait(&cond_var,&mutex);' and, finally, unlocked the mutex.
For this to properly work, the condition variable had to be signaled elsewhere by another thread using the signal call 'pthread_cond_signal(&cond_var);',
which was accomplished when a user left (which is the other threads' responsibility).
