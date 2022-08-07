#pragma once

struct Canvas : public Component, public LassoSource<Component*>
{
    LassoComponent<Component*> lasso;
    
    OwnedArray<Object> objects;
    OwnedArray<Connection> connections;
    
    TextButton recompileButton = TextButton("Recompile");
    TextButton autocompileButton = TextButton("Autocompile");
    
    void* pdObject;
    
    Canvas(void* object) : pdObject(object) {
        
        setSize(500, 300);
        
        addAndMakeVisible(&lasso);
        lasso.setAlwaysOnTop(true);
        
        lasso.setColour(LassoComponent<Component*>::lassoFillColourId, Colours::transparentWhite);
        lasso.setColour(LassoComponent<Component*>::lassoOutlineColourId, Colours::black);
        
        setWantsKeyboardFocus(true);
        
        addAndMakeVisible(recompileButton);
        addAndMakeVisible(autocompileButton);
        
        recompileButton.onClick = [this](){
            recompile();
        };
        
        autocompileButton.onClick = [](){
            
        };
        
    }
    
    void resized() override {
        recompileButton.setBounds(getLocalBounds().removeFromBottom(20).removeFromRight(50));
    }

    
    void paint(Graphics& g) override
    {
        g.fillAll(Colours::white);
        
        if(Iolet::connectingIolet) {
            auto start = getLocalArea(Iolet::connectingIolet, Iolet::connectingIolet->getLocalBounds()).getCentre();
            
            auto end = getMouseXYRelative();
            
            g.setColour(Colours::black);
            g.drawLine(start.x, start.y, end.x, end.y);
        }
    }
    
    SelectedItemSet<Component*> selectedComponents;
    
    SelectedItemSet<Component*>& getLassoSelection() override { return selectedComponents; };
    
    void findLassoItemsInArea(Array<Component*>& itemsFound, const Rectangle<int>& area) override
    {
        for(auto* object : objects) {
            if(object->getBounds().intersects(area)) {
                itemsFound.add(object);
                object->setSelected(true);
            }
            else if(!getPeer()->getCurrentModifiersRealtime().isAnyModifierKeyDown()){
                object->setSelected(false);
            }
        }
        
        for(auto* connection : connections) {
            if(connection->intersectsRectangle(area)) {
                itemsFound.add(connection);
                connection->setSelected(true);
            }
            else if(!getPeer()->getCurrentModifiersRealtime().isAnyModifierKeyDown()){
                connection->setSelected(false);
            }
        }
    }

    void mouseDown(const MouseEvent& e) override
    {
        if(e.originalComponent == this) {
            lasso.beginLasso(e, this);
            
            if(!e.mods.isAnyModifierKeyDown()) {
                selectedComponents.deselectAll();
                for(auto* object : objects) {
                    object->setSelected(false);
                }
            }
        }
        if(auto* obj = dynamic_cast<Object*>(e.originalComponent)) {
            selectedComponents.addToSelection(obj);
            obj->setSelected(true);
        }
    }
    
    void mouseUp(const MouseEvent& e) override
    {
        if(e.originalComponent == this) {
            lasso.endLasso();
        }
        
        if(Iolet::connectingIolet) {
            for(auto* obj : objects) {
                for(auto* iolet : obj->iolets) {
                    if(iolet->isOutlet == Iolet::connectingIolet->isOutlet) continue;
                    
                    if(iolet->getScreenBounds().contains(e.getScreenPosition())) {
                        
                        auto* inlet = iolet->isOutlet ? Iolet::connectingIolet : iolet;
                        auto* outlet = iolet->isOutlet ? iolet : Iolet::connectingIolet;
                        
                        auto* connection = connections.add(new Connection(inlet, outlet));
                        addAndMakeVisible(connection);
                    }
                }
            }
        }
        
        Iolet::connectingIolet = nullptr;
        repaint();
    }
    
    void mouseDrag(const MouseEvent& e) override
    {
        if(Iolet::connectingIolet) {
            repaint();
        }
        else if(e.originalComponent == this){
            lasso.dragLasso(e);
        }
    }
    
    void mouseMove(const MouseEvent& e) override {
        setMouseCursor(MouseCursor::NormalCursor);
    }
    
    void removeObject(Object* obj) {
        for(int i = connections.size() - 1; i >= 0; i--) {
            auto* connection = connections[i];
            if(connection->inbox == obj || connection->outbox) {
                connections.remove(i);
            }
        }
        
        objects.removeObject(obj);
    }
    
    bool keyPressed(const KeyPress& key) override {
        if(key.getModifiers().isCommandDown()) {
            if(key.getKeyCode() == 49) {
                auto pos = getMouseXYRelative();
                auto* obj = objects.add(new Object(this, "", pos.x, pos.y));
                obj->showEditor();
            }
            if(key.getKeyCode() == 50) {
                //auto pos = getMouseXYRelative();
                //auto* obj = objects.add(new Message(this, "", pos.x, pos.y));
                //obj->addMouseListener(this, true);
            }
        }
        
        if(key == KeyPress::backspaceKey) {
            for(auto* item : getLassoSelection()) {
                if(auto* connection = dynamic_cast<Connection*>(item)) {
                    selectedComponents.deselect(connection);
                    connections.removeObject(connection);
                }
            }
            for(auto* item : getLassoSelection()) {
                if(auto* obj = dynamic_cast<Object*>(item)) {
                    selectedComponents.deselect(obj);
                    removeObject(obj);
                }
            }
        }
    }
   
    
    void recompile() {
        String save = "#N canvas 63 88 450 300 12;\n";
        
        
        for(auto* obj : objects) {
            save += obj->getState();
        }
        
        for(auto* con : connections) {
           
            int outbox = objects.indexOf(con->outbox);
            int inbox  = objects.indexOf(con->inbox);
            int outlet = con->outbox->iolets.indexOf(con->outlet) - con->outbox->num_in;
            int inlet  = con->inbox->iolets.indexOf(con->inlet);
            
            save += "#X connect " + String(outbox) + " " + String(outlet) + " " + String(inbox) + " " + String(inlet) + ";\n";
        }
        
        auto saveFile = File::createTempFile(".pd");
        
        FileOutputStream fstream(saveFile);
        fstream.setNewLineString("\n");
        fstream << save;
        fstream.flush();
        
        auto atoms = std::vector<t_atom>(1);
        SETSYMBOL(atoms.data(), gensym(saveFile.getFullPathName().toRawUTF8()));
        
        pd_typedmess(static_cast<t_pd*>(pdObject), gensym("open"), 1, atoms.data());
        
    }
    
    
};
