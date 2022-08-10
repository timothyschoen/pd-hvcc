#pragma once


namespace hvcc
{

struct Message : public Object
{
    Message(Component* parentComponent, String text, int x, int y) : Object(parentComponent, text, x, y)
    {
    }
    
    void setType(const String& newType) override {
        if(type == newType) return;
        type = newType;
        
        if(getText() != newType) {
            setText(newType, dontSendNotification);
        }
        
        if(iolets.size() != 2) {
            iolets.add(new Iolet(this, false));
            iolets.add(new Iolet(this, true));
        }
        
        auto name = newType.upToFirstOccurrenceOf(" ", false, false);
        
        setSize(std::max(getFont().getStringWidth(newType) + 16, 35), 20);
        resized();
        repaint();
        
        auto updateFn = parent->getProperties()["Update"];
        parent->getProperties()["Update"].getNativeFunction()(var::NativeFunctionArgs(updateFn, nullptr, 0));
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
        
        Path path;
        
        auto b = getLocalBounds().toFloat();
        path.startNewSubPath(b.getTopLeft());
        path.lineTo(b.getTopRight());
        path.lineTo(b.getTopRight().translated(-4, 4));
        path.lineTo(b.getBottomRight().translated(-4, -4));
        path.lineTo(b.getBottomRight());
        path.lineTo(b.getBottomLeft());
        path.lineTo(b.getTopLeft());
        
        g.strokePath(path, PathStrokeType(1.0f));
    }
    
    String getState() override {
        return "#X msg " + String(getX()) + " " + String(getY()) + " " + getText() + ";\n";
    }
    
};

}
