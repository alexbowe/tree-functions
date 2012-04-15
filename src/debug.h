#ifndef DEBUG_H
#define DEBUG_H

// Tracing
// NOTE: Needs to be called using double brackets:
// TRACE( ("%d", variable) );
#ifndef NDEBUG
    #include <cstdarg>
    #include <cstdio>
    inline void trace_impl(const char * format, ...)
    {
        const static size_t BUFFER_SIZE = 1024;
        char buffer[BUFFER_SIZE];
        
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, BUFFER_SIZE, format, args);
        va_end (args);
        
        fprintf(stderr, "%s", buffer);
    }
    #define TRACE(msg) trace_impl msg
#else
    #define TRACE(msg)
#endif


#ifndef NDEBUG
    #include <iostream>
    template <class Vector>
    inline void trace_v_impl(const Vector& v)
    { 
        int size = v.size();
        std::cerr << "[";
        for (int i = 0; i < size; i++)
        {
            std::cerr << v[i];
            if (i < size - 1)
                std::cerr << ", ";
        }
        std::cerr << "]" << std::endl;
    }
    #define TRACE_V(seq) trace_v_impl seq
#else
    #define TRACE_V(seq)
#endif

#endif /* end of include guard */
