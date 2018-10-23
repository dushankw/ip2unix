// SPDX-License-Identifier: LGPL-3.0-only
#ifndef IP2UNIX_REALCALLS_HH
#define IP2UNIX_REALCALLS_HH

#include <mutex>

#include <unistd.h>
#include <dlfcn.h>

/* Let's declare all of the wrappers as extern, so that we can define them by
 * simply overriding IP2UNIX_REALCALL_EXTERN before the #include directive.
 */
#ifndef IP2UNIX_REALCALL_EXTERN
#define IP2UNIX_REALCALL_EXTERN extern
#endif

extern std::mutex g_dlsym_mutex;

/* This namespace is here so that we can autogenerate and call wrappers for C
 * library functions in a convenient way. For example to call the wrapper for
 * close we can just use real::close(fd).
 */
namespace real {
    template <typename Self, typename Ret, typename ... FunArgs>
    struct DlsymFun
    {
        Ret (*fptr)(FunArgs ...) = nullptr;

        template <typename ... Args>
        auto operator()(Args ... args) -> decltype(fptr(args ...))
        {
            g_dlsym_mutex.lock();
            if (this->fptr == nullptr) {
                void *result = dlsym(RTLD_NEXT, Self::fname);
                if (result == nullptr) {
                    std::string msg("dlsym(" + std::string(Self::fname) + ")");
                    perror(msg.c_str());
                    g_dlsym_mutex.unlock();
                    _exit(EXIT_FAILURE);
                }
                this->fptr = reinterpret_cast<decltype(fptr)>(result);
            }
            g_dlsym_mutex.unlock();
            return this->fptr(args ...);
        }
    };

#define DLSYM_FUN(name, ...) IP2UNIX_REALCALL_EXTERN \
    struct name##_fun_t : public DlsymFun<name##_fun_t, __VA_ARGS__> { \
        static constexpr const char *fname = #name; \
    } name

    DLSYM_FUN(accept, int, int, struct sockaddr*, socklen_t*);
    DLSYM_FUN(accept4, int, int, struct sockaddr*, socklen_t*, int);
    DLSYM_FUN(bind, int, int, const struct sockaddr*, socklen_t);
    DLSYM_FUN(close, int, int);
    DLSYM_FUN(connect, int, int, const struct sockaddr*, socklen_t);
    DLSYM_FUN(getpeername, int, int, struct sockaddr*, socklen_t*);
    DLSYM_FUN(getsockname, int, int, struct sockaddr*, socklen_t*);
#ifdef SOCKET_ACTIVATION
    DLSYM_FUN(listen, int, int, int);
#endif
    DLSYM_FUN(setsockopt, int, int, int, int, const void*, socklen_t);
    DLSYM_FUN(socket, int, int, int, int);
}

#endif