#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdint>
#include <cmath>
#include <setjmp.h>
#include "jit.h"

static FILE* getStdIn()  { return stdin;  }
static FILE* getStdOut() { return stdout; }
static FILE* getStdErr() { return stderr; }

void* ks_strdup(const char* p)
{
    return strdup(p);
}

void* ks_malloc(uint32_t size)
{
    return malloc(size);
}

void* ks_calloc(uint32_t count, uint32_t size)
{
    return calloc(count, size);
}

void *ks_realloc(void* p, int32_t size)
{
    return realloc(p, size);
}

void ks_free(void* p)
{
    free(p);
}

double ks_sin(double d1) { return sin(d1); }
double ks_cos(double d1) { return cos(d1); }
double ks_tan(double d1) { return tan(d1); }
double ks_fabs(double d1) { return fabs(d1); }
double ks_pow(double d1, double d2) { return pow(d1, d2); }
double ks_fmod(double d1, double d2) { return fmod(d1, d2); }
double ks_ceil(double d1) { return ceil(d1); }
double ks_floor(double d1) { return floor(d1); }
double ks_sqrt(double d1) { return sqrt(d1); }
double ks_exp(double d1) { return exp(d1); }
double ks_log10(double d1) { return log10(d1); }
double ks_log(double d1) { return log(d1); }
double ks_asin(double d1) { return asin(d1); }
double ks_acos(double d1) { return acos(d1); }
double ks_atan(double d1) { return atan(d1); }
double ks_atan2(double d1, double d2) { return atan2(d1, d2); }
double ks_sinh(double d1) { return sinh(d1); }
double ks_cosh(double d1) { return cosh(d1); }
double ks_tanh(double d1) { return tanh(d1); }

void setupPredefinedFunctions(ClangJitCompiler& compiler)
{
    // Memory management.
    ClangJitCompiler::addSymbol("ks_strdup", (void*)ks_strdup);
    ClangJitCompiler::addSymbol("ks_malloc", (void*)ks_malloc);
    ClangJitCompiler::addSymbol("ks_calloc", (void*)ks_calloc);
    ClangJitCompiler::addSymbol("ks_realloc", (void*)ks_realloc);
    ClangJitCompiler::addSymbol("ks_free", (void*)ks_free);

    // Math functions.
    ClangJitCompiler::addSymbol("sin", (void*)ks_sin);
    ClangJitCompiler::addSymbol("cos", (void*)ks_cos);
    ClangJitCompiler::addSymbol("tan", (void*)ks_tan);
    ClangJitCompiler::addSymbol("fabs", (void*)ks_fabs);
    ClangJitCompiler::addSymbol("pow", (void*)ks_pow);
    ClangJitCompiler::addSymbol("fmod", (void*)ks_fmod);
    ClangJitCompiler::addSymbol("ceil", (void*)ks_ceil);
    ClangJitCompiler::addSymbol("floor", (void*)ks_floor);
    ClangJitCompiler::addSymbol("sqrt", (void*)ks_sqrt);
    ClangJitCompiler::addSymbol("exp", (void*)ks_exp);
    ClangJitCompiler::addSymbol("log10", (void*)ks_log10);
    ClangJitCompiler::addSymbol("log", (void*)ks_log);
    ClangJitCompiler::addSymbol("asin", (void*)ks_asin);
    ClangJitCompiler::addSymbol("acos", (void*)ks_acos);
    ClangJitCompiler::addSymbol("atan", (void*)ks_atan);
    ClangJitCompiler::addSymbol("atan2", (void*)ks_atan2);
    ClangJitCompiler::addSymbol("sinh", (void*)ks_sinh);
    ClangJitCompiler::addSymbol("cosh", (void*)ks_cosh);
    ClangJitCompiler::addSymbol("tanh", (void*)ks_tanh);

    // Printf style formatters
    ClangJitCompiler::addSymbol("printf", (void*)printf);
    ClangJitCompiler::addSymbol("putchar", (void*)putchar);

    // JIT can't find following string functions.
    ClangJitCompiler::addSymbol("sscanf", (void*)sscanf);
    ClangJitCompiler::addSymbol("strtol", (void*)strtol);
    ClangJitCompiler::addSymbol("strtoll", (void*)strtoll);

    // Helpers for standard I/O.
    ClangJitCompiler::addSymbol("getStdIn", (void*)getStdIn);
    ClangJitCompiler::addSymbol("getStdOut", (void*)getStdOut);
    ClangJitCompiler::addSymbol("getStdErr", (void*)getStdErr);

    // File I/O interfaces
    ClangJitCompiler::addSymbol("fopen", (void*)fopen);
    ClangJitCompiler::addSymbol("fgets", (void*)fgets);
    ClangJitCompiler::addSymbol("fclose", (void*)fclose);
    
    //ClangJitCompiler::addSymbol("fopen", (void*)fopen);
    //ClangJitCompiler::addSymbol("fgets", (void*)fgets);
    //ClangJitCompiler::addSymbol("fclose", (void*)fclose);
}
