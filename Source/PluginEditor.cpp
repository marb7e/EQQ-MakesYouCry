/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), 
    peakGainSliderAttachement(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakFreqSliderAttachement(audioProcessor.apvts, "PeakCut Freq", peakFreqSlider),
    peakQualitySliderAttachement(audioProcessor.apvts,"Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachement(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachement(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachement(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachement(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),
    masterVolumeSliderAttachement(audioProcessor.apvts, "Master Volume", masterVolumeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }


   

    setSize (1000, 500);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;

    // Background Color
    g.fillAll (Colours::dimgrey);

    // Response Curve
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto w = responseArea.getWidth();

    auto& lowCut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highCut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags;

    mags.resize(w);

    for (int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

        if (!monoChain.isBypassed <ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!lowCut.isBypassed<0>())
            mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowCut.isBypassed<1>())
            mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowCut.isBypassed<2>())
            mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowCut.isBypassed<3>())
            mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!highCut.isBypassed<0>())
            mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highCut.isBypassed<1>())
            mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highCut.isBypassed<2>())
            mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highCut.isBypassed<3>())
            mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = Decibels::gainToDecibels(mag);

        Path responseCurve;

        const double outputMin = responseArea.getBottom();
        const double outputMax = responseArea.getY();
        auto map = [outputMin, outputMax](double input)
        {
            return jmap(input, -24.0, 24.0, outputMin, outputMax);
        };

        responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

        for (size_t i = 1; i < mags.size(); i++)
        {
            responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
        }

        //Rectangle around response curve
        g.setColour(Colours::transparentBlack);
        g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

        //Response curve color
        g.setColour(Colours::azure);
        g.strokePath(responseCurve, PathStrokeType(2.f));
    }
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //bounding box for the components
    auto bounds = getLocalBounds();

    //dedicated area for future visualizer - cuts a third off the top
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto windowSpacing = bounds.removeFromBottom(bounds.getHeight() * 0.1);
   


    // THE ORDER ACTUALLY MATTERS FOR THE BOUND CALCULATIONS
    
    //Peak Gain Slider
    auto peakGainArea = bounds.removeFromLeft(bounds.getWidth() * 0.1);
    

    peakGainSlider.setBounds(peakGainArea);

    //Master Volume Slider
    auto masterVolumeArea = bounds.removeFromRight(bounds.getWidth() * 0.125);

    masterVolumeSlider.setBounds(masterVolumeArea);

    //Peak Frequency Slider
    
    int sliderWidth = getWidth() * 0.2;
    int sliderHeight = getHeight() * 0.125;
    int sliderXpos = getWidth() * 0.1;
    int sliderFreqYpos = getHeight() * 0.66;

   peakFreqSlider.setBounds(sliderXpos,sliderFreqYpos,sliderWidth,sliderHeight);


    //Peak Quality Slider

    peakQualitySlider.setBounds(sliderXpos, sliderFreqYpos - sliderHeight * 2 , sliderWidth, sliderHeight);

    //Low Cut Freq Knob

    int knobRadius = 100;

    lowCutFreqSlider.setBounds(sliderXpos * 2 + knobRadius , sliderFreqYpos - knobRadius / 3, knobRadius, knobRadius);

    //Low Cut Slope Knob


    lowCutSlopeSlider.setBounds(sliderXpos * 2 + knobRadius * 2, sliderFreqYpos - knobRadius / 3, knobRadius, knobRadius);

    //High Cut Freq Knob
    
    highCutFreqSlider.setBounds(sliderXpos * 2 + knobRadius, sliderFreqYpos - knobRadius - 10 - knobRadius / 3 , knobRadius, knobRadius);

    //High Cut Slope Knob

    highCutSlopeSlider.setBounds(sliderXpos * 2 + knobRadius * 2, sliderFreqYpos - knobRadius - 10 - knobRadius / 3, knobRadius, knobRadius);



 


}

void SimpleEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void SimpleEQAudioProcessorEditor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
}

void SimpleEQAudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        //update the mono chain
        //signal a repaint
    }
}


std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &masterVolumeSlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}
