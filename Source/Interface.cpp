#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <m_pd.h>

#include "Interface.h"
#include "whereami.h"
#include "concurrentqueue.h"

using namespace juce;

struct Interface : public ChildProcessCoordinator
{
    String ID = "test_id";
    String workerLocation = "";
    
    std::map<void*, String> externalsMap;
    std::map<String, void*> invExternalsMap;
    
    OwnedArray<DynamicLibrary> loadedLibraries;
    
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
            
            if(selector == "Load") {
                auto path = stream.readString();
                auto name = File(path).getFileNameWithoutExtension();

                auto* lib = loadedLibraries.add(new DynamicLibrary());
                lib->open(path);
                
                auto* func = lib->getFunction("hv_" + name + "_new");
                hvcc_load(invExternalsMap[ID], (t_create)func);
                
                auto tmpdir = File(path).getParentDirectory().getParentDirectory();
                if(tmpdir.getFileName() == "tmp") {
                    tmpdir.deleteRecursively();
                }
                else
                {
                    File(path).deleteFile();
                }
                
                
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
    Interface::getInstance()->loadState(obj, content);
}

void dequeue_messages() {
    Interface::getInstance()->dequeueMessages();
}
