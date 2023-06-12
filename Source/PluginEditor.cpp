/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//===========================================================================

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(Colours::slategrey);
    g.fillEllipse(bounds);

    g.setColour(Colours::lightskyblue);
    g.drawEllipse(bounds, 1.f);

    auto center = bounds.getCentre();

    Path p;

    Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY());

    p.addRectangle(r);

    jassert(rotaryStartAngle < rotaryEndAngle);

    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

    g.setColour(Colours::lightskyblue);

    p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

    g.fillPath(p);

}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f + MathConstants<float>::twoPi);

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    getLookAndFeel().drawRotarySlider(g, 
        sliderBounds.getX(), 
        sliderBounds.getY(), 
        sliderBounds.getWidth(), 
        sliderBounds.getHeight(),
        jmap<float>(getValue(), range.getStart(), range.getEnd(), 0.0f, 1.0f), 
        startAng, 
        endAng, 
        *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    return getLocalBounds();
}

void CustomLinearHorizontalSlider::paint(juce::Graphics& g)
{
    using namespace juce;

    auto minPos = 0;
    auto maxPos = 10;

    auto sliderBounds = getSliderBounds();

    getLookAndFeel().drawLinearSlider(g,
        sliderBounds.getX() + 10,
        sliderBounds.getY(),
        sliderBounds.getWidth() - 20,
        sliderBounds.getHeight(),
        1.0f,
        minPos,
        maxPos,
        Slider::LinearHorizontal,
        *this);



}

juce::Rectangle<int> CustomLinearHorizontalSlider::getSliderBounds() const
{
    return getLocalBounds();
}

void CustomLinearVerticalSlider::paint(juce::Graphics& g)
{
    using namespace juce;

    auto minPos = 0;
    auto maxPos = 10;

    auto sliderBounds = getSliderBounds();

    getLookAndFeel().drawLinearSlider(g,
        sliderBounds.getX(),
        sliderBounds.getY() + 10,
        sliderBounds.getWidth(),
        sliderBounds.getHeight() - 20,
        1.0f,
        minPos,
        maxPos,
        Slider::LinearBarVertical,
        *this);
        

}

juce::Rectangle<int> CustomLinearVerticalSlider::getSliderBounds() const
{
    return getLocalBounds();
}


ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}


void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
}

void ResponseCurveComponent::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        //update the mono chain
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

        updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
        updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
        //signal a repaint
        repaint();
    }
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    // Response Curve
    auto responseArea = getLocalBounds();

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
        g.setColour(Colours::lightskyblue);
        g.strokePath(responseCurve, PathStrokeType(0.5f));
    }
}

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),

    peakFreqSlider(*audioProcessor.apvts.getParameter("PeakCut Freq"), "Hz"),
    peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
    peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "db/Oct"),
    highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "db/Oct"),
    masterVolumeSlider(*audioProcessor.apvts.getParameter("Master Volume"), "dB"),
    


    responseCurveComponent(audioProcessor),
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

    setSize(1000, 500);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
   
}

//==============================================================================


void SimpleEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;

    // Background Color
    g.fillAll(Colours::dimgrey);

}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //bounding box for the components
    auto bounds = getLocalBounds();

    //dedicated area for future visualizer - cuts a third off the top
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    responseCurveComponent.setBounds(responseArea);


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
        &highCutSlopeSlider,
        &responseCurveComponent
    };
}


