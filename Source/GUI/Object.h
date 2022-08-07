#pragma once
#include "Objects.h"

struct Iolet : public Component
{
    bool isOutlet;
    
    Iolet(bool outlet) : isOutlet(outlet) {
        
    }
    
    void paint(Graphics& g) {
        g.fillAll(Colours::black);
    }
    
    
    void mouseDrag(const MouseEvent& e) {
        connectingIolet = this;
    }
    
    void mouseEnter(const MouseEvent& e) {
        setMouseCursor(MouseCursor::CrosshairCursor);
    }
    void mouseExit(const MouseEvent& e) {
        setMouseCursor(MouseCursor::NormalCursor);
    }
    
    static inline Iolet* connectingIolet = nullptr;
};



struct Object : public Label
{
    OwnedArray<Iolet> iolets;
    Component* parent;
    
    ComponentDragger dragger;
    
    int num_in = 0;
    int num_out = 0;
    
    bool validObject = false;
    bool isSelected = false;
    
    Object(Component* parentComponent, String name, int x, int y) : parent(parentComponent)
    {
        setType(name);
        repaint();
        setColour(Label::textColourId, Colours::black);
        setColour(Label::outlineColourId, Colours::black);
        setColour(Label::textWhenEditingColourId, Colours::black);
        setColour(Label::outlineWhenEditingColourId, Colours::black);
        
        setColour(Label::backgroundColourId, Colours::transparentWhite);
        setColour(Label::backgroundWhenEditingColourId, Colours::transparentWhite);
        
        setFont(Font(15));
        
        setBounds(x, y, std::max(getFont().getStringWidth(name) + 6, 25), 20);
        
        onEditorHide = [this](){
            setType(getText());
        };
        
        addMouseListener(parent, true);
        parent->addAndMakeVisible(this);
    }
    
    ~Object() {
        
        removeMouseListener(parent);
    }
    
    
    void setSelected(bool selected)  {
        if(isSelected != selected) {
            isSelected = selected;
            repaint();
        }
    }
    
    void setType(String newType) {
        validObject = false;
        
        if(getText() != newType) {
            setText(newType, dontSendNotification);
        }
        
        iolets.clear();
        
        auto name = newType.upToFirstOccurrenceOf(" ", false, false);
        
        if(objectTypes.count(name)) {
            auto spec = objectTypes.at(name);
            num_in = spec.first;
            num_out = spec.second;
            validObject = true;
        }
        else {
            num_in = 0;
            num_out = 0;
            validObject = false;
        }
        
        for(int i = 0; i < num_in; i++) {
            auto* inlet = iolets.add(new Iolet(false));
            inlet->setAlwaysOnTop(true);
            addAndMakeVisible(inlet);
        }
        for(int i = 0; i < num_out; i++) {
            auto* outlet = iolets.add(new Iolet(true));
            outlet->setAlwaysOnTop(true);
            addAndMakeVisible(outlet);
        }
        
        setSize(std::max(getFont().getStringWidth(newType) + 6, 25), 20);
        resized();
        repaint();
    }
    
    void resized() override {
        int n_in = 0;
        int n_out = 0;
        for(auto* iolet : iolets) {
            const int width = 8;
            const int height = 3;
            
            iolet->setSize(width, height);
            
            if(iolet->isOutlet) {
                bool singleOutlet = (num_out == 1) && (n_out == 0);
                float x = singleOutlet ? 0.0f : static_cast<float>(n_out) / (num_out - 1);
                x = std::clamp<float>(x * getWidth(), 0, getWidth() - width);
                iolet->setTopLeftPosition(x, getHeight() - 3);
                n_out++;
            }
            else {
                bool singleInlet = (num_in == 1) && (n_in == 0);
                float x = singleInlet ? 0.0f : static_cast<float>(n_in) / (num_in - 1);
                x = std::clamp<float>(x * getWidth(), 0, getWidth() - width);
                iolet->setTopLeftPosition(x, 0);
                n_in++;
            }
        }
        
        Label::resized();
    }
    
    void paint(Graphics& g) override {
        
        if (!isBeingEdited())
        {
            const Font font(getFont());
            
            g.setColour (findColour (Label::textColourId));
            g.setFont (font);
            
            auto textArea = getBorderSize().subtractedFrom(getLocalBounds());
            
            g.drawFittedText (getText(), textArea, getJustificationType(),
                              jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                              getMinimumHorizontalScale());
            
        }
        
        g.setColour (isSelected ? Colours::blue : findColour (Label::outlineColourId));
        
        if(validObject) {
            g.drawRect (getLocalBounds());
        }
        else {
            
            float dashPattern[2];
            dashPattern[0] = 4.0;
            dashPattern[1] = 4.0;
            g.drawDashedLine(Line<float>(0, 0, getWidth(), 0), dashPattern, 2, 2.0);
            g.drawDashedLine(Line<float>(0, 0, 0, getHeight()), dashPattern, 2, 2.0);
            g.drawDashedLine(Line<float>(getWidth(), 0, getWidth(), getHeight()), dashPattern, 2, 2.0);
            g.drawDashedLine(Line<float>(getWidth(), getHeight(), 0, getHeight()), dashPattern, 2, 2.0);
            
        }
    }
    
    
    void mouseDown(const MouseEvent& e) override
    {
        dragger.startDraggingComponent(this, e);
    }
    
    void mouseUp(const MouseEvent& e) override
    {
        if(!e.mouseWasDraggedSinceMouseDown()) {
            showEditor();
        }
    }
    
    void mouseMove(const MouseEvent& e) override {
        setMouseCursor(MouseCursor::NormalCursor);
    }
    
    void mouseDrag(const MouseEvent& e) override
    {
        dragger.dragComponent(this, e, nullptr);
    }
    
    String getState() {
        return "#X obj " + String(getX()) + " " + String(getY()) + " " + getText() + ";\n";
    }
    
};
