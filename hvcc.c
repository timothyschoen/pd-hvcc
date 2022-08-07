#include "m_pd.h"
#include "g_canvas.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_LIBDL) || defined(__FreeBSD__)
#include <dlfcn.h>
#elif _WIN32
#include <windows.h>
#else
#error dlopen not supported!
#endif
#include "hvcc/HvHeavy.h"

extern void create_gui(const char* content, void* cnv);
extern void run_loop_until(int ms);
extern void stop_gui();

#if __linux__
const char* cxx_flags = "";
const char* file_ext = "so";
const char* out_dir = "/tmp/hvcc";
#elif __APPLE__
const char* cxx_flags = "-std=c++17 -DPD -DUNIX -DMACOSX -I /sw/include   -Ishared -DHAVE_STRUCT_TIMESPEC -O3 -ffast-math -funroll-loops -fomit-frame-pointer -arch arm64 -mmacosx-version-min=10.6";
const char* linker_flags = "-undefined suppress -flat_namespace -bundle  -arch arm64 -mmacosx-version-min=10.6";
const char* file_ext = "dylib";
const char* out_dir = "/private/tmp/hvcc";
#elif _WIN32
const char* cxx_flags = "";
const char* file_ext = "dll";
const char* out_dir = "c:\temp\hvcc";
#endif

#ifndef HVCC_PATH
const char* hvcc_path = "/opt/homebrew/bin/hvcc";
#else
const char* hvcc_path = HVCC_PATH;
#endif



typedef HeavyContextInterface* (*t_create)(double);

static t_class *hvcc_class;

typedef struct _hvcc
{
    t_object x_obj;
    int n_in;
    int n_out;
    HeavyContextInterface* hv_object;
    t_float x_sig;
    t_glist* x_glist;
    t_inlet* x_inlets[8];
    t_outlet* x_outlets[8];
    t_clock* x_clock;
    
} t_hvcc;

static void printHook(void *c, const char *receiverName, const char *msgString, const HvMessage *m) {
  double timestampMs = 1000.0 * ((double) hv_msg_getTimestamp(m)) / hv_getSampleRate(c);
  post("[heavy~ @ %0.3gms] %s: %s", timestampMs, receiverName, msgString);
    
}

static void get_filename(const char* path, char* filename) {
    strcpy(filename, path);
    
    int init_size = strlen(filename);
    char delim[] = "/";

    char* ptr = strtok(filename, delim);
    char* last;
    
    while(ptr != NULL)
    {
        last = ptr;
        ptr = strtok(NULL, delim);
    }
    
    
    strcpy(filename, last);
    
    char *end = filename + strlen(filename);

    while (end > filename && *end != '.' && *end != '\\' && *end != '/') {
        --end;
    }
    if ((end > filename && *end == '.') &&
        (*(end - 1) != '\\' && *(end - 1) != '/')) {
        *end = '\0';
    }
    
    
}

static void hvcc_edit(t_hvcc *x)
{
    create_gui("", x);
    clock_delay(x->x_clock, 50);
    
    //sys_vgui("")
}

