#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // Sleep for wait_to_obtain_ms duration
    usleep(thread_func_args->wait_to_obtain_ms * 1000); // Multiply by 1000 to convert ms to us

    // Attempt to lock the mutex
    if (pthread_mutex_lock(thread_func_args->mutex) != 0) {
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // Sleep for wait_to_release_ms duration
    usleep(thread_func_args->wait_to_release_ms * 1000); // Multiply by 1000 to convert ms to us

    // Attempt to unlock the mutex
    if (pthread_mutex_unlock(thread_func_args->mutex) != 0) {
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    thread_func_args->thread_complete_success = true;
    return thread_param;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data *data = malloc(sizeof(struct thread_data));
    if (data == NULL) {
        ERROR_LOG("Failed to allocate memory for thread data");
        return false;
    }

    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;

    if (pthread_create(thread, NULL, threadfunc, data) != 0) {
        ERROR_LOG("Failed to create thread");
        free(data);
        return false;
    }

    return true;
}
