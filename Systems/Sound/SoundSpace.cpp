///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------------------------- Sound Space

//**************************************************************************************************
ZilchDefineType(SoundSpace, builder, type)
{
  ZeroBindComponent();
  type->AddAttribute(ObjectAttributes::cCore);
  ZeroBindDocumented();
  ZeroBindDependency(TimeSpace);

  ZilchBindFieldProperty(mPauseWithTimeSpace);
  ZilchBindFieldProperty(mPitchWithTimeSpace);
  ZilchBindGetterSetter(Paused);
  ZilchBindGetterSetter(Volume);
  ZilchBindGetterSetter(MuteAudio);
  ZilchBindGetterSetter(Decibels);
  ZilchBindGetterSetter(Pitch);
  ZilchBindGetterSetter(Semitones);
  ZilchBindGetter(InputNode)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetter(OutputNode)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetter(SoundNodeInput);
  ZilchBindGetter(SoundNodeOutput);
  ZilchBindMethod(InterpolatePitch);
  ZilchBindMethod(InterpolateSemitones);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindMethod(InterpolateDecibels);
  ZilchBindMethod(PlayCue);
  ZilchBindMethod(PlayCuePaused);
}

//**************************************************************************************************
SoundSpace::SoundSpace() : 
  mPauseWithTimeSpace(true),
  mPitchWithTimeSpace(true),
  mPitchNode(nullptr), 
  mVolumeNode(nullptr),
  mVolume(1.0f),
  mPitch(0.0f),
  mPause(false),
  mLevelPaused(false),
  mEditorMode(false)
{

}

//**************************************************************************************************
SoundSpace::~SoundSpace()
{
  // Remove this space from the system's list
  Z::gSound->RemoveSoundSpace(this, mEditorMode);
  // Remove the output node from the audio system
  if (mSoundNodeInput && mSoundNodeInput->mNode)
  {
      mSoundNodeInput->mNode->DisconnectThisAndAllInputs();
      mSoundNodeInput->mNode->DeleteThisNode();
      mSoundNodeInput->mNode = nullptr;
  }
  // If there is a pitch node, remove it
  if (mPitchNode)
    mPitchNode->DeleteThisNode();
  // If there is a volume node, remove it
  if (mVolumeNode)
    mVolumeNode->DeleteThisNode();

  // Keep the SoundNode from doing anything in destructor
  if (mSoundNodeOutput)
    mSoundNodeOutput->mNode = nullptr;
}

//**************************************************************************************************
void SoundSpace::Initialize(CogInitializer& config)
{
  // Are we in editor mode?
  mEditorMode = !GetGameSession() || GetGameSession()->IsEditorMode();
  // Add this space to the system's list
  Z::gSound->AddSoundSpace(this, mEditorMode);

  // Create the input node
  mSpaceNodeID = Z::gSound->mCounter++;
  mSoundNodeInput = new SoundNode();
  String name = "Space";
  if (mEditorMode)
    name = "EditorSpace";
  mSoundNodeInput->mNode = new Audio::CombineAndPauseNode(name, mSpaceNodeID, this);

  mSoundNodeInput->mCanInsertAfter = false;
  mSoundNodeInput->mCanReplace = false;
  mSoundNodeInput->mCanRemove = false;

  // Create the volume node as the output node
  mSoundNodeOutput = new SoundNode();
  mVolumeNode = new Audio::VolumeNode(name, mSpaceNodeID, this);
  mSoundNodeOutput->mNode = mVolumeNode;
  
  mVolumeNode->AddInput(mSoundNodeInput->mNode);
  mSoundNodeOutput->mCanInsertBefore = false;
  mSoundNodeOutput->mCanReplace = false;
  mSoundNodeOutput->mCanRemove = false;

  Z::gSound->mOutputNode->AddInputNode(mSoundNodeOutput);
}

//**************************************************************************************************
void SoundSpace::Serialize(Serializer& stream)
{
  SerializeNameDefault(mPauseWithTimeSpace, true);
  SerializeNameDefault(mPitchWithTimeSpace, true);
}

//**************************************************************************************************
float SoundSpace::GetVolume()
{
  return mVolume;
}

//**************************************************************************************************
void SoundSpace::SetVolume(float value)
{
  InterpolateVolume(value, 0.0f);
}

//**************************************************************************************************
bool SoundSpace::GetMuteAudio()
{
  if (!mSoundNodeInput->mNode)
    return false;

  return ((Audio::CombineAndPauseNode*)mSoundNodeInput->mNode)->GetMuted();
}

//**************************************************************************************************
void SoundSpace::SetMuteAudio(bool mute)
{
  if (!mSoundNodeInput->mNode)
    return;

  ((Audio::CombineAndPauseNode*)mSoundNodeInput->mNode)->SetMuted(mute);
}

//**************************************************************************************************
void SoundSpace::InterpolateVolume(float value, float interpolationTime)
{
  mVolume = Math::Clamp(value, 0.0f, Audio::MaxVolumeValue);

  if (mVolumeNode)
    mVolumeNode->SetVolume(mVolume, Math::Max(interpolationTime, 0.0f));
}

//**************************************************************************************************
float SoundSpace::GetDecibels()
{
  return Z::gSound->VolumeToDecibels(mVolume);
}

