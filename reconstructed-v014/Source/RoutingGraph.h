#pragma once
#include "PluginProcessor.h"
#include <array>

// Interactive original Liberty routing graph. The nodes describe and change
// exactly the effect IDs read in the real-time processBlock function.
class LibertyRoutingGraph final : public juce::Component {
public:
    explicit LibertyRoutingGraph(LibertyRomplerAudioProcessor& p) : processor(p) {
        setOpaque(false);
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    }
    void paint(juce::Graphics& g) override {
        const auto background=juce::Colour(0xff151e2b);
        const auto module=juce::Colour(0xff1a2635);
        const auto border=juce::Colour(0xff35485a);
        const auto cyan=juce::Colour(0xff68d9cc);
        const auto white=juce::Colour(0xfff2f4f6);
        const auto quiet=juce::Colour(0xff9cafc3);
        const std::array<juce::Colour,4> accent{{cyan,juce::Colour(0xffe9ba74),
            juce::Colour(0xff988ae5),juce::Colour(0xff77c9a2)}};
        g.setColour(background);g.fillRoundedRectangle(getLocalBounds().toFloat(),9.0f);
        auto label=[&](const juce::String& str,juce::Rectangle<int> b,int font,juce::Colour c,bool bold=false) {
            if (b.isEmpty()) return;
            g.setColour(c);
            g.setFont(juce::Font(juce::FontOptions((float)font,
                bold?juce::Font::bold:juce::Font::plain)));
            g.drawFittedText(str,b.reduced(3,1),juce::Justification::centred,1,0.78f);
        };
        auto chip=[&](juce::Rectangle<int> bounds,const juce::String& title,juce::Colour col,
                       bool selected,bool mute=false) {
            g.setColour(selected?col.withAlpha(0.26f):module);
            g.fillRoundedRectangle(bounds.toFloat(),6.0f);
            g.setColour(selected?col:border);
            g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f),6.0f,selected?1.8f:1.1f);
            label(title,bounds,12,mute?quiet:selected?col:white,true);
        };
        auto arrow=[&](juce::Rectangle<int> from, juce::Rectangle<int> to, juce::Colour color) {
            const float y=(float)(from.getCentreY()+to.getCentreY())/2.0f;
            const float x1=(float)from.getRight()+3.0f,x2=(float)to.getX()-3.0f;
            if (x2<=x1) return;
            g.setColour(color.withAlpha(0.75f));
            g.drawLine(x1,y,x2,y,1.4f);
            juce::Path tip;
            tip.startNewSubPath(x2-5.0f,y-3.5f);
            tip.lineTo(x2,y);
            tip.lineTo(x2-5.0f,y+3.5f);
            g.strokePath(tip,juce::PathStrokeType(1.6f));
        };
        label("4 COUCHES  /  TRAITEMENTS INDIVIDUELS",{10,0,getWidth()-20,18},11,quiet,true);
        for(int layer=0;layer<4;++layer) {
            const auto boxes=layerBoxes(layer);
            const bool enabled=processor.getControls().layers[(size_t)layer].enabled.load();
            chip(boxes[0],"COUCHE "+juce::String(layer+1),accent[(size_t)layer],enabled,!enabled);
            chip(boxes[1],"FILTRE",accent[(size_t)layer],false);
            for(int slot=0;slot<2;++slot) {
                const int id=processor.getControls().layerFx[(size_t)layer][(size_t)slot].load();
                chip(boxes[(size_t)slot+2],id==0?"+ AJOUTER FX":nameFor(id),accent[(size_t)layer],
                     drag.type==HitType::Layer&&drag.layer==layer&&drag.slot==slot,id==0);
            }
            chip(boxes[4],"BUS",accent[(size_t)layer],false);
            for(int i=0;i<4;++i) arrow(boxes[(size_t)i],boxes[(size_t)i+1],accent[(size_t)layer]);
        }
        const auto master=masterBoxes();
        label(juce::String::fromUTF8(u8"CHA\u00ceNE GLOBALE  /  GLISSER POUR R\u00c9ORGANISER"),{10,238,getWidth()-20,18},11,quiet,true);
        for(int i=0;i<6;++i) {
            const int id=processor.getControls().masterFx[(size_t)i].load();
            chip(master[(size_t)i],nameFor(id),cyan,drag.type==HitType::Master&&drag.slot==i);
            if(i>0)arrow(master[(size_t)i-1],master[(size_t)i],cyan);
        }
        const auto palette=paletteBoxes();
        label("PALETTE D'EFFETS  /  GLISSER SUR UN SLOT DE COUCHE",{10,306,getWidth()-20,18},11,quiet,true);
        for(int i=0;i<6;++i) chip(palette[(size_t)i],nameFor(i+1),juce::Colour(0xffe9ba74),
                                   drag.type==HitType::Palette&&drag.slot==i);
        label(juce::String::fromUTF8(u8"Double-clic : vider un slot  -  Clic droit : retirer  -  Glisser : d\u00e9placer un effet"),
              {10,getHeight()-26,getWidth()-20,21},11,quiet);
        if(drag.type!=HitType::None && moving) {
            g.setColour(cyan.withAlpha(0.85f));
            g.drawEllipse((float)cursor.x-7,(float)cursor.y-7,14.0f,14.0f,2.0f);
        }
    }
    void mouseDown(const juce::MouseEvent& event) override {
        drag=hit(event.getPosition());
        cursor=event.getPosition();moving=false;
        if(event.mods.isRightButtonDown()&&drag.type==HitType::Layer) {
            processor.getControls().layerFx[(size_t)drag.layer][(size_t)drag.slot].store(0);
            drag={};
        }
        repaint();
    }
    void mouseDrag(const juce::MouseEvent& event) override {
        if(drag.type==HitType::None)return;
        cursor=event.getPosition();moving=true;repaint();
    }
    void mouseUp(const juce::MouseEvent& event) override {
        if(!moving){drag={};repaint();return;}
        const auto dest=hit(event.getPosition());
        auto& c=processor.getControls();
        if(drag.type==HitType::Master && dest.type==HitType::Master && dest.slot!=drag.slot) {
            const int oldA=c.masterFx[(size_t)drag.slot].load();
            const int oldB=c.masterFx[(size_t)dest.slot].load();
            c.masterFx[(size_t)drag.slot].store(oldB);
            c.masterFx[(size_t)dest.slot].store(oldA);
        } else if(drag.type==HitType::Palette && dest.type==HitType::Layer) {
            c.layerFx[(size_t)dest.layer][(size_t)dest.slot].store(drag.slot+1);
        } else if(drag.type==HitType::Layer && dest.type==HitType::Layer &&
                  (dest.slot!=drag.slot || dest.layer!=drag.layer)) {
            const int a=c.layerFx[(size_t)drag.layer][(size_t)drag.slot].load();
            const int b=c.layerFx[(size_t)dest.layer][(size_t)dest.slot].load();
            c.layerFx[(size_t)drag.layer][(size_t)drag.slot].store(b);
            c.layerFx[(size_t)dest.layer][(size_t)dest.slot].store(a);
        }
        moving=false;drag={};repaint();
    }
    void mouseDoubleClick(const juce::MouseEvent& event) override {
        auto h=hit(event.getPosition());
        if(h.type==HitType::Layer)
            processor.getControls().layerFx[(size_t)h.layer][(size_t)h.slot].store(0);
        repaint();
    }
    void resized() override {repaint();}
