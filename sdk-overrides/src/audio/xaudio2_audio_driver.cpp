#ifdef _WIN32

#include "xaudio2_audio_driver.h"

#include <cstring>

#include <ks.h>
#include <ksmedia.h>
#include <mmreg.h>

#include <rex/audio/conversion.h>
#include <rex/audio/downmix.h>
#include <rex/audio/flags.h>
#include <rex/cvar.h>
#include <rex/logging.h>

namespace rex::audio {

XAudio2AudioDriver::XAudio2AudioDriver(memory::Memory* memory,
                                     rex::thread::Semaphore* semaphore)
    : AudioDriver(memory), callback_(semaphore) {}

XAudio2AudioDriver::~XAudio2AudioDriver() { Shutdown(); }

void XAudio2AudioDriver::VoiceCallback::OnBufferEnd(void*) {
  semaphore_->Release(1, nullptr);
}

void XAudio2AudioDriver::VoiceCallback::OnVoiceError(void*, HRESULT error) {
  REXAPU_ERROR("XAudio2 voice error: 0x{:08X}", static_cast<uint32_t>(error));
  semaphore_->Release(1, nullptr);
}

bool XAudio2AudioDriver::Initialize() {
  HRESULT hr = XAudio2Create(&engine_);
  if (FAILED(hr)) {
    REXAPU_ERROR("XAudio2Create failed: 0x{:08X}", static_cast<uint32_t>(hr));
    return false;
  }
  hr = engine_->CreateMasteringVoice(&mastering_voice_);
  if (FAILED(hr)) {
    REXAPU_ERROR("XAudio2 mastering voice failed: 0x{:08X}", static_cast<uint32_t>(hr));
    Shutdown();
    return false;
  }

  WAVEFORMATEXTENSIBLE format{};
  format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
  format.Format.nChannels = kChannels;
  format.Format.nSamplesPerSec = 48000;
  format.Format.wBitsPerSample = 32;
  format.Format.nBlockAlign = kChannels * sizeof(float);
  format.Format.nAvgBytesPerSec = format.Format.nSamplesPerSec * format.Format.nBlockAlign;
  format.Format.cbSize = sizeof(format) - sizeof(WAVEFORMATEX);
  format.Samples.wValidBitsPerSample = 32;
  format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER |
                         SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
  format.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

  hr = engine_->CreateSourceVoice(&source_voice_, &format.Format, 0,
                                  XAUDIO2_DEFAULT_FREQ_RATIO, &callback_);
  if (FAILED(hr)) {
    REXAPU_ERROR("XAudio2 5.1 source voice failed: 0x{:08X}", static_cast<uint32_t>(hr));
    Shutdown();
    return false;
  }
  hr = source_voice_->Start();
  if (FAILED(hr)) {
    REXAPU_ERROR("XAudio2 source start failed: 0x{:08X}", static_cast<uint32_t>(hr));
    Shutdown();
    return false;
  }
  XAUDIO2_VOICE_DETAILS details{};
  mastering_voice_->GetVoiceDetails(&details);
  REXAPU_INFO("XAudio2 native 5.1 output; Windows endpoint {} ch, {} Hz",
              details.InputChannels, details.InputSampleRate);
  return true;
}

void XAudio2AudioDriver::SubmitFrame(uint32_t samples_ptr) {
  if (!source_voice_) return;
  auto& frame = frames_[next_frame_];
  const auto* input = memory_->TranslateVirtual<float*>(samples_ptr);
  if (REXCVAR_GET(audio_mute)) {
    frame.fill(0.0f);
  } else {
    conversion::sequential_6_BE_to_interleaved_6_LE(
        frame.data(), input, kSamplesPerChannel, GetSurroundMix(), GetOutputGain());
  }
  XAUDIO2_BUFFER buffer{};
  buffer.AudioBytes = static_cast<UINT32>(frame.size() * sizeof(float));
  buffer.pAudioData = reinterpret_cast<const BYTE*>(frame.data());
  HRESULT hr = source_voice_->SubmitSourceBuffer(&buffer);
  if (FAILED(hr)) {
    REXAPU_ERROR("XAudio2 SubmitSourceBuffer failed: 0x{:08X}", static_cast<uint32_t>(hr));
    callback_.OnBufferEnd(nullptr);
    return;
  }
  next_frame_ = (next_frame_ + 1) % kFrameCount;
}

void XAudio2AudioDriver::Shutdown() {
  if (engine_) engine_->StopEngine();
  if (source_voice_) {
    source_voice_->DestroyVoice();
    source_voice_ = nullptr;
  }
  if (mastering_voice_) {
    mastering_voice_->DestroyVoice();
    mastering_voice_ = nullptr;
  }
  if (engine_) {
    engine_->Release();
    engine_ = nullptr;
  }
}

}  // namespace rex::audio

#endif  // _WIN32