//**************************************************************************************************
void SoundSpace::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void SoundSpace::InterpolateDecibels(float decibels, float interpolationTime)
{
  mVolume = Math::Clamp(Z::gSound->DecibelsToVolume(decibels), 0.0f, Audio::MaxVolumeValue);

  if (mVolumeNode)
    mVolumeNode->SetVolume(mVolume, Math::Max(interpolationTime, 0.0f));
}

//**************************************************************************************************
float SoundSpace::GetPitch()
{
  return mPitch;
}

//**************************************************************************************************
void SoundSpace::SetPitch(float pitch)
{
  InterpolatePitch(pitch, 0.0f);
}

//**************************************************************************************************
void SoundSpace::InterpolatePitch(float pitch, float time)
{
  mPitch = Math::Clamp(pitch, Audio::MinPitchValue, Audio::MaxPitchValue);

  if (!mPitchNode)
  {
    mPitchNode = new Audio::PitchNode("Space", mSpaceNodeID, this);
    mSoundNodeInput->mNode->InsertNodeAfter(mPitchNode);
  }

  mPitchNode->SetPitch(Z::gSound->PitchToSemitones(mPitch), Math::Max(time, 0.0f));
}

//**************************************************************************************************
float SoundSpace::GetSemitones()
{
  return Z::gSound->PitchToSemitones(mPitch);
}

//**************************************************************************************************
void SoundSpace::SetSemitones(float pitch)
{
  InterpolateSemitones(pitch, 0.0f);
}

//**************************************************************************************************
void SoundSpace::InterpolateSemitones(float semitones, float time)
{
  semitones = Math::Clamp(semitones, Audio::MinSemitonesValue, Audio::MaxSemitonesValue);

  mPitch = Z::gSound->SemitonesToPitch(semitones);

  if (!mPitchNode)
  {
    mPitchNode = new Audio::PitchNode("Space", mSpaceNodeID, this);
    mSoundNodeInput->mNode->InsertNodeAfter(mPitchNode);
  }

  mPitchNode->SetPitch(semitones, Math::Max(time, 0.0f));
}

//**************************************************************************************************
bool SoundSpace::GetPaused()
{
  return mPause;
}

//**************************************************************************************************
void SoundSpace::SetPaused(bool pause)
{
  if (!mSoundNodeInput->mNode)
    return;

  if (!mPause && pause)
  {
    ((Audio::CombineAndPauseNode*)mSoundNodeInput->mNode)->SetPaused(true);
  }
  else if (mPause && !pause)
  {
    ((Audio::CombineAndPauseNode*)mSoundNodeInput->mNode)->SetPaused(false);
  }

  mPause = pause;
}

//**************************************************************************************************
HandleOf<SoundInstance> SoundSpace::PlayCue(SoundCue* cue)
{
  if (!cue)
    return nullptr;

  HandleOf<SoundInstance> instance = cue->PlayCue(this, mSoundNodeInput->mNode, false);

  if (instance)
  {
    SoundInstanceEvent eventToSend(instance);
    DispatchEvent(Events::SoundInstancePlayed, &eventToSend);
  }

  return instance;
}

//**************************************************************************************************
HandleOf<SoundInstance> SoundSpace::PlayCuePaused(SoundCue* cue)
{
  if (!cue)
    return nullptr;

  HandleOf<SoundInstance> instance = cue->PlayCue(this, mSoundNodeInput->mNode, true);

  if (instance)
  {
    SoundInstanceEvent eventToSend(instance);
    DispatchEvent(Events::SoundInstancePlayed, &eventToSend);
  }

  return instance;
}

//**************************************************************************************************
Zilch::HandleOf<Zero::SoundNode> SoundSpace::GetSoundNodeInput()
{
  return mSoundNodeInput;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundSpace::GetInputNode()
{
  return mSoundNodeInput;
}

//**************************************************************************************************
Zilch::HandleOf<Zero::SoundNode> SoundSpace::GetSoundNodeOutput()
{
  return mSoundNodeOutput;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundSpace::GetOutputNode()
{
  return mSoundNodeOutput;
}

//**************************************************************************************************
void SoundSpace::Update()
{
  // If this sound space should pause when the level is paused, check for handling that
  if (mPauseWithTimeSpace)
  {
    bool spacePaused = GetOwner()->has(TimeSpace)->GetGloballyPaused();

    // If the level was just paused
    if (spacePaused && !mLevelPaused)
      SetPaused(true);
    // If the level was just un-paused
    else if (!spacePaused && mLevelPaused)
      SetPaused(false);

    mLevelPaused = spacePaused;
  }

  float dt = GetOwner()->has(TimeSpace)->GetDtOrZero();
  float invDt = dt!= 0.0f ? (1.0f / dt) : 0.0f;

  // Check if this sound space should change pitch with time scale
  if (mPitchWithTimeSpace)
  {
    float scale = GetOwner()->has(TimeSpace)->mTimeScale;
    if (scale != 1.0f)
      InterpolatePitch(Math::Log2(scale), dt);
  }

  // Update emitters
  for(InList<SoundEmitter>::range r = mEmitters.All(); !r.Empty(); r.PopFront())
    r.Front().Update(dt);

  // Update listeners
  InList<SoundListener>::range r = mListeners.All();
  for(; !r.Empty(); r.PopFront())
    r.Front().Update(invDt);
}

//**************************************************************************************************
InList<SoundListener>* Zero::SoundSpace::GetListeners()
{
  return &mListeners;
}

}
