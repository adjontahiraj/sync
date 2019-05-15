# sync
In this project, I began by getting the lock() and unlock() functions to work by leveraging
the sigprocmask function. After doing this I started to work on the pthread_join function.
In this functon I used the states of SUSPENDED and TERMINATED to get the handler to skip
over the threads that were done or waiting on other threads.

After getting this function to pass the test cases is where I ran into some issues with the 
semaphore function especially the sem_wait function. I created a struct for the semaphore and used that struct to keep track
of the values and ids and held it in the sem->align. For some reason my sem_wait function does not work propperly and after spending
hours trying to fix it, I could not. 


