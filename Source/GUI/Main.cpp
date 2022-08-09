#if ENABLE_LIBCLANG
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <llvm/Support/TargetSelect.h>
#endif

#include "m_pd.h"
#include "g_canvas.h"

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "BinaryData.h"

using namespace juce;

#include <unordered_map>
#include "Compiler.h"
#include "Object.h"
#include "Connection.h"
#include "Canvas.h"

//==============================================================================
class HvccEditor  : public JUCEApplication, public ChildProcessWorker
{
public:
    
    const static inline Font DejaVu = Font(Typeface::createSystemTypefaceFor(BinaryData::DejaVuSansMono_ttf, BinaryData::DejaVuSansMono_ttfSize)).withPointHeight(10.f);
    
    //==============================================================================
    HvccEditor() {
        LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(DejaVu.getTypefacePtr());
    }

    const String getApplicationName() override       { return "hvcc~"; }
    const String getApplicationVersion() override    { return "0.0.1"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        // For debugging
        if(commandLine.isEmpty()) {
            createPatch("test", true);
        }
        else {
            initialiseFromCommandLine(commandLine, "test_id");
        }
        
        
        
    }


    void shutdown() override
    {
        patchWindows.clear();
    }
    
    void loadPatch(String ID, String patch)
    {
        if(!windowsById.count(ID)) {
            auto* patchWindow = patchWindows.add(new PatchWindow (getApplicationName(), ID));
            windowsById[ID] = patchWindow;
        }
        
        windowsById[ID]->cnv->getCanvas()->loadState(patch);
    }
    
    void createPatch(String ID, bool makeVisible)
    {
        if(!windowsById.count(ID)) {
            auto* patchWindow = patchWindows.add(new PatchWindow (getApplicationName(), ID));
            windowsById[ID] = patchWindow;
            patchWindow->setVisible(makeVisible);
            patchWindow->setWantsKeyboardFocus(true);
            if(makeVisible) patchWindow->toFront(true);
        }
        else if(makeVisible) {
            windowsById[ID]->setVisible(true);
            windowsById[ID]->toFront(true);
        }
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // Do nothing!
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
    }
    
    
    void handleMessageFromCoordinator (const MemoryBlock &mb) override
    {
        auto stream = MemoryInputStream(mb, false);
        
        //if(!patchWindows) return;
        
        auto ID = stream.readString();
        auto selector = stream.readString();
        
        if(selector == "Open") {
            MessageManager::callAsync([this, ID]() mutable {
                createPatch(ID, true);
            });
            
        }
        
        if(selector == "LoadState") {
            auto content = stream.readString();
            MessageManager::callAsync([this, ID, content]() mutable {
                createPatch(ID, false);
                
                windowsById[ID]->cnv->getCanvas()->loadState(content);
            });
        }
        if(selector == "Close") {
            MessageManager::callAsync([this, ID]() mutable {
                windowsById[ID]->setVisible(false);
            });
        }
    }
    void handleConnectionLost() override {
        appWillTerminateByForce();
    }

    class PatchWindow : public DocumentWindow
    {
    public:
        
        CanvasHolder* cnv;
        PatchWindow (String name, String ID)
            : DocumentWindow (name,
                              Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            cnv = new CanvasHolder(ID);
            setContentOwned (cnv, true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            setSize(400, 300);
            
           #endif

            setVisible (true);
            
#if JUCE_MAC
            Process::setDockIconVisible(false);
#endif
            
            getLookAndFeel().setColour(TextButton::buttonColourId, Colours::white);
            getLookAndFeel().setColour(TextButton::buttonOnColourId, Colours::grey);
            getLookAndFeel().setColour(TextButton::textColourOnId, Colours::black);
            getLookAndFeel().setColour(TextButton::textColourOffId, Colours::black);
            
            getLookAndFeel().setColour(ResizableWindow::backgroundColourId, Colours::white);
            
            getLookAndFeel().setColour(ScrollBar::backgroundColourId, Colours::transparentWhite);
            getLookAndFeel().setColour(ScrollBar::trackColourId, Colours::transparentWhite);
            getLookAndFeel().setColour(ScrollBar::thumbColourId, Colours::darkgrey);
            
            getLookAndFeel().setColour(Label::textColourId, Colours::black);
            getLookAndFeel().setColour(Label::outlineColourId, Colours::black);
            getLookAndFeel().setColour(Label::textWhenEditingColourId, Colours::black);
            getLookAndFeel().setColour(Label::outlineWhenEditingColourId, Colours::black);
            
            getLookAndFeel().setColour(Label::backgroundColourId, Colours::transparentWhite);
            getLookAndFeel().setColour(Label::backgroundWhenEditingColourId, Colours::transparentWhite);
            
            getLookAndFeel().setColour(TextEditor::backgroundColourId, Colours::white);
            getLookAndFeel().setColour(TextEditor::outlineColourId, Colours::black);
            getLookAndFeel().setColour(TextEditor::textColourId, Colours::black);
        }


        void closeButtonPressed() override
        {
            setVisible(false);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PatchWindow)
    };

    std::map<String, PatchWindow*> windowsById;
    OwnedArray<PatchWindow> patchWindows;
};

START_JUCE_APPLICATION(HvccEditor)
