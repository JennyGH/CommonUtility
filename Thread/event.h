#ifndef _EVENT_H_
#define _EVENT_H_
class event
{
public:
    enum
    {
        timeout = -2,
        fail    = -1,
        success = 0
    };

public:
    event(bool bManualReset = false, bool bInitState = false);
    ~event();
    int  wait();
    int  wait(unsigned long milliseconds);
    void notify(int code = event::success);
    void reset();
    void destroy();

private:
    void* m_handle;
    int   m_code;
};
#endif // !_EVENT_H_