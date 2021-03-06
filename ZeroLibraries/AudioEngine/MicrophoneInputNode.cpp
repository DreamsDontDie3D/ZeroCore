///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //-------------------------------------------------------------------------- Microphone Input Node

  //************************************************************************************************
  MicrophoneInputNode::MicrophoneInputNode(Zero::StringParam name, unsigned ID, 
      ExternalNodeInterface* extInt, bool isThreaded) :
    SoundNode(name, ID, extInt, false, false, isThreaded),
    Active(true),
    Volume(1.0f),
    Stopping(false),
    CurrentVolume(1.0f)
  {
    if (!isThreaded)
    {
      SetSiblingNodes(new MicrophoneInputNode(name, ID, nullptr, true));

      if (!gAudioSystem->StartInput())
      {
        Active = false;
        ((MicrophoneInputNode*)GetSiblingNode())->Active = false;
      }
    }
  }

  //************************************************************************************************
  float MicrophoneInputNode::GetVolume()
  {
    return Volume;
  }

  //************************************************************************************************
  void MicrophoneInputNode::SetVolume(float newVolume)
  {
    if (!Threaded)
    {
      // Don't do anything if setting to same value
      if (newVolume == Volume)
        return;

      Volume = newVolume;

      AddTaskForSiblingThreaded(&MicrophoneInputNode::SetVolume, newVolume);
    }
    else
    {
      Volume = newVolume;

      // Don't change volume if we are currently not active or deactivating
      if (Active && !Stopping)
        VolumeInterpolator.SetValues(CurrentVolume, newVolume,
          PropertyChangeFrames);
    }
  }

  //************************************************************************************************
  bool MicrophoneInputNode::GetActive()
  {
    return Active;
  }

  //************************************************************************************************
  void MicrophoneInputNode::SetActive(bool active)
  {
    if (!Threaded)
    {
      // Don't do anything if setting to same value
      if (active == Active)
        return;

      Active = active;
      
      if (Active && !gAudioSystem->StartInput())
        Active = false;

      // Send task to threaded node
      AddTaskForSiblingThreaded(&MicrophoneInputNode::SetActive, Active);
    }
    else
    {
      // Check if we are deactivating
      if (!active && Active)
      {
        // Mark that we are stopping
        Stopping = true;
        // Interpolate volume to 0
        VolumeInterpolator.SetValues(CurrentVolume, 0.0f,
          PropertyChangeFrames);
      }
      // Activating
      else
      {
        // Mark that we are active
        Active = true;
        Stopping = false;
        // Interpolate volume to its previous setting
        VolumeInterpolator.SetValues(CurrentVolume, Volume,
          PropertyChangeFrames);
      }
    }
  }

  //************************************************************************************************
  bool MicrophoneInputNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Active || gAudioSystem->InputBuffer.Empty())
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Start with the number of samples in the system's input buffer
    unsigned samplesToCopy = gAudioSystem->InputBuffer.Size();
    // If this is larger than the output buffer, adjust the amount
    if (samplesToCopy > bufferSize)
      samplesToCopy = bufferSize;

    // Copy the samples from the input buffer to the output buffer
    memcpy(outputBuffer->Data(), gAudioSystem->InputBuffer.Data(), sizeof(float) * samplesToCopy);

    // If we copied less samples than the output buffer size, fill the rest with zeros
    if (samplesToCopy < bufferSize)
      memset(outputBuffer->Data() + samplesToCopy, 0, sizeof(float) * (bufferSize - samplesToCopy));

    // If interpolating volume or volume is not 1.0, apply volume change
    if (!VolumeInterpolator.Finished() || !IsWithinLimit(CurrentVolume, 1.0f, 0.01f))
    {
      // Step through output buffer
      BufferRange outputRange = outputBuffer->All();
      while (!outputRange.Empty())
      {
        // Check if volume is interpolating
        if (!VolumeInterpolator.Finished())
        {
          CurrentVolume = VolumeInterpolator.NextValue();

          // If interpolation is now finished and we are stopping, mark as not active
          if (VolumeInterpolator.Finished() && Stopping)
            Active = false;
        }

        // Apply volume to all channels in this frame
        for (unsigned channel = 0; channel < numberOfChannels; ++channel, outputRange.PopFront())
          outputRange.Front() *= CurrentVolume;
      }
    }

    return true;
  }

}