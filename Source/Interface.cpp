#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_data_structures/juce_data_structures.h>

#include <m_pd.h>
#include "concurrentqueue.h"

using namespace juce;

#include "Interface.h"
#include "GUI/Compiler.h"

struct Interface : public ChildProcessCoordinator
{
    String ID = "test_id";
    String workerLocation = "";
    
    std::map<void*, String> externalsMap;
    std::map<String, void*> invExternalsMap;
    
    std::map<String, std::unique_ptr<DynamicLibrary>> loadedLibraries;
    
    moodycamel::ConcurrentQueue<MemoryBlock> queue;
    
    Interface() {
        int length, dirname_length;
        length = wai_getModulePath(NULL, 0, &dirname_length);
        if (length > 0)
        {
          char* path = (char*)malloc(length + 1);
          wai_getModulePath(path, length, &dirname_length);
          path[dirname_length] = '\0';

          launchWorkerProcess (File(String(path) + "/hvcc_gui"), ID, 10000);
          
          free(path);
        }
        
        // Initialise it, but don't use it
        MessageManager::getInstance();
    };
    
    void handleConnectionLost() override {
        
        // This will never get executed...
        std::cout << "Connection lost!" << std::endl;
        
    }
    
    void handleMessageFromWorker(const MemoryBlock &mb) override
    {
        queue.enqueue(mb);
    }
    
    
    
    void dequeueMessages() {
        
        MemoryBlock mb;
        while (queue.try_dequeue(mb)) {
            auto stream = MemoryInputStream(mb, false);
            auto ID = stream.readString();
            auto selector = stream.readString();
            
            if(selector == "Error") {
                auto message = stream.readString();
                post("[hvcc~]: %s", message.toRawUTF8());
            }
            
            if(selector == "Load") {
                auto path = stream.readString();
                loadLibrary(invExternalsMap[ID], path);
            }
            if(selector == "SaveState") {
                auto content = stream.readString();
                hvcc_save_state(invExternalsMap[ID], content.toRawUTF8());
            }
        }
    }
    
    void loadState(void* ext, const char* state) {
        if(!externalsMap.count(ext)) {
            String ID = Uuid().toString();
            externalsMap[ext] = ID;
            invExternalsMap[ID] = ext;
        }
        
        MemoryOutputStream ostream;
        ostream.writeString(externalsMap[ext]);
        ostream.writeString("LoadState");
        ostream.writeString(String(state));
        sendMessageToWorker(ostream.getMemoryBlock());
    }
    
    
    void openWindow(void* ext) {
        if(!externalsMap.count(ext)) {
            String ID = Uuid().toString();
            externalsMap[ext] = ID;
            invExternalsMap[ID] = ext;
        }
        
        MemoryOutputStream ostream;
        ostream.writeString(externalsMap[ext]);
        ostream.writeString("Open");
        sendMessageToWorker(ostream.getMemoryBlock());
    }
    
    void closeWindow() {
        MemoryOutputStream ostream;
        //ostream.writeString(externalsMap[ext]);
        ostream.writeString("Close");
        sendMessageToWorker(ostream.getMemoryBlock());
    }
    
    void compileAndLoad(void* external, const String& patch) {
        auto compiler = Compiler(true);
        
        MemoryOutputStream ostream;
        Base64::convertFromBase64(ostream, patch);
        auto patchContent = String((char*)ostream.getData(), ostream.getDataSize());
        
        auto libdir = compiler.generateLibrary(patchContent);
        loadLibrary(external, libdir);
    }
    
    void loadLibrary(void* external, const String& path) {
        auto name = File(path).getFileNameWithoutExtension();

        loadedLibraries[ID] = std::make_unique<DynamicLibrary>();
        auto* lib = loadedLibraries[ID].get();
        lib->open(path);
        
        
        auto* func = lib->getFunction("hv_" + name + "_new");
        if(func) {
            
            hvcc_load(external, (t_create)func);
            
            auto tmpdir = File(path).getParentDirectory().getParentDirectory();
            if(tmpdir.getFileName() == "tmp") {
                tmpdir.deleteRecursively();
            }
            else
            {
                File(path).deleteFile();
            }
        }
        else {
            loadedLibraries[ID].reset(nullptr);
        }
        
    }
    
    JUCE_DECLARE_SINGLETON(Interface, true);
};

JUCE_IMPLEMENT_SINGLETON(Interface);

void init_interface() {
    Interface::getInstance();
}
void open_window(void* obj) {
    Interface::getInstance()->openWindow(obj);
}

void close_window() {
    Interface::getInstance()->closeWindow();
}

void load_state(void* obj, const char* content)
{
    // Pass state to GUI
    Interface::getInstance()->loadState(obj, content);
    
    // Compile and load last state
    Interface::getInstance()->compileAndLoad(obj, content);
}


void dequeue_messages() {
    Interface::getInstance()->dequeueMessages();
}
