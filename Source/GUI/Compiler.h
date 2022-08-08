#pragma once
#include "../JIT/jit.h"

void setupPredefinedFunctions(ClangJitCompiler& compiler);

extern "C" int error_handler(int level, const char *filename, int line, int column, const char *message)
{
    static int count = 0;
    static char *levelString[] = {"Ignore", "Note", "Remark", "Warning", "Error", "Fatal"};
    printf("%s(%d): %s\n", levelString[level], ++count, message);
    printf("    File: %s\n", filename);
    printf("    Line: %d\n\n", line);
    return 1;
}


struct Compiler : public Thread
{
    const File tmpDir = File::getSpecialLocation(File::tempDirectory);
    const File library = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("hvcc~").getChildFile("Resources");
    const File ccPath = library.getChildFile("cc");
    const File pyPath = library.getChildFile("python");
    const File hvccPath = library.getChildFile("hvcc").getChildFile("hvcc").getChildFile("__init__.py");

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
    
    Compiler() : Thread("Compiler Thread") {
    }
    
    String currentPatch;
    t_pd* currentObject = nullptr;
    
    void run() override
    {
        if(currentPatch.isEmpty() || currentObject == nullptr) return;
        
        auto saveFile = File::createTempFile(".pd");
        
        FileOutputStream fstream(saveFile);
        fstream.setNewLineString("\n");
        fstream << currentPatch;
        fstream.flush();
        
        auto externalName = saveFile.getFileNameWithoutExtension();
        
        auto libDir = tmpDir.getChildFile("lib");
        libDir.createDirectory();
        
        auto generationCommand = "python3 " + hvccPath.getFullPathName() + " -o " + tmpDir.getFullPathName() + " -n " + externalName + " " + saveFile.getFullPathName();
        
        auto clearCommand = "rm -rf " + libDir.getFullPathName() + " && mkdir -p " + libDir.getFullPathName();
        
        auto outPath = libDir.getFullPathName() + "/" + externalName + ".o";
        auto inPath = tmpDir.getFullPathName() + "/c/Heavy_" + externalName + ".cpp";
        auto libPath = libDir.getFullPathName() + "/" + externalName + "." + dllExtension;
        
        auto compileCommand = "c++ " + compileFlags + " -o " + outPath + " -c " + inPath;
        auto linkCommand = "cc " + linkerFlags + " -o " + libPath + " " + outPath;
        
        // Generate C++ code
        system(generationCommand.toRawUTF8());
        system(clearCommand.toRawUTF8());
        
        // Compile and link the code
        system(compileCommand.toRawUTF8());
        system(linkCommand.toRawUTF8());
        
#if ENABLE_LIBCLANG
        assemble(File(inPath).loadFileAsString(), inPath, externalName);
#endif
        
        auto atoms = std::vector<t_atom>(1);
        SETSYMBOL(atoms.data(), gensym(libPath.toRawUTF8()));
        
        pd_typedmess(static_cast<t_pd*>(currentObject), gensym("open"), 1, atoms.data());
    }
    
    void compile(const String& patchContent, t_pd* object) {
        if(isThreadRunning()) {
            std::cout << "Compile action already running!" << std::endl;
            return;
        }
        currentPatch = patchContent;
        currentObject = object;
        
        //startThread();
        run();
    }
    
#if ENABLE_LIBCLANG
    void assemble(const String& code, const String& filename, const String& externalName) {
        const auto* testCodeFileName = filename.toRawUTF8();
        const auto* testCode = code.toRawUTF8();
        
        ClangJitCompiler compiler;
        
        int fileType = ClangJitSourceType_CXX_File;
        
        
        compiler.setWarningLimit(100);
        compiler.setOptimizeLevel(2);
        setupPredefinedFunctions(compiler);
        try
        {
            compiler.compile(testCodeFileName, fileType, error_handler);
            compiler.finalize();
        }
        catch (...)
        {
            std::cerr << "Compilation failed" << std::endl;
        }
        
        auto createFuncName = "hv_" + externalName + "_new";
        auto* createFunc = (void*(*)(double))compiler.getFunctionAddress<void*>(createFuncName.toRawUTF8());
        
        auto* obj = createFunc(44100);
        
       
        
        std::cout << "Done!" << std::endl;
    }
#endif
    
    
    JUCE_DECLARE_SINGLETON(Compiler, false)
};

JUCE_IMPLEMENT_SINGLETON(Compiler)

