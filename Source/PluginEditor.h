/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"



struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                           juce::Slider::TextEntryBoxPosition::TextBoxAbove)
    {

    }
};

struct CustomLinearVerticalSlider : juce::Slider
{
    CustomLinearVerticalSlider() : juce::Slider(juce::Slider::SliderStyle::LinearVertical,
                                   juce::Slider::TextEntryBoxPosition::TextBoxAbove)
                                   
    {

    }

};

struct CustomLinearHorizontalSlider : juce::Slider
{
    CustomLinearHorizontalSlider() : juce::Slider(juce::Slider::SliderStyle::LinearHorizontal,
                                     juce::Slider::TextBoxAbove)

    {

    }

};
//
//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor : public juce::AudioProcessorEditor,
    juce::AudioProcessorParameter::Listener,
    juce::Timer

{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    SimpleEQAudioProcessor& audioProcessor;

    juce::Atomic<bool> parametersChanged{ false };

    CustomRotarySlider lowCutFreqSlider,
                       highCutFreqSlider,
                       lowCutSlopeSlider,
                       highCutSlopeSlider;

    CustomLinearVerticalSlider peakGainSlider,
                               masterVolumeSlider;
                               

    CustomLinearHorizontalSlider peakFreqSlider,
                                 peakQualitySlider;
                               
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachement = juce::AudioProcessorValueTreeState::SliderAttachment;

    Attachement
        peakFreqSliderAttachement,
        peakGainSliderAttachement,
        masterVolumeSliderAttachement,
        peakQualitySliderAttachement,
        lowCutFreqSliderAttachement,
        highCutFreqSliderAttachement,
        lowCutSlopeSliderAttachement,
        highCutSlopeSliderAttachement;
    
    std::vector<juce::Component*> getComps();

    MonoChain monoChain;

        

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};
