#pragma once

struct Settings : public Component
{

    
    std::unique_ptr<FileChooser> chooser;
    static inline Settings* instance = nullptr;
    
    Settings() {

        // Position in centre of screen
        setBounds(Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.withSizeKeepingCentre(500, 150));

        pyPathBrowse.onClick = [this](){
            chooser = std::make_unique<FileChooser>("Find Python3", File::getSpecialLocation(File::userHomeDirectory), "*");
            auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

            chooser->launchAsync(chooserFlags,
                [this](FileChooser const& fc) {
                    if (fc.getResult() == File {})
                        return;
                
                pyPathEditor.setText(fc.getResult().getFullPathName());
                    
                });
        };
        cxxPathBrowse.onClick = [this](){
            
            chooser = std::make_unique<FileChooser>("Find C++ Compiler", File::getSpecialLocation(File::userHomeDirectory), "*");
            auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;
            
            chooser->launchAsync(chooserFlags,
                [this](FileChooser const& fc) {
                    if (fc.getResult() == File {})
                        return;
                
                    cxxPathEditor.setText(fc.getResult().getFullPathName());
                    
                });
        };
        
        addAndMakeVisible(pyPathLabel);
        addAndMakeVisible(cxxPathLabel);
        
        pyPathLabel.setColour(Label::outlineColourId, Colours::white);
        cxxPathLabel.setColour(Label::outlineColourId, Colours::white);
    
        
        addAndMakeVisible(pyPathEditor);
        addAndMakeVisible(cxxPathEditor);
    
        addAndMakeVisible(pyPathBrowse);
        addAndMakeVisible(cxxPathBrowse);
        
        pyPathBrowse.setConnectedEdges(12);
        cxxPathBrowse.setConnectedEdges(12);
        
        addAndMakeVisible(okButton);
        addAndMakeVisible(cancelButton);
        
        pyPathEditor.setText(Compiler::pyPath);
        cxxPathEditor.setText(Compiler::cxxPath);
        
        okButton.onClick = [this](){
            String pyPath = pyPathEditor.getText();
            String cxxPath = cxxPathEditor.getText();
            Compiler::setPaths(pyPath, cxxPath);
            setVisible(false);
        };
        
        cancelButton.onClick = [this](){
            setVisible(false);
        };
        
        
        addToDesktop(ComponentPeer::windowHasTitleBar | ComponentPeer::windowHasDropShadow);
        setVisible(true);
        toFront(true);
    }
    
    static void showSettingsDialog() {
        if(!instance) {
            instance = new Settings();
        }
        else {
            instance->setVisible(true);
            instance->toFront(true);
        }
    }
    
    void paint(Graphics& g) override
    {
        g.fillAll(Colours::white);
    }
    
    void resized() override
    {
        pyPathLabel.setBounds(5, 10, 150, 20);
        cxxPathLabel.setBounds(5, 35, 150, 20);
        
        pyPathEditor.setBounds(160, 10, getWidth() - 250, 20);
        cxxPathEditor.setBounds(160, 35, getWidth() - 250, 20);
        
        pyPathBrowse.setBounds(getWidth() - 80, 10, 70, 20);
        cxxPathBrowse.setBounds(getWidth() - 80, 35, 70, 20);
        
        
        okButton.setBounds(130, getHeight() - 40, 70, 30);
        cancelButton.setBounds(300, getHeight() - 40, 70, 30);
    }
    
    Label pyPathLabel = Label("python3", "Path to python3:");
    Label cxxPathLabel = Label("cxx", "Path to C++ compiler:");
    
    TextEditor pyPathEditor;
    TextEditor cxxPathEditor;
    
    TextButton pyPathBrowse = TextButton("Browse");
    TextButton cxxPathBrowse = TextButton("Browse"); 
    
    TextButton okButton = TextButton("OK");
    TextButton cancelButton = TextButton("Cancel");
};
