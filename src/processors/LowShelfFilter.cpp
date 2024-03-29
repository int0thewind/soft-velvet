#include "LowShelfFilter.h"
const juce::String LowShelfFilter::getName() const {
  return "Low Shelf Filter";
}

void LowShelfFilter::prepareToPlay(double sampleRate,
                                   int maximumExpectedSamplesPerBlock) {
  dlog(juce::String("LowShelfFilter::prepareToPlay() method called") +
       juce::String(". Sample rate: ") + juce::String(sampleRate, 1) +
       juce::String(". Samples per block: ") +
       juce::String(maximumExpectedSamplesPerBlock));

  this->savedSampleRate = sampleRate;
  this->lowShelf.prepare(
      {sampleRate, static_cast<juce::uint32>(maximumExpectedSamplesPerBlock),
       2});
  this->requestToUpdateProcessorSpec();
}

void LowShelfFilter::releaseResources() {
  dlog(juce::String("LowShelfFilter::releaseResources() method called."));
  this->lowShelf.reset();
}

void LowShelfFilter::processBlock(juce::AudioBuffer<float>& buffer,
                                  juce::MidiBuffer& midiMessages) {
  if (this->isDirty.load()) {
    dlog(
        "LowShelfFilter::processBlock() detected processor spec update "
        "request. Update it.");
    this->updateProcessorSpec();
    dlog("LowShelfFilter::processBlock() update processor spec finished.");
  }
  juce::dsp::AudioBlock<float> block(buffer);
  juce::dsp::ProcessContextReplacing<float> context(block);
  this->lowShelf.process(context);
}

LowShelfFilter::LowShelfFilter(float _cutoffFreq, float _attenuationDecibel,
                               float _q)
    : BaseAudioProcessor(),
      cutoffFreq(_cutoffFreq),
      attenuationDecibel(_attenuationDecibel),
      q(_q) {
  dlog("LowShelfFilter new instance created");
}

void LowShelfFilter::updateProcessorSpec() {
  dlog("LowShelfFilter::updateProcessorSpec() method called.");
  *(this->lowShelf.state) = *(dsp::IIR::Coefficients<float>::makeLowShelf(
      this->savedSampleRate, this->cutoffFreq, this->q,
      Decibels::decibelsToGain(this->attenuationDecibel)));
  this->isDirty = false;
  dlog("LowShelfFilter::updateProcessorSpec() new filter created.");
}

bool LowShelfFilter::setCutoffFreq(float _cutoffFreq) {
  if (_cutoffFreq != this->cutoffFreq) {
    this->getCallbackLock().enter();
    this->cutoffFreq = _cutoffFreq;
    this->requestToUpdateProcessorSpec();
    this->getCallbackLock().exit();
    dlog(
        "LowShelfFilter::setCutoffFreq() successfully set "
        "the cutoff frequency to " +
        juce::String(_cutoffFreq) + " Hz.");
    return true;
  }
  dlog("LowShelfFilter::setCutoffFreq() value doesn't change from " +
       juce::String(_cutoffFreq) + " Hz.");
  return false;
}

bool LowShelfFilter::setQ(float _q) {
  if (_q != this->q) {
    this->getCallbackLock().enter();
    this->q = _q;
    this->requestToUpdateProcessorSpec();
    this->getCallbackLock().exit();
    dlog("LowShelfFilter::setQ() successfully set the Q to " +
         juce::String(_q));
    return true;
  }
  dlog("LowShelfFilter::setQ() value doesn't change from " + juce::String(_q));
  return false;
}

bool LowShelfFilter::setAttenuationDecibel(float _attenuationDecibel) {
  if (_attenuationDecibel != this->attenuationDecibel) {
    this->getCallbackLock().enter();
    this->attenuationDecibel = _attenuationDecibel;
    this->requestToUpdateProcessorSpec();
    this->getCallbackLock().exit();
    dlog(
        "LowShelfFilter::setAttenuationDecibel() successfully set "
        "the attenuation to " +
        juce::String(_attenuationDecibel) + " dB.");
    return true;
  }
  dlog("LowShelfFilter::setAttenuationDecibel() value doesn't change from " +
       juce::String(_attenuationDecibel) + " dB.");
  return false;
}
