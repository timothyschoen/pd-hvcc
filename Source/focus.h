static t_class *mouse_proxy_class, *active_gui_class, *active_class;

typedef struct _mouse_proxy{
    t_object    p_obj;
    t_symbol   *p_sym;
    t_clock    *p_clock;
    struct      _hvcc *p_parent;
}t_mouse_proxy;

typedef struct _active_gui{
    t_pd       g_pd;
    t_symbol  *g_psgui;
    t_symbol  *g_psfocus;
}t_active_gui;


t_active_gui *gui_sink = 0;

static void active_gui__focus(t_active_gui *snk, t_symbol *s, t_floatarg f){
    if(!snk->g_psfocus) // bug
        return;
    if(snk->g_psfocus->s_thing){
        t_atom at[2];
        SETSYMBOL(&at[0], s);
        SETFLOAT(&at[1], f);
        pd_typedmess(snk->g_psfocus->s_thing, gensym("_focus"), 2, at);
    }
}

static void gui_dobindfocus(t_active_gui *snk){ // once for all objects
    sys_vgui("bind Canvas <<active_focusin>> \
             {if {[active_gui_ispatcher %%W]} \
             {pdsend {%s _focus %%W 1}}}\n", snk->g_psgui->s_name);
    sys_vgui("bind Canvas <<active_focusout>> \
             {if {[active_gui_ispatcher %%W]} \
             {pdsend {%s _focus %%W 0}}}\n", snk->g_psgui->s_name);
}

static void active_gui__refocus(t_active_gui *snk){
    if(!snk->g_psfocus) // bug
        return;
    if(snk->g_psfocus->s_thing) // if new master bound in gray period, restore gui bindings
        gui_dobindfocus(snk);
}

static int active_gui_setup(void){
    if(gensym("#active_gui")->s_thing){
        if(strcmp(class_getname(*gensym("#active_gui")->s_thing), gensym("_active_gui")->s_name))
            return(0); // bug - avoid something (e.g. receive) bind to #active_gui
        else{
            active_gui_class = *gensym("#active_gui")->s_thing;
            return(1);
        }
    }
    active_gui_class = class_new(gensym("_active_gui"), 0, 0,
        sizeof(t_active_gui), CLASS_PD | CLASS_NOINLET, 0);    class_addmethod(active_gui_class, (t_method)active_gui__refocus,
        gensym("_refocus"), 0);
    class_addmethod(active_gui_class, (t_method)active_gui__focus,
        gensym("_focus"), A_SYMBOL, A_FLOAT, 0);
    
    /* Protect against pdCmd being called (via "Canvas <Destroy>" binding)
     during Tcl_Finalize().  FIXME this should be a standard exit handler. */
    sys_gui("proc active_gui_exithook {cmd op} {proc ::pdsend {} {}}\n");
    sys_gui("if {[info tclversion] >= 8.4} {\n\
            trace add execution exit enter active_gui_exithook}\n");
    
    sys_gui("proc active_gui_ispatcher {cv} {\n");
    sys_gui(" if {[string range $cv 0 1] == \".x\"");
    sys_gui("  && [string range $cv end-1 end] == \".c\"} {\n");
    sys_gui("  return 1} else {return 0}\n");
    sys_gui("}\n");
    
    sys_gui("proc active_gui_getscreen {} {\n");
    sys_gui(" set px [winfo pointerx .]\n");
    sys_gui(" set py [winfo pointery .]\n");
    sys_gui(" pdsend \"#active_mouse _getscreen $px $py\"\n");
    sys_gui("}\n");
    
    sys_gui("proc active_gui_getscreenfocused {} {\n");
    sys_gui(" set px [winfo pointerx .]\n");
    sys_gui(" set py [winfo pointery . ]\n");
    sys_gui(" set wx [winfo x $::focused_active]\n");
    sys_gui(" set wy [winfo y $::focused_active]\n");
    sys_gui(" pdsend \"#active_mouse _getscreenfocused ");
    sys_gui("$px $py $wx $wy\"\n");
    sys_gui("}\n");
    
    sys_gui("proc active_gui_refocus {} {\n");
    sys_gui(" bind Canvas <<active_focusin>> {}\n");
    sys_gui(" bind Canvas <<active_focusout>> {}\n");
    sys_gui(" pdsend {#active_gui _refocus}\n");
    sys_gui("}\n");
    
    return(1);
}

void active_gui_getscreenfocused(void){
    if(active_gui_class && gui_sink)
        sys_gui("active_gui_getscreenfocused\n");
}

void active_gui_getscreen(void){
    if(active_gui_class && gui_sink)
        sys_gui("active_gui_getscreen\n");
}

static void mouse_proxy_any(t_mouse_proxy *p, t_symbol*s, int ac, t_atom *av){
    ac = 0;
}

static void mouse_proxy_free(t_mouse_proxy *p){
    pd_unbind(&p->p_obj.ob_pd, p->p_sym);
    clock_free(p->p_clock);
    pd_free(&p->p_obj.ob_pd);
}

static t_mouse_proxy * mouse_proxy_new(t_hvcc *x, t_symbol*s){
    t_mouse_proxy *p = (t_mouse_proxy*)pd_new(mouse_proxy_class);
    p->p_parent = x;
    pd_bind(&p->p_obj.ob_pd, p->p_sym = s);
    p->p_clock = clock_new(p, (t_method)mouse_proxy_free);
    return(p);
}
