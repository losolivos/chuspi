#ifndef TRINITY_SHARED_PROBE_POINT_HPP
#define TRINITY_SHARED_PROBE_POINT_HPP

#ifdef HAVE_SYS_SDT_H
#  include <sys/sdt.h>
#  define TC_PROBE(provider, name) \
        DTRACE_PROBE(provider, name)
#  define TC_PROBE1(provider, name, arg1) \
        DTRACE_PROBE1(provider, name, arg1)
#  define TC_PROBE2(provider, name, arg1, arg2) \
        DTRACE_PROBE2(provider, name, arg1, arg2)
#  define TC_PROBE3(provider, name, arg1, arg2, arg3) \
        DTRACE_PROBE3(provider, name, arg1, arg2, arg3)
#  define TC_PROBE4(provider, name, arg1, arg2, arg3, arg4) \
        DTRACE_PROBE4(provider, name, arg1, arg2, arg3, arg4)
#  define TC_PROBE5(provider, name, arg1, arg2, arg3, arg4, arg5) \
        DTRACE_PROBE5(provider, name, arg1, arg2, arg3, arg4, arg5)
#  define TC_PROBE6(provider, name, arg1, arg2, arg3, arg4, arg5, arg6) \
        DTRACE_PROBE6(provider, name, arg1, arg2, arg3, arg4, arg5, arg6)
#  define TC_PROBE7(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
        DTRACE_PROBE7(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#  define TC_PROBE8(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
        DTRACE_PROBE8(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#  define TC_PROBE9(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
        DTRACE_PROBE9(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
#  define TC_PROBE10(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
        DTRACE_PROBE10(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
#  define TC_PROBE11(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
        DTRACE_PROBE11(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
#  define TC_PROBE12(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
        DTRACE_PROBE12(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)
#else
#  define TC_PROBE(provider, name) \
        do { } while(0)
#  define TC_PROBE1(provider, name, arg1) \
        do { } while(0)
#  define TC_PROBE2(provider, name, arg1, arg2) \
        do { } while(0)
#  define TC_PROBE3(provider, name, arg1, arg2, arg3) \
        do { } while(0)
#  define TC_PROBE4(provider, name, arg1, arg2, arg3, arg4) \
        do { } while(0)
#  define TC_PROBE5(provider, name, arg1, arg2, arg3, arg4, arg5) \
        do { } while(0)
#  define TC_PROBE6(provider, name, arg1, arg2, arg3, arg4, arg5, arg6) \
        do { } while(0)
#  define TC_PROBE7(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
        do { } while(0)
#  define TC_PROBE8(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
        do { } while(0)
#  define TC_PROBE9(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
        do { } while(0)
#  define TC_PROBE10(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
        do { } while(0)
#  define TC_PROBE11(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
        do { } while(0)
#  define TC_PROBE12(provider, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
        do { } while(0)
#endif

#endif // TRINITY_SHARED_PROBE_POINT_HPP
