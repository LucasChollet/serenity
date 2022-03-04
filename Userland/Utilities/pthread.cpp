#include <AK/String.h>
#include <pthread.h>
#include <unistd.h>

static void* thread_fun(void* args)
{
    [[maybe_unused]] auto& n_args = *(String*)args;
    for (unsigned i = 0; i < 100; ++i) {
        dbgln("Simply Running({})", i);
        pthread_testcancel();
        usleep(500000);
    }
    return nullptr;
}

int main()
{
    pthread_t thread;
    String arg;
    pthread_create(&thread, NULL, thread_fun, (void*)&arg);
    int old;
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old);
    dbgln("Old Type: {}", old);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old);
    dbgln("Old State: {}", old);

    sleep(2);
    auto rc = pthread_cancel(thread);
    dbgln("rc: {}", rc);
    if (rc != 0)
        perror("pthread_cancel");

    usleep(1200000);
    dbgln("Ending");
}