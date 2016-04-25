#include "concurrence.hpp"
#include "globals.hpp"
#include "except/exceptions.hpp"
#include "utils/logging.hpp"

using namespace cerb;

ListenThread::ListenThread(int listen_port)
    : _proxy(new Proxy(listen_port))
    , _thread(nullptr)
    , _mem_buffer_stat(nullptr)
{}

void ListenThread::run()
{
    this->_thread.reset(new std::thread(
        [this]()
        {
            _mem_buffer_stat = &cerb_global::allocated_buffer;
            try {
                poll::pevent events[poll::MAX_EVENTS];
                while (true) {
                    int nfds = poll::poll_wait(this->_proxy->epfd, events, poll::MAX_EVENTS, -1);
                    this->_proxy->handle_events(events, nfds);
                }
            } catch (SystemError& e) {
                LOG(ERROR) << "Unexpected error";
                LOG(ERROR) << e.stack_trace;
                LOG(ERROR) << "Terminated by SystemError: " << e.what();
                exit(1);
            } catch (std::runtime_error& e) {
                LOG(FATAL) << "Terminated by runtime error: " << e.what();
                exit(1);
            }
        }));
}

void ListenThread::join()
{
    this->_thread->join();
}
