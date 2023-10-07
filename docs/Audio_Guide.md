# Audio System Guide

## Sound Playback

Play short sound effects using the Sound class:

```cpp
#include <Nebula/Audio.h>

auto sound = nebula::audio::Sound::load("explosion.wav");
sound->play();
sound->setVolume(80);
sound->setPitch(1.2f); // Speed/pitch shift
```

## Music Streaming

For longer audio tracks, use the Music class which streams from disk:

```cpp
auto music = nebula::audio::Music::load("background.ogg");
music->setLooping(true);
music->setVolume(50);
music->play();

// Later
music->pause();
music->stop();
music->setPlaybackPosition(30.0f); // Seek to 30 seconds
```

## 3D Audio

Positional audio with distance attenuation:

```cpp
auto source = nebula::audio::AudioSource::create("footsteps.wav");
source->setPosition({100, 200, 0});
source->setMinDistance(50);   // Full volume within 50px
source->setMaxDistance(500);  // Inaudible beyond 500px
source->setLooping(true);
source->play();

// Update listener position each frame
auto& listener = nebula::audio::AudioListener::get();
listener.setPosition(cameraPosition);
listener.setDirection(cameraDirection);
```

## Audio Effects

Apply real-time effects to audio:

```cpp
auto reverb = nebula::audio::Effect::create(EffectType::Reverb);
reverb->setParameter("roomSize", 0.8f);
reverb->setParameter("damping", 0.5f);
source->addEffect(reverb);

auto echo = nebula::audio::Effect::create(EffectType::Echo);
echo->setParameter("delay", 200.0f); // ms
echo->setParameter("decay", 0.3f);
source->addEffect(echo);
```

## Streaming Audio

For procedural or network audio:

```cpp
auto stream = nebula::audio::AudioStream::create(44100, 2); // sampleRate, channels
stream->onData = []() -> AudioBuffer {
    // Generate or receive audio data
    return buffer;
};
stream->play();
```

## Audio Groups

Organize sounds into groups for global control:

```cpp
auto sfxGroup = nebula::audio::Group::create("sfx");
auto musicGroup = nebula::audio::Group::create("music");

sound->setGroup(sfxGroup);
music->setGroup(musicGroup);

// Global mute/volume for all SFX
sfxGroup->setVolume(100);
musicGroup->setMuted(true);
```