private:
    enum class HitType {None,Layer,Master,Palette};
    struct Hit {HitType type=HitType::None; int layer=-1,slot=-1;};
    LibertyRomplerAudioProcessor& processor;
    Hit drag;
    bool moving=false;
    juce::Point<int> cursor;
    static juce::String nameFor(int id) {
        switch(id){case 1:return "DISTORSION";case 2:return "EQ";
            case 3:return "COMPRESSEUR";case 4:return "CHORUS";
            case 5:return "DELAY";case 6:return juce::String::fromUTF8(u8"R\u00c9VERB\u00c9RATION");
            default:return "BYPASS";}
    }
    std::array<juce::Rectangle<int>,5> layerBoxes(int row) const {
        const int widths[5]={88,92,142,142,80};
        const int total=88+92+142+142+80;
        const int gap=juce::jmax(8,(getWidth()-28-total)/4);
        int x=14;const int y=22+row*52;
        std::array<juce::Rectangle<int>,5> result;
        for(int i=0;i<5;++i){result[(size_t)i]={x,y,widths[i],37};x+=widths[i]+gap;}
        return result;
    }
    std::array<juce::Rectangle<int>,6> masterBoxes() const {
        const int gap=12;
        const int width=juce::jlimit(85,140,(getWidth()-40-5*gap)/6);
        const int xStart=juce::jmax(10,(getWidth()-(6*width+5*gap))/2);
        std::array<juce::Rectangle<int>,6> out;
        for(int i=0;i<6;++i)out[(size_t)i]={xStart+i*(width+gap),262,width,39};
        return out;
    }
    std::array<juce::Rectangle<int>,6> paletteBoxes() const {
        const int gap=10;
        const int width=juce::jlimit(85,140,(getWidth()-40-5*gap)/6);
        const int xStart=juce::jmax(10,(getWidth()-(6*width+5*gap))/2);
        std::array<juce::Rectangle<int>,6> out;
        for(int i=0;i<6;++i)out[(size_t)i]={xStart+i*(width+gap),330,width,36};
        return out;
    }
    Hit hit(juce::Point<int> point) const {
        for(int row=0;row<4;++row){
            const auto b=layerBoxes(row);
            for(int i=2;i<4;++i)if(b[(size_t)i].contains(point))return {HitType::Layer,row,i-2};
        }
        const auto master=masterBoxes();
        for(int i=0;i<6;++i)if(master[(size_t)i].contains(point))return {HitType::Master,-1,i};
        const auto palette=paletteBoxes();
        for(int i=0;i<6;++i)if(palette[(size_t)i].contains(point))return {HitType::Palette,-1,i};
        return {};
    }
};