static void hvcc_load(t_hvcc *x, t_symbol* s)
{
    char external_name[MAXPDSTRING];
    get_filename(s->s_name, external_name);
    

    char lib_dir[MAXPDSTRING];
    snprintf(lib_dir, sizeof(lib_dir), "%s%s", out_dir, "/lib");
    
    char generate[MAXPDSTRING];
    snprintf(generate, sizeof(generate), "%s -o %s -n %s %s", hvcc_path, out_dir, external_name, s->s_name);
    
    // Generate C++ code
    system(generate);
    
    // Clear out previous build just to be sure
    char clear_command[MAXPDSTRING];
    snprintf(clear_command, sizeof(clear_command), "rm -rf %s && mkdir -p %s", lib_dir, lib_dir);
    system(clear_command);
    
    // Construct path for .o output from compiler
    char out_path[MAXPDSTRING];
    snprintf(out_path, sizeof(out_path), "%s/lib/%s.o", out_dir, external_name);
    
    // Construct path for cpp input file
    char in_path[MAXPDSTRING];
    snprintf(in_path, sizeof(in_path), "%s/c/Heavy_%s.cpp", out_dir, external_name);

    // Construct compile command
    char compile_cxx[MAXPDSTRING];
    snprintf(compile_cxx, sizeof(compile_cxx), "c++ %s -o %s -c %s", cxx_flags, out_path, in_path);
    
    // Construct final output path for library
    char lib_path[MAXPDSTRING];
    snprintf(lib_path, sizeof(lib_path), "%s/lib/%s.%s", out_dir, external_name, file_ext);

    char link[MAXPDSTRING];
    snprintf(link, sizeof(link), "cc %s -o %s %s", linker_flags, lib_path, out_path);

    
    // Compile and link the code
    system(compile_cxx);
    system(link);
    
    // Load the dynamic library we've created
    void* dlobj = dlopen(lib_path, RTLD_NOW | RTLD_GLOBAL);
    
    // Clean up
    system(clear_command);
    
    // Check for errors
    char* error = dlerror();
    if(!dlobj && error)  {
        pd_error(x, "%s", error);
        return;
    }
    
    // Generate name of instance creation function
    char create_func[MAXPDSTRING];
    snprintf(create_func, sizeof(create_func), "hv_%s_new", external_name);
    
    // Get create function from the loaded dynamic library
    t_create create = (t_create)dlsym(dlobj, create_func);
    
    // Create the patch instance
    x->hv_object = create(sys_getsr());
    

    int old_n_in = x->n_in;
    int old_n_out = x->n_out;
    
    // Fix number of inlets/outlets
    x->n_in = hv_getNumInputChannels(x->hv_object);
    x->n_out = hv_getNumOutputChannels(x->hv_object);

    for(int i = x->n_in + 1; i < old_n_in; i++) {
        inlet_free(x->x_inlets[i]);
    }
    
    for(int i = x->n_out; i < x->n_out; i++) {
        outlet_free(x->x_outlets[i]);
    }
    
    for(int i = old_n_in; i < x->n_in; i++) {
        x->x_inlets[i] = signalinlet_new(&x->x_obj, 0.0);
    }
    
    for(int i = old_n_out; i < x->n_out; i++) {
        x->x_outlets[i] = outlet_new(&x->x_obj, &s_signal);
    }

    // Update DSP
    canvas_update_dsp();
    
    // Repaint inlets/outlets
    gobj_vis(&x->x_obj.te_g, x->x_glist, 0);
    gobj_vis(&x->x_obj.te_g, x->x_glist, 1);
    
    // Set printhook
    hv_setPrintHook(x->hv_object, printHook);
    
    printf("Done!");
}


static t_int *hvcc_perform(t_int *w) {
    
    
  t_hvcc *x = (t_hvcc *) w[1];
    
  if(!x->hv_object) return (w+7);
    
  float *inputChannels[2] = {
    (t_sample *) w[2],
    (t_sample *) w[3],
  };
  float *outputChannels[2] = {
    (t_sample *) w[4],
    (t_sample *) w[5],
  };
  int n = (int) (w[6]);

  hv_process(x->hv_object, inputChannels, outputChannels, n);

  return (w+7);
    
}

static t_int *hvcc_dsp(t_hvcc *x, t_signal **sp) {
    
    if(!x->hv_object) return;
    
    dsp_add(hvcc_perform,
        6, x,
        sp[0]->s_vec,
        sp[1]->s_vec,
        sp[2]->s_vec,
        sp[3]->s_vec,
        sp[0]->s_n);
}
static void *hvcc_tick(t_hvcc *x)
{
    run_loop_until(1);
    clock_delay(x->x_clock, 1);
}

static void *hvcc_new(t_floatarg nin, t_floatarg nout)
{
    t_hvcc *x = (t_hvcc *) pd_new(hvcc_class);
    
    x->n_in = 1;
    x->n_out = 0;
    x->hv_object = NULL;
    x->x_glist = canvas_getcurrent();
     
    x->x_clock = clock_new(x, (t_method)hvcc_tick);
    
    return (x);
}

static void hvcc_free(t_hvcc *x)
{
    stop_gui();
}



void hvcc_tilde_setup(void)
{
    hvcc_class = class_new(gensym("hvcc~"), (t_newmethod)hvcc_new,
                           (t_method)hvcc_free,sizeof(t_hvcc), 0, 0);
    
    CLASS_MAINSIGNALIN(hvcc_class, t_hvcc, x_sig);

    class_addmethod(hvcc_class, hvcc_load, gensym("open"), A_SYMBOL, 0);
    class_addmethod(hvcc_class, hvcc_edit, gensym("edit"), 0, 0);
    
    class_addmethod(hvcc_class, (t_method)hvcc_dsp,
        gensym("dsp"), 0);
}

