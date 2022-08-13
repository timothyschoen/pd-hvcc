#pragma once
#include "../JIT/jit.h"
#include "../whereami.h"

namespace hvcc
{

extern "C" int error_handler(int level, const char *filename, int line, int column, const char *message)
{
    static int count = 0;
    static const char *levelString[] = {"Ignore", "Note", "Remark", "Warning", "Error", "Fatal"};
    printf("%s(%d): %s\n", levelString[level], ++count, message);
    printf("    File: %s\n", filename);
    printf("    Line: %d\n\n", line);
    return 1;
}

struct Compiler : public Thread
{
    
    inline static String cxxPath = "";
    inline static String pyPath = "";
    inline static File workingDir = File();
#if JUCE_LINUX
    const String compileFlags = "-std=c++17 -fPIC -Ishared -DHAVE_STRUCT_TIMESPEC -O3 -ffast-math -funroll-loops -fomit-frame-pointer";
    const String dllExtension = "so";
    const String linkerFlags = "-rdynamic -shared -fPIC -Wl,-rpath,\"\\$ORIGIN\",--enable-new-dtags -lc -lm ";
#elif JUCE_MAC
    const String compileFlags = "-std=c++17 -DPD -DUNIX -DMACOSX -I /sw/include -Ishared -DHAVE_STRUCT_TIMESPEC -O3 -ffast-math -funroll-loops -fomit-frame-pointer -arch arm64 -mmacosx-version-min=10.12";
    const String linkerFlags = "-undefined suppress -flat_namespace -bundle  -arch arm64 -mmacosx-version-min=10.12";
    const String dllExtension = "dylib";
#elif JUCE_WINDOWS
    const String compileFlags = "";
    const String dllExtension = "dll";
    const String linkerFlags = "-undefined suppress -flat_namespace -bundle";
#endif
    
    
    Compiler(bool insideExternal) : Thread("Compiler Thread") {
        
        if(!workingDir.isDirectory()) {
            
            auto* getPath = insideExternal ? wai_getModulePath : wai_getExecutablePath;
            
            // Get directory of executable
            int length, dirLength;
            
            length = getPath(NULL, 0, &dirLength);
            if (length == 0) return; // I sure hope this never happens...
            
            char* path = (char*)malloc(length + 1);
            getPath(path, length, &dirLength);
            path[dirLength] = '\0';
            
            workingDir = File(path);
        }
        
        auto pathsFile = workingDir.getChildFile("Paths.xml");
        if(pathsFile.existsAsFile()) {
            auto tree = ValueTree::fromXml(pathsFile.loadFileAsString());
            pyPath = tree.getProperty("python3").toString();
            cxxPath = tree.getProperty("cxx").toString();
            checkPaths();
        }
        else {
            // by default, just hope that they are in the $PATH variable
            setPaths("python3", "c++");
        }
    }
    
    String currentPatch;
    String currentObjectID;
    
    static void setPaths(String python, String cxx) {
        pyPath = python;
        cxxPath = cxx;
        
        ValueTree pathsTree("Paths");
        pathsTree.setProperty("python3", python, nullptr);
        pathsTree.setProperty("cxx", cxx, nullptr);
        
        auto pathsFile = workingDir.getChildFile("Paths.xml");
        pathsFile.replaceWithText(pathsTree.toXmlString());
        
        checkPaths();
    }
    
    static void sendError(String error) {
        MemoryOutputStream message;
        message.writeString("All");
        message.writeString("Error");
        message.writeString(error);
        
        dynamic_cast<ChildProcessWorker*>(JUCEApplicationBase::getInstance())->sendMessageToCoordinator(message.getMemoryBlock());
    }
    
    
    static void checkPaths() {
#if JUCE_MAC || JUCE_LINUX
        int pyFound = system((pyPath + " --version").toRawUTF8()) == 0;
        int cxxFound = system((cxxPath + " --version").toRawUTF8()) == 0;
        
        if(!cxxFound) {
            sendError("Error: C++ compiler not found at: " + cxxPath);
        }
        if(!pyFound) {
            sendError("Error: Python3 not found at: " + pyPath);
            return;
        }
        
        // Make sure pip is installed
        system((pyPath + " -m ensurepip").toRawUTF8());
        
        int hvccFound = system((pyPath + " -m pip list | grep hvcc").toRawUTF8()) == 0;
        
        if(!hvccFound) {
            
            system((pyPath + " -m pip install hvcc").toRawUTF8());
        }
        
        
#elif JUCE_WINDOWS
        // ??
#endif
    }
    
    void run() override
    {
        auto libPath = generateLibrary(currentPatch);
        
        MemoryOutputStream message;
        message.writeString(currentObjectID);
        message.writeString("Load");
        message.writeString(libPath);
        dynamic_cast<ChildProcessWorker*>(JUCEApplicationBase::getInstance())->sendMessageToCoordinator(message.getMemoryBlock());
    }
    
    String generateLibrary(String patchContent) {
        auto script = workingDir.getChildFile("run_hvcc.py");
        auto tmpDir = workingDir.getChildFile("tmp");
        tmpDir.createDirectory();
        
        auto saveFile = tmpDir.getChildFile("temp_" + String::toHexString(Random::getSystemRandom().nextInt())).withFileExtension(".pd");
        
        FileOutputStream fstream(saveFile);
        fstream.setNewLineString("\n");
        fstream << patchContent;
        fstream.flush();
        
        auto cxxCommand = cxxPath.isEmpty() ? "c++ " : (cxxPath + " ");
        auto pyCommand = pyPath.isEmpty() ? "python3 " : (pyPath + " ");
        auto hvccCommand = pyCommand + script.getFullPathName();
        
        auto externalName = saveFile.getFileNameWithoutExtension();
        
        auto libDir = tmpDir.getChildFile("lib");
        libDir.createDirectory();
        
        auto generationCommand = hvccCommand + " -o " + tmpDir.getFullPathName() + " -n " + externalName + " " + saveFile.getFullPathName();
        
        auto outPath = libDir.getFullPathName() + "/" + externalName + ".o";
        auto inPath = tmpDir.getFullPathName() + "/c/Heavy_" + externalName + ".cpp";
        auto libPath = libDir.getFullPathName() + "/" + externalName + "." + dllExtension;
        
        auto compileCommand = cxxCommand + compileFlags + " -o " + outPath + " -c " + inPath;
        auto linkCommand = cxxCommand + linkerFlags + " -o " + libPath + " " + outPath;
        
        setenv("PATH", "/usr/bin:/usr/local/bin:/opt/homebrew/bin", 1);
        
        // Generate C++ code
        system(generationCommand.toRawUTF8());
        
        // Compile and link the code
        system(compileCommand.toRawUTF8());
        system(linkCommand.toRawUTF8());
        
        // Clean up
        File(outPath).deleteFile();
        File(tmpDir).getChildFile("c").deleteRecursively();
        File(tmpDir).getChildFile("ir").deleteRecursively();
        File(tmpDir).getChildFile("hv").deleteRecursively();
        
        return libPath;
    }
    
    void compile(const String& patchContent, const String& ID) {
        if(isThreadRunning()) {
            std::cout << "Compile action already running!" << std::endl;
            return;
        }
        currentPatch = patchContent;
        currentObjectID = ID;
        startThread(8);
    }

};

}
