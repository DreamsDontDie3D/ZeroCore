///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
  DefineEvent(SoundStopped);
  DefineEvent(SoundLooped);
  DefineEvent(MusicBeat);
  DefineEvent(MusicBar);
  DefineEvent(MusicEighthNote);
  DefineEvent(MusicQuarterNote);
  DefineEvent(MusicHalfNote);
  DefineEvent(MusicWholeNote);
  DefineEvent(MusicCustomTime);

  DefineEvent(SoundCuePrePlay);
  DefineEvent(SoundCuePostPlay);
}

//----------------------------------------------------------------------------- Sound Instance Event

//**************************************************************************************************
ZilchDefineType(SoundInstanceEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetter(SoundInstance);
}

//----------------------------------------------------------------------------------- Sound Instance

//**************************************************************************************************
ZilchDefineType(SoundInstance, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Volume);
  ZilchBindGetterSetter(Decibels);
  ZilchBindGetterSetter(Pitch);
  ZilchBindGetterSetter(Semitones);
  ZilchBindGetter(IsPlaying);
  ZilchBindGetter(SoundNode);
  ZilchBindMethod(InterpolatePitch);
  ZilchBindMethod(InterpolateSemitones);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindMethod(InterpolateDecibels);
  ZilchBindGetterSetter(Paused);
  ZilchBindMethod(Stop);
  ZilchBindGetterSetter(Looping);
  ZilchBindGetterSetter(Time);
  ZilchBindGetter(FileLength);
  ZilchBindGetterSetter(EndTime);
  ZilchBindGetterSetter(LoopStartTime);
  ZilchBindGetterSetter(LoopEndTime);
  ZilchBindGetterSetter(CustomEventTime);
  ZilchBindGetter(SoundName);

  ZeroBindEvent(Events::SoundLooped, SoundInstanceEvent);
  ZeroBindEvent(Events::SoundStopped, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicBeat, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicBar, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicEighthNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicQuarterNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicHalfNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicWholeNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicCustomTime, SoundInstanceEvent);
}

//**************************************************************************************************
SoundInstance::SoundInstance(Status& status, SoundSpace* space, Audio::SoundAsset* asset, 
    float volume, float pitch) : 
  mSpace(space), 
  mAssetObject(asset)
{
  // Create the SoundInstance
  Audio::SoundInstanceNode* instance = new Audio::SoundInstanceNode(status, "SoundInstance", 
    Z::gSound->mCounter++, mAssetObject, false, true, this);
  // If it was created successfully, set the data
  if (status.Succeeded())
  {
    instance->SetVolume(volume, 0.0f);
    if (pitch != 0.0f)
      instance->SetPitch(Z::gSound->PitchToSemitones(pitch), 0.0f);

    SoundNode* node = new SoundNode();
    node->mNode = instance;
    node->mCanReplace = false;
    node->mCanRemove = false;
    mSoundNode = node;
  }
  else
  {
    DoNotifyWarning("Audio Error", status.Message);

    if (instance)
      instance->DeleteThisNode();
  }
}

//**************************************************************************************************
float SoundInstance::GetVolume()
{
  return GetNode()->GetVolume();
}

