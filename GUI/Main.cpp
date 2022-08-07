
#include "../pure-data/src/m_pd.h"
#include "../pure-data/src/g_canvas.h"

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;


#include <unordered_map>
#include "Object.h"
#include "Connection.h"
#include "Canvas.h"

//==============================================================================
class HvccEditor  : public JUCEApplication
{
public:
    //==============================================================================
    HvccEditor() {
        
    }

    const String getApplicationName() override       { return "hvccgen~"; }
    const String getApplicationVersion() override    { return "0.0.1"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
    }
    
    void init(void* object) {
        mainWindow.reset (new MainWindow (getApplicationName(), object));
    }
    

    void shutdown() override
    {
        mainWindow = nullptr; // (deletes our window)
    }
    
    void makeVisible()
    {
        mainWindow->setVisible(true);
        
        // Async call prevents pd from getting
        // keyboard focus after the click that triggered this action
        MessageManager::callAsync([this](){
            mainWindow->setWantsKeyboardFocus(true);
            //mainWindow->setAlwaysOnTop(true);
            mainWindow->toFront(true);
        });
        
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // Do nothing!
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow : public DocumentWindow
    {
    public:
        MainWindow (String name, void* object)
            : DocumentWindow (name,
                              Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new Canvas(object), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            setSize(400, 300);
           #endif

            setVisible (true);
            
            Process::setDockIconVisible(true);
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            setVisible(false);
            Process::setDockIconVisible(false);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

//START_JUCE_APPLICATION(HvccEditor)
JUCE_CREATE_APPLICATION_DEFINE(HvccEditor)

extern "C"
{

static JUCEApplicationBase* app;

void create_gui(const char* content, void* object) {
    static JUCEApplicationBase* app;
    
    if(!JUCEApplicationBase::createInstance) {
        initialiseJuce_GUI();
        
        
        JUCEApplicationBase::createInstance = &::juce_CreateApplication;
        app = juce_CreateApplication();
        dynamic_cast<HvccEditor*>(app)->init(object);
    }
    else {
        dynamic_cast<HvccEditor*>(app)->makeVisible();
    }
}

void run_loop_until(int ms) {
   // MessageManager::getInstance()->runDispatchLoop();
    MessageManager::getInstance()->runDispatchLoopUntil(ms);
}

void stop_gui() {
    shutdownJuce_GUI();
}

}
