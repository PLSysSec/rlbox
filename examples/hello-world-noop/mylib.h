#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    void hello();
    void echo(const char* str);
    unsigned add(unsigned, unsigned);
    void call_cb(void (*cb) (const char* str));
#ifdef __cplusplus
}
#endif