//**************************************************************************************************
void SoundInstance::SetVolume(float newVolume)
{
  InterpolateVolume(newVolume, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolateVolume(float newVolume, float interpolationTime)
{
  GetNode()->SetVolume(Math::Max(newVolume, 0.0f), interpolationTime);
}

//**************************************************************************************************
float SoundInstance::GetDecibels()
{
  return Z::gSound->VolumeToDecibels(GetNode()->GetVolume());
}

//**************************************************************************************************
void SoundInstance::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolateDecibels(float decibels, float interpolationTime)
{
  GetNode()->SetVolume(Z::gSound->DecibelsToVolume(decibels), interpolationTime);
}

//**************************************************************************************************
float SoundInstance::GetPitch()
{
  return Z::gSound->SemitonesToPitch(GetNode()->GetPitch());
}

//**************************************************************************************************
void SoundInstance::SetPitch(float newPitch)
{
  InterpolatePitch(newPitch, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolatePitch(float newPitch, float interpolationTime)
{
  GetNode()->SetPitch(Z::gSound->PitchToSemitones(newPitch), interpolationTime);
}

//**************************************************************************************************
float SoundInstance::GetSemitones()
{
  return GetNode()->GetPitch();
}

//**************************************************************************************************
void SoundInstance::SetSemitones(float newSemitones)
{
  InterpolateSemitones(newSemitones, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolateSemitones(float newSemitones, float interpolationTime)
{
  GetNode()->SetPitch(newSemitones, interpolationTime);
}

//**************************************************************************************************
bool SoundInstance::GetPaused()
{
  return GetNode()->GetPaused();
}

//**************************************************************************************************
void SoundInstance::SetPaused(bool pause)
{
  GetNode()->SetPaused(pause);
}

//**************************************************************************************************
void SoundInstance::Stop()
{
  GetNode()->Stop();
}

//**************************************************************************************************
bool SoundInstance::GetIsPlaying()
{
  return GetNode()->IsPlaying();
}

//**************************************************************************************************
SoundNode* SoundInstance::GetSoundNode()
{
  return mSoundNode;
}

//**************************************************************************************************
bool SoundInstance::GetLooping()
{
  return GetNode()->GetLooping();
}

//**************************************************************************************************
void SoundInstance::SetLooping(bool loop)
{
  GetNode()->SetLooping(loop);
}

//**************************************************************************************************
float SoundInstance::GetTime()
{
  return GetNode()->GetTime();
}

//**************************************************************************************************
void SoundInstance::SetTime(float seconds)
{
  GetNode()->JumpTo(seconds);
}

//**************************************************************************************************
float SoundInstance::GetFileLength()
{
  return ((Audio::SoundAssetFromFile*)mAssetObject)->GetLengthOfFile();
}

//**************************************************************************************************
float SoundInstance::GetEndTime()
{
  return GetNode()->GetEndTime();
}

//**************************************************************************************************
void SoundInstance::SetEndTime(float seconds)
{
  GetNode()->SetEndTime(seconds);
}

//**************************************************************************************************
float SoundInstance::GetLoopStartTime()
{
  if (mSoundNode->mNode)
    return ((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetLoopStartTime();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetLoopStartTime(float seconds)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetLoopStartTime(seconds);
}

//**************************************************************************************************
float SoundInstance::GetLoopEndTime()
{
  if (mSoundNode->mNode)
    return ((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetLoopEndTime();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetLoopEndTime(float seconds)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetLoopEndTime(seconds);
}

//**************************************************************************************************
float SoundInstance::GetBeatsPerMinute()
{
  return GetNode()->GetBeatsPerMinute();
}

//**************************************************************************************************
void SoundInstance::SetBeatsPerMinute(float beats)
{
  GetNode()->SetBeatsPerMinute(beats);
}

//**************************************************************************************************
void SoundInstance::SetTimeSignature(float beats, float noteType)
{
  GetNode()->SetTimeSignature((int)beats, (int)noteType);
}

//**************************************************************************************************
float SoundInstance::GetCustomEventTime()
{
  return GetNode()->GetCustomNotifyTime();
}

//**************************************************************************************************
void SoundInstance::SetCustomEventTime(float seconds)
{
  GetNode()->SetCustomNotifyTime(seconds);
}

//**************************************************************************************************
Zero::StringParam SoundInstance::GetSoundName()
{
  return mAssetObject->mName;
}

//**************************************************************************************************
void SoundInstance::Play(bool loop, SoundTag* tag, Audio::SoundNode* outputNode, bool startPaused)
{
  // Save a pointer to the SoundInstance
  Audio::SoundInstanceNode* instance = (Audio::SoundInstanceNode*)mSoundNode->mNode;

  if (tag)
    tag->TagSound(this);
  
  instance->SetLooping(loop);

  // If there is an output node, add the instance as input
  if (outputNode)
    outputNode->AddInput(instance);
  // If there is no output node but there is a SoundSpace, add to direct output of space
  else if (mSpace)
    mSpace->GetInputNode()->AddInputNode(mSoundNode);

  if (!startPaused)
    instance->SetPaused(false);
}

//**************************************************************************************************
void SoundInstance::SendAudioEvent(const Audio::AudioEventTypes::Enum eventType, void* data)
{
  if (eventType == Audio::AudioEventTypes::InstanceFinished)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::SoundStopped, &event);

    // Remove from any SoundTags
    for (unsigned i = 0; i < SoundTags.Size(); ++i)
      SoundTags[i]->SoundInstanceList.EraseValue(Handle(this));
  }
  else if (eventType == Audio::AudioEventTypes::InstanceLooped)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::SoundLooped, &event);
  }
  else if (eventType == Audio::AudioEventTypes::MusicBeat)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicBeat, &event);
  }
  else if (eventType == Audio::AudioEventTypes::MusicBar)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicBar, &event);
  }
  else if (eventType == Audio::AudioEventTypes::MusicEighthNote)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicEighthNote, &event);
  }
  else if (eventType == Audio::AudioEventTypes::MusicQuarterNote)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicQuarterNote, &event);
  }
  else if (eventType == Audio::AudioEventTypes::MusicHalfNote)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicHalfNote, &event);
  }
  else if (eventType == Audio::AudioEventTypes::MusicWholeNote)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicWholeNote, &event);
  }
  else if (eventType == Audio::AudioEventTypes::MusicCustomTime)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicCustomTime, &event);
  }
  else if (eventType == Audio::AudioEventTypes::InterpolationDone)
  {
    SoundEvent event;
    DispatchEvent(Events::AudioInterpolationDone, &event);
  }
  else
    mSoundNode->SendAudioEvent(eventType, data);
}

//**************************************************************************************************
Audio::SoundInstanceNode* SoundInstance::GetNode()
{
  ErrorIf(!mSoundNode->mNode, "SoundInstance node pointer is null");

  return (Audio::SoundInstanceNode*)(mSoundNode->mNode);
}

}//namespace Zero
