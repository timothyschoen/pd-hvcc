#pragma once
#include "Objects.h"

namespace hvcc
{

struct Iolet : public Component
{
    bool isOutlet;
    
    Iolet(Component* parent, bool outlet) : isOutlet(outlet) {
        setAlwaysOnTop(true);
        parent->addAndMakeVisible(this);
    }
    
    void paint(Graphics& g) {
        g.fillAll(Colours::black);
    }
    
    void mouseEnter(const MouseEvent& e) {
        setMouseCursor(MouseCursor::CrosshairCursor);
    }
    void mouseExit(const MouseEvent& e) {
        setMouseCursor(MouseCursor::NormalCursor);
    }
};



struct Object : public Label
{
    OwnedArray<Iolet> iolets;
    Component* parent;
    
    int numInlets = 0;
    int numOutlets = 0;
    
    bool validObject = false;
    bool isSelected = false;
    
    String type;
    
    Point<int> mouseDownPosition;
    
    Object(Component* parentComponent, String name, int x, int y) : parent(parentComponent)
    {
        setType(name);
        repaint();
        
        setFont(Font(14));
        
        setBounds(x, y, std::max(getFont().getStringWidth(name) + 6, 25), 20);
        
        onEditorHide = [this](){
            setType(getText());
        };
        
        
        onEditorShow = [this](){
            auto* editor = getCurrentTextEditor();
            editor->onTextChange = [this, editor]() {
                auto width = getFont().getStringWidth(editor->getText()) + 15;
                
                if (width > getWidth()) {
                    setSize(width, getHeight());
                }
            };
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
    
    virtual void setType(const String& newType) {
        if(type == newType) return;
        
        type = newType;
        
        validObject = false;
        
        if(getText() != newType) {
            setText(newType, dontSendNotification);
        }
        
        auto name = newType.upToFirstOccurrenceOf(" ", false, false);
        
        int oldNumInlets = numInlets;
        int oldNumOutlets = numOutlets;
        auto info = ObjectInfo::getObjectInfo(StringArray::fromTokens(newType, true));
        
        if(info != std::pair<int, int>(-1, -1)) {
            numInlets = info.first;
            numOutlets = info.second;
            validObject = true;
        }
        else {
            numInlets = 0;
            numOutlets = 0;
            validObject = false;
        }
        
        while (numInlets < oldNumInlets) iolets.remove(--oldNumInlets);
        while (numInlets > oldNumInlets) iolets.insert(oldNumInlets++, new Iolet(this, false));
        while (numOutlets < oldNumOutlets) iolets.remove(numInlets + (--oldNumOutlets));
        while (numOutlets > oldNumOutlets) iolets.insert(numInlets + (++oldNumOutlets), new Iolet(this, true));
        
        setSize(std::max(getFont().getStringWidth(newType) + 6, 35), 20);
        resized();
        repaint();
        
        auto updateFn = parent->getProperties()["Update"];
        parent->getProperties()["Update"].getNativeFunction()(var::NativeFunctionArgs(updateFn, nullptr, 0));
    }
    
    void resized() override {
        int n_in = 0;
        int n_out = 0;
        for(auto* iolet : iolets) {
            const int width = 8;
            const int height = 3;
            
            iolet->setSize(width, height);
            
            if(iolet->isOutlet) {
                bool singleOutlet = (numOutlets == 1) && (n_out == 0);
                float x = singleOutlet ? 0.0f : static_cast<float>(n_out) / (numOutlets - 1);
                x = jmap<float>(x, 0, 1, 0, getWidth() - width);
                
                
                iolet->setTopLeftPosition(x, getHeight() - 3);
                n_out++;
            }
            else {
                bool singleInlet = (numInlets == 1) && (n_in == 0);
                float x = singleInlet ? 0.0f : static_cast<float>(n_in) / (numInlets - 1);
                x = jmap<float>(x, 0, 1, 0, getWidth() - width);
                iolet->setTopLeftPosition(x, 0);
                n_in++;
            }
        }
        
        Label::resized();
    }
    
    void paint(Graphics& g) override {
        
        auto bounds = getLocalBounds();
        
        if (!isBeingEdited())
        {
            const Font font(getFont());
            
            g.setColour (findColour (Label::textColourId));
            g.setFont (font);
            
            auto textArea = getBorderSize().subtractedFrom(bounds);
            
            g.drawFittedText (getText(), textArea, getJustificationType(),
                              jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                              getMinimumHorizontalScale());
            
        }
        
        g.setColour (isSelected ? Colours::blue : findColour (Label::outlineColourId));
        
        if(validObject) {
            g.drawRect (bounds);
        }
        else {
            
            float dashPattern[2];
            dashPattern[0] = 4.0;
            dashPattern[1] = 4.0;
            g.drawDashedLine(Line<float>(0, 0, bounds.getWidth(), 0), dashPattern, 2, 2.0);
            g.drawDashedLine(Line<float>(0, 0, 0, bounds.getHeight()), dashPattern, 2, 2.0);
            g.drawDashedLine(Line<float>(bounds.getWidth(), 0, bounds.getWidth(), bounds.getHeight()), dashPattern, 2, 2.0);
            g.drawDashedLine(Line<float>(bounds.getWidth(), bounds.getHeight(), 0, bounds.getHeight()), dashPattern, 2, 2.0);
            
        }
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
    
    virtual String getState() {
        return "#X obj " + String(getX()) + " " + String(getY()) + " " + getText() + ";\n";
    }
    
};

}
