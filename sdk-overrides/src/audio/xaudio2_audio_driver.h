#pragma once

#ifdef _WIN32

#include <array>

#include <rex/audio/audio_driver.h>
#include <rex/thread.h>

#include <xaudio2.h>

namespace rex::audio {

// The guest renders planar, big-endian 5.1 float frames. XAudio2 accepts the
// six interleaved channels and lets the Windows mastering voice route them to
// the configured endpoint, as Xenia's Windows backend does.
class XAudio2AudioDriver final : public AudioDriver {
 public:
  XAudio2AudioDriver(memory::Memory* memory, rex::thread::Semaphore* semaphore);
  ~XAudio2AudioDriver() override;

  bool Initialize();
  void SubmitFrame(uint32_t samples_ptr) override;
  void Shutdown();

 private:
  class VoiceCallback final : public IXAudio2VoiceCallback {
   public:
    explicit VoiceCallback(rex::thread::Semaphore* semaphore) : semaphore_(semaphore) {}
    void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) override {}
    void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
    void STDMETHODCALLTYPE OnStreamEnd() override {}
    void STDMETHODCALLTYPE OnBufferStart(void*) override {}
    void STDMETHODCALLTYPE OnBufferEnd(void*) override;
    void STDMETHODCALLTYPE OnLoopEnd(void*) override {}
    void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT) override;

   private:
    rex::thread::Semaphore* semaphore_;
  };

  static constexpr size_t kChannels = 6;
  static constexpr size_t kSamplesPerChannel = 256;
  static constexpr size_t kFrameCount = 64;
  static constexpr size_t kSamplesPerFrame = kChannels * kSamplesPerChannel;

  VoiceCallback callback_;
  IXAudio2* engine_ = nullptr;
  IXAudio2MasteringVoice* mastering_voice_ = nullptr;
  IXAudio2SourceVoice* source_voice_ = nullptr;
  std::array<std::array<float, kSamplesPerFrame>, kFrameCount> frames_{};
  size_t next_frame_ = 0;
};

}  // namespace rex::audio

#endif  // _WIN32
