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
    const String compileFlags = "-DHAVE_STRUCT_TIMESPEC -O3 -ffast-math -funroll-loops -fomit-frame-pointer";
    const String dllExtension = "so";
    const String linkerFlags = "-undefined suppress -flat_namespace -bundle";
    #elif JUCE_MAC
    const String compileFlags = "-std=c++17 -DPD -DUNIX -DMACOSX -I /sw/include   -Ishared -DHAVE_STRUCT_TIMESPEC -O3 -ffast-math -funroll-loops -fomit-frame-pointer -arch arm64 -mmacosx-version-min=10.6";
    const String linkerFlags = "-undefined suppress -flat_namespace -bundle  -arch arm64 -mmacosx-version-min=10.6";
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
        auto libPath = libDir.getFullPathName() + externalName + "." + dllExtension;
        
        
        auto compileCommand = "c++ " + compileFlags + " -o " + outPath + " -c " + inPath;
        auto linkCommand = "cc " + linkerFlags + " -o " + libPath + " " + outPath;
        
        // Generate C++ code
        system(generationCommand.toRawUTF8());
        system(clearCommand.toRawUTF8());
                
        
        // Compile and link the code
        //system(compileCommand.toRawUTF8());
        //system(linkCommand.toRawUTF8());
        assemble(File(inPath).loadFileAsString(), inPath);
        
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
        
        startThread();
        
    }
    
    void assemble(const String& code, const String& filename) {
        const auto* testCodeFileName = filename.toRawUTF8();
        const auto* testCode = code.toRawUTF8();

        ClangJitCompiler compiler;
        
        int fileType = ClangJitSourceType_CXX_File;
        
        compiler.setOptimizeLevel(3);
        setupPredefinedFunctions(compiler);
        compiler.compile(testCodeFileName, fileType, error_handler);
        compiler.finalize();
 
        /*
        typedef int (*mainf_t)();
        mainf_t ff = compiler.getFunctionAddress<mainf_t>("main");
        if (ff) {
            return ff();
        } */
        /*
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
        llvm::InitializeAllTargets();

        // Prepare DiagnosticEngine
        auto* DiagOpts = new clang::DiagnosticOptions();
        clang::TextDiagnosticPrinter *textDiagPrinter =
        new clang::TextDiagnosticPrinter(llvm::errs(),
                                             DiagOpts);
        clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
        clang::DiagnosticsEngine *pDiagnosticsEngine =
        new clang::DiagnosticsEngine(pDiagIDs,
                                             DiagOpts,
                                             textDiagPrinter);


        // Prepare compilation arguments
        std::vector<const char *> args = {testCodeFileName, "-I/Users/timschoen/Downloads/llvm/include/c++/v1", "-I/Library/Developer/CommandLineTools/usr/lib/clang/13.1.6/include", nullptr};
         llvm::ArrayRef<const char *> cla(args.data(), 3);
        auto CI = std::make_shared<clang::CompilerInvocation>();
        llvm::ArrayRef<const char *> cla(args.data(), 3);
        
        clang::CompilerInvocation::CreateFromArgs(*CI, cla, *pDiagnosticsEngine);

        // Map code filename to a memoryBuffer
        llvm::StringRef testCodeData(testCode);
        std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBufferCopy(testCodeData);
        CI->getPreprocessorOpts().addRemappedFile(testCodeFileName, buffer.get());

        // Create and initialize CompilerInstance
        clang::CompilerInstance Clang;
        Clang.setInvocation(CI);
        Clang.createDiagnostics();

        const std::shared_ptr<clang::TargetOptions> targetOptions = std::make_shared<clang::TargetOptions>();
        targetOptions->Triple = std::string("aarch64");
        clang::TargetInfo *pTargetInfo = clang::TargetInfo::CreateTargetInfo(*pDiagnosticsEngine,targetOptions);
        Clang.setTarget(pTargetInfo);

        // Create and execute action
        // CodeGenAction *compilerAction = new EmitLLVMOnlyAction();
        clang::CodeGenAction *compilerAction = new clang::EmitAssemblyAction();
        Clang.ExecuteAction(*compilerAction);

        buffer.release(); */
    }
    
    
    JUCE_DECLARE_SINGLETON(Compiler, false)
};

JUCE_IMPLEMENT_SINGLETON(Compiler)

