#pragma once

#include "Settings.h"

struct Canvas : public Component, public LassoSource<Component*>, public Timer
{
    LassoComponent<Component*> lasso;
    
    Viewport* viewport;
    Compiler compiler;
    
    OwnedArray<Object> objects;
    OwnedArray<Connection> connections;

    Iolet* connectingIolet = nullptr;
    Object* objectBeingDragged = nullptr;

    
    const int minimumMovementToStartDrag = 10;
    bool didStartDragging = false;
    
    String objectID;
    
    Canvas(Viewport* port, String ID) : objectID(ID), viewport(port), compiler(false) {
        
        setSize(500, 300);
        addAndMakeVisible(&lasso);
        lasso.setAlwaysOnTop(true);
        
        lasso.setColour(LassoComponent<Component*>::lassoFillColourId, Colours::transparentWhite);
        lasso.setColour(LassoComponent<Component*>::lassoOutlineColourId, Colours::black);
        
        setWantsKeyboardFocus(true);
        
        /*
        auto line1 = Line<int>(5, 5, 35, 35);
        auto line2 = Line<int>(6, 5, 36, 5);
        auto dummy = Point<int>();
        
        if(line1.intersects(line2, dummy)) {
            std::cout << "false positive!" << std::endl;
        } */
        
        getProperties().set("Update", var(var::NativeFunction([this](const var::NativeFunctionArgs&) -> var {
            startNewAction();
            return var();
        })));
        
    }
    
    
    void startNewAction() {
        startTimer(100);
    }
    
    void timerCallback() override {
        auto state = getContent();
        saveState(state);
        stopTimer();
    }
    
