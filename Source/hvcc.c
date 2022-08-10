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

#include "Interface.h"

t_widgetbehavior hvcc_widgetbehaviour;


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

static t_class *hvcc_class;

typedef struct _hvcc
{
    t_object x_obj;
    HeavyContextInterface* x_hv_object;
    int x_n_in;
    int x_n_out;
    t_inlet* x_inlets[8];
    t_outlet* x_outlets[8];
    t_glist* x_glist;
    t_clock* x_clock;
    char* x_state;
    t_float x_sig;
    
} t_hvcc;


static void printHook(void* c, const char* receiverName, const char* msgString, const HvMessage* m) {
    double timestampMs = 1000.0 * ((double) hv_msg_getTimestamp(m)) / hv_getSampleRate(c);
    post("[hvcc~ @ %0.3gms] %s: %s", timestampMs, receiverName, msgString);
    
}

void hvcc_save(t_gobj *z, t_binbuf *b)
{
    t_hvcc* x = (t_hvcc *)z;
    binbuf_addv(b, "ssiiss", gensym("#X"), gensym("obj"),
                (int)x->x_obj.te_xpix, (int)x->x_obj.te_ypix,
                gensym("hvcc~"), gensym(x->x_state));
    
    binbuf_addv(b, ";");
}

void hvcc_save_state(void* obj, const char* content)
{
    t_hvcc* x = (t_hvcc*)obj;
    x->x_state = strdup(content);
    load_state(x, content);
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

static void hvcc_click(t_hvcc* x, t_floatarg xpos, t_floatarg ypos,
                       t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    open_window(x);
}

static void hvcc_edit(t_hvcc* x)
{
    open_window(x);
}

static void hvcc_focus(t_hvcc* x, t_floatarg* f)
{
}

void hvcc_load(void* obj, t_create create)
{
    t_hvcc* x = (t_hvcc*)obj;
    // Create the patch instance
    x->x_hv_object = create(sys_getsr());
    
    int old_n_in = x->x_n_in;
    int old_n_out = x->x_n_out;
    
    // Fix number of inlets/outlets
    x->x_n_in = hv_getNumInputChannels(x->x_hv_object);
    x->x_n_out = hv_getNumOutputChannels(x->x_hv_object);
    
    gobj_vis(&x->x_obj.te_g, x->x_glist, 0);
    
    for(int i = (x->x_n_in - 1); i < (old_n_in - 1); i++) {
        if(i < 0) continue;
        canvas_deletelinesforio(x->x_glist, &x->x_obj,
                                x->x_inlets[i], 0);
        
        inlet_free(x->x_inlets[i]);
    }
    
    for(int i = x->x_n_out; i < old_n_out; i++) {
        canvas_deletelinesforio(x->x_glist, &x->x_obj,
                                0, x->x_outlets[i]);
        outlet_free(x->x_outlets[i]);
        
    }
    
    for(int i = (old_n_in - 1); i < (x->x_n_in - 1); i++) {
        if(i < 0) continue;
        x->x_inlets[i] = signalinlet_new(&x->x_obj, 0.0);
    }
    
    for(int i = old_n_out; i < x->x_n_out; i++) {
        x->x_outlets[i] = outlet_new(&x->x_obj, &s_signal);
    }
    
    // Update DSP
    canvas_update_dsp();
    
    // Repaint inlets/outlets
    gobj_vis(&x->x_obj.te_g, x->x_glist, 1);
    
    // Set printhook
    hv_setPrintHook(x->x_hv_object, printHook);
}

static t_int* hvcc_vis(t_gobj *z, t_glist *glist, int vis) {
    
    t_text *x = (t_text *)z;
    if (vis)
    {
        t_rtext *y = glist_findrtext(glist, x);
        if (y && gobj_shouldvis(&x->te_g, glist))
        {
            text_drawborder(x, glist, rtext_gettag(y),
                            rtext_width(y), rtext_height(y), 1);
            
            // Don't show the long base64 state!
            binbuf_free(x->te_binbuf);
            t_binbuf* b = binbuf_new();
            binbuf_text(b, "hvcc~", 5);
            x->te_binbuf = b;
            
            rtext_retext(y);
            rtext_draw(y);
        }
    }
    else
    {
        t_rtext *y = glist_findrtext(glist, x);
        if (y && gobj_shouldvis(&x->te_g, glist))
        {
            text_eraseborder(x, glist, rtext_gettag(y));
            rtext_erase(y);
        }
    }
    
    
}

static t_int* hvcc_tick(t_hvcc* x) {
    
    dequeue_messages();
    clock_delay(x->x_clock, 20);
}

static t_int* hvcc_perform(t_int* w) {
    
    assert(sizeof(t_sample) == sizeof(float));
    
    t_hvcc* x = (t_hvcc *) w[1];
    t_sample** input_pointers;
    t_sample** output_pointers;
    
    int offset = 2;
    if(x->x_n_in == 0) offset = 3;
    
    if(!x->x_hv_object) return (w + x->x_n_in + x->x_n_out + offset + 1);
    
    input_pointers = (t_sample**)w + offset;
    output_pointers = (t_sample**)w + x->x_n_in + offset;
    
    int n = (int) (w[x->x_n_in + x->x_n_out + offset]);
    
    hv_process(x->x_hv_object, input_pointers, output_pointers, n);
    
    return (w + x->x_n_in + x->x_n_out + offset + 1);
    
}

static void hvcc_dsp(t_hvcc* x, t_signal **sp);

static void* hvcc_new(t_symbol *s, int argc, t_atom *argv)
{
    t_hvcc* x = (t_hvcc *) pd_new(hvcc_class);
    
    init_interface();
    
    x->x_n_in = 0;
    x->x_n_out = 0;
    x->x_hv_object = NULL;
    
    x->x_glist = canvas_getcurrent();
    x->x_clock = clock_new(x, (t_method)hvcc_tick);
    
    if(argc == 1) {
        x->x_state = (char*)atom_getsymbol(argv)->s_name;
        load_state(x, x->x_state);
        
        
    }
    else {
        x->x_state = "";
    }
    
    clock_delay(x->x_clock, 20);
    
    return (x);
}


static void hvcc_free(t_hvcc* x)
{
    clock_free(x->x_clock);
    close_window();
}

void hvcc_tilde_setup(void)
{
    hvcc_class = class_new(gensym("hvcc~"), (t_newmethod)hvcc_new,
                           (t_method)hvcc_free, sizeof(t_hvcc), 0, A_GIMME, 0);
    
    CLASS_MAINSIGNALIN(hvcc_class, t_hvcc, x_sig);
    
    class_addmethod(hvcc_class, hvcc_edit, gensym("edit"), 0, 0);
    class_addmethod(hvcc_class, hvcc_focus, gensym("_focus"), A_FLOAT, 0);
    class_addmethod(hvcc_class, hvcc_edit, gensym("menu-open"), A_NULL);
    
    hvcc_widgetbehaviour = text_widgetbehavior;
    hvcc_widgetbehaviour.w_visfn = hvcc_vis;
    
    class_setwidget(hvcc_class, &hvcc_widgetbehaviour);
    
    class_setsavefn(hvcc_class, hvcc_save);
    
    class_addmethod(hvcc_class, (t_method)hvcc_click,
                    gensym("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    
    class_addmethod(hvcc_class, (t_method)hvcc_dsp,
                    gensym("dsp"), 0);
}


static void hvcc_dsp(t_hvcc* x, t_signal** sp) {
    
    if(!x->x_hv_object) return;
    
    t_int* argv;
    int actual_in = hv_max_i(1, x->x_n_in);
    
    int argc = actual_in + x->x_n_out + 2;
    argv = calloc(sizeof(t_int), argc);
    
    argv[0] = (t_int)x;
    
    for(int ch = 0; ch < actual_in + x->x_n_out; ch++) {
        argv[ch + 1] = sp[ch]->s_vec;
    }
    
    argv[argc - 1] = sp[0]->s_n;
    
    dsp_addv(hvcc_perform, argc, argv);
}
