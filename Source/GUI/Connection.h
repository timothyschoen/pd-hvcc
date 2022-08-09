#pragma once

struct Connection : public Component, public ComponentListener
{
    SafePointer<Iolet> inlet, outlet;
    SafePointer<Object> inbox, outbox;
    
    bool isSelected = false;
    
    std::function<void()> selfDestruct;
    
    Connection(Component* parent, Iolet* inletPtr, Iolet* outletPtr) : inlet(inletPtr), outlet(outletPtr){
        
        inbox = dynamic_cast<Object*>(inlet->getParentComponent());
        outbox = dynamic_cast<Object*>(outlet->getParentComponent());
        
        parent->addAndMakeVisible(this);
        
        auto bounds = Rectangle<int>(inlet->getBounds().getCentre(), outlet->getBounds().getCentre());
        setBounds(bounds);
        
        MessageManager::callAsync([this](){
            componentMovedOrResized(*inbox, false, false);
        });
        
        
        inbox->addComponentListener(this);
        outbox->addComponentListener(this);
    }
    
    ~Connection() {
        if(inbox) inbox->removeComponentListener(this);
        if(outbox) outbox->removeComponentListener(this);
    }
    
    bool hitTest(int x, int y) override {
        return false;
    }
    
    bool intersectsRectangle(Rectangle<int> rect) {
        if(!inlet || !outlet) return;
        
        auto* cnv = getParentComponent();
        auto inpos = cnv->getLocalPoint(inlet, inlet->getLocalBounds().getCentre());
        auto outpos = cnv->getLocalPoint(outlet, outlet->getLocalBounds().getCentre());
        return rect.intersects(Line<int>(inpos, outpos));
    }
    
    void visibilityChanged() override {
        repaint();
    }
    
    void setSelected(bool selected) {
        if(isSelected != selected) {
            isSelected = selected;
            repaint();
        }
    }
    
    void paint(Graphics& g) override {
        
        if(!inlet || !outlet) return;
        
        auto inpos = getLocalPoint(inlet, inlet->getLocalBounds().getCentre().toFloat());
        auto outpos = getLocalPoint(outlet, outlet->getLocalBounds().getCentre().toFloat());
        
        g.setColour(isSelected ? Colours::blue : Colours::black);
        g.drawLine(Line(inpos, outpos), 1.0f);
    }
    
    void componentMovedOrResized(Component& component, bool wasMoved, bool wasResized) override
    {
        if(!inlet || !outlet) return;
        
        auto* cnv = getParentComponent();
        auto inpos = cnv->getLocalPoint(inlet, inlet->getLocalBounds().getCentre());
        auto outpos = cnv->getLocalPoint(outlet, outlet->getLocalBounds().getCentre());
        
        auto bounds = Rectangle<int>(inpos, outpos);
        bounds = bounds.withSizeKeepingCentre(std::max(bounds.getWidth(), 2), std::max(bounds.getHeight(), 2));
        setBounds(bounds);
        repaint();
    }
    
    void componentBeingDeleted (Component &component) override
    {
        selfDestruct();
    }
};