    void paint(Graphics& g) override
    {
        g.fillAll(Colours::white);
        
        if(connectingIolet) {
            auto start = getLocalArea(connectingIolet, connectingIolet->getLocalBounds()).getCentre();
            
            auto end = getMouseXYRelative();
            
            g.setColour(Colours::black);
            g.drawLine(start.x, start.y, end.x, end.y);
        }
        g.setColour(Colours::black);
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
                for(auto* connection : connections) {
                    connection->setSelected(false);
                }
            }
        }
        if(auto* obj = dynamic_cast<Object*>(e.originalComponent)) {
            selectedComponents.addToSelection(obj);
            obj->setSelected(true);
            objectBeingDragged = obj;
        }
        for (auto* object : getSelectionOfType<Object>())
        {
            object->mouseDownPosition = object->getPosition();
        }
        
        for(auto* object : objects) {
            for(auto* iolet : object->iolets) {
                if(iolet->getScreenBounds().expanded(8).contains(e.getScreenPosition())) {
                    connectingIolet = iolet;
                    break;
                }
            }
        }
    }
    
    void mouseUp(const MouseEvent& e) override
    {
        if(e.originalComponent == this) {
            lasso.endLasso();
        }
        
        if(connectingIolet) {
            for(auto* obj : objects) {
                for(auto* iolet : obj->iolets) {
                    if(iolet->isOutlet == connectingIolet->isOutlet) continue;
                    
                    if(iolet->getScreenBounds().expanded(8).contains(e.getScreenPosition())) {
                        
                        auto* inlet = iolet->isOutlet ? connectingIolet : iolet;
                        auto* outlet = iolet->isOutlet ? iolet : connectingIolet;
                        
                        if(inlet->getParentComponent() == outlet->getParentComponent()) return;
                        
                        auto* connection = connections.add(new Connection(this, inlet, outlet));
                        startNewAction();
                    }
                }
            }
        }
        
        if(e.originalComponent != this && selectedComponents.getNumSelected() && e.mouseWasDraggedSinceMouseDown()) {
            startNewAction();
        }
        
        connectingIolet = nullptr;
        repaint();
        
        checkBounds();
    }
    
    void mouseDrag(const MouseEvent& e) override
    {
        if(connectingIolet) {
            repaint();
        }
        else if(e.originalComponent == this){
            lasso.dragLasso(e);
        }
        
        if(dynamic_cast<Object*>(e.originalComponent)) {
            for (auto* object : getSelectionOfType<Object>())
            {
                // In case we dragged near the edge and the canvas moved
                object->setTopLeftPosition(object->mouseDownPosition + e.getOffsetFromDragStart());
            }
        }
        
    }
    
    void mouseMove(const MouseEvent& e) override {
        
        for(auto* object : objects) {
            for(auto* iolet : object->iolets) {
                if(iolet->getScreenBounds().expanded(8).contains(e.getScreenPosition())) {
                    setMouseCursor(MouseCursor::CrosshairCursor);
                    return;
                }
            }
        }
        
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
            }
        }
        
        if(key == KeyPress::backspaceKey || key == KeyPress::deleteKey) {
            if(selectedComponents.getNumSelected()) startNewAction();
            
            for(auto* connection : getSelectionOfType<Connection>()) {
                selectedComponents.deselect(connection);
                connections.removeObject(connection);
            }
            for(auto* obj : getSelectionOfType<Object>()) {
                selectedComponents.deselect(obj);
                removeObject(obj);
            }
            
        }
    }
    
    String getContent() {
        String save = "#N canvas 63 88 450 300 12;\n";
        
        
        for(auto* obj : objects) {
            save += obj->getState();
        }
        
        for(auto* con : connections) {
           
            int outbox = objects.indexOf(con->outbox);
            int inbox  = objects.indexOf(con->inbox);
            int outlet = con->outbox->iolets.indexOf(con->outlet) - con->outbox->numInlets;
            int inlet  = con->inbox->iolets.indexOf(con->inlet);
            
            save += "#X connect " + String(outbox) + " " + String(outlet) + " " + String(inbox) + " " + String(inlet) + ";\n";
        }
        
        return save;
    }
    
    void loadState(const String& saveContent) {
        MemoryOutputStream ostream;
        Base64::convertFromBase64(ostream, saveContent);
        auto patchContent = String((char*)ostream.getData(), ostream.getDataSize());
        
        connections.clear();
        objects.clear();
        
        for(auto& line : StringArray::fromLines(patchContent)) {
            if(line.startsWith("#X obj"))
            {
                auto objDefinition = StringArray::fromTokens(line, true);
                int x = objDefinition[2].getIntValue();
                int y = objDefinition[3].getIntValue();
                String name = objDefinition.joinIntoString(" ", 4);
                if(name.getLastCharacter() == ';') name = name.dropLastCharacters(1);
                objects.add(new Object(this, name, x, y));
                
            }
            if(line.startsWith("#X connect"))
            {
                auto connectionDefinition = StringArray::fromTokens(line, true);
                int outboxIdx = connectionDefinition[2].getIntValue();
                int outletIdx = connectionDefinition[3].getIntValue();
                int inboxIdx  = connectionDefinition[4].getIntValue();
                int inletIdx = connectionDefinition[5].getIntValue();
                
                auto* outlet = objects[outboxIdx]->iolets[objects[outboxIdx]->numInlets + outletIdx];
                auto* inlet  = objects[inboxIdx]->iolets[inletIdx];
                
                auto* connection = connections.add(new Connection(this, inlet, outlet));
                connection->selfDestruct = [this, connection](){
                    MessageManager::callAsync([this, connection](){
                        connections.removeObject(connection);
                    });
                };
            }
            
        }
    }
   
    void saveState(String state) {
        MemoryOutputStream message;
        message.writeString(objectID);
        message.writeString("SaveState");
        message.writeString(Base64::toBase64(state));
        dynamic_cast<ChildProcessWorker*>(JUCEApplicationBase::getInstance())->sendMessageToCoordinator(message.getMemoryBlock());
        
    }
    
    void recompile() {
        compiler.compile(getContent(), objectID);
    }
    
    void checkBounds() {
        
        auto viewBounds = viewport->getBounds();
        
        for (auto* obj : objects)
        {
            viewBounds = obj->getBounds().getUnion(viewBounds);
        }
        
        if(viewBounds.getX() < 0 || viewBounds.getY() < 0) {
            for (auto* obj : objects)
            {
                int x = viewBounds.getX() < 0 ? obj->getX() - viewBounds.getX() : obj->getX();
                int y = viewBounds.getY() < 0 ? obj->getY() - viewBounds.getY() : obj->getY();
                obj->setTopLeftPosition(x, y);
            }
        }

        setBounds(viewBounds.withZeroOrigin());
    }

        
    template <typename T>
    Array<T*> getSelectionOfType()
    {
        Array<T*> result;

        for (auto* obj : selectedComponents)
        {
            if (auto* objOfType = dynamic_cast<T*>(obj))
            {
                result.add(objOfType);
            }
        }

        return result;
    }
};

struct CanvasHolder : public Component
{
    Canvas cnv;
    Viewport viewport;
    
    CanvasHolder(String ID) : cnv(&viewport, ID)
    {
        setSize(500, 300);
        viewport.setViewedComponent(&cnv);
    
        
        recompileButton.setConnectedEdges(12);
        settingsButton.setConnectedEdges(12);
        
        addAndMakeVisible(viewport);
        addAndMakeVisible(recompileButton);
        addAndMakeVisible(settingsButton);
        
        viewport.setScrollBarsShown(false, false, true, true);
        
        recompileButton.onClick = [this](){
            cnv.recompile();
        };
        
        settingsButton.onClick = [](){
            Settings::showSettingsDialog();
        };
    }
    
    Canvas* getCanvas() {
        return &cnv;
    };
    
    void resized() override
    {
        viewport.setBounds(getLocalBounds());
        
        auto buttonBounds = getLocalBounds().removeFromBottom(20);
        recompileButton.setBounds(buttonBounds.removeFromRight(70));
        settingsButton.setBounds(buttonBounds.removeFromRight(60).translated(2, 0));
    }
    
    TextButton recompileButton = TextButton("Recompile");
    TextButton settingsButton = TextButton("Settings");
    
};
