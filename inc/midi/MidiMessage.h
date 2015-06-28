/*----------------------------------------------------------------------------------------------------------------------

                 %%%%%%%%%%%%%%%%%                
                 %%%%%%%%%%%%%%%%%
                 %%%           %%%
                 %%%           %%%
                 %%%           %%%
%%%%%%%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% www.shaduzlabs.com %%%%%

------------------------------------------------------------------------------------------------------------------------

  Copyright (C) 2014 Vincenzo Pacella

  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
  License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program.  
  If not, see <http://www.gnu.org/licenses/>.

----------------------------------------------------------------------------------------------------------------------*/

#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "util/Types.h"

namespace sl
{
using namespace kio;
namespace midi
{

//----------------------------------------------------------------------------------------------------------------------

#define M_MIDI_BYTE(data) static_cast<uint8_t>(0x7F&data)

//----------------------------------------------------------------------------------------------------------------------

/**
  \class MidiNote
  \brief Class representing a single midi note

*/

class MidiNote
{
public:  
  enum class Name : uint8_t {
    C,
    CSharp, DFlat = CSharp,
    D,
    DSharp, EFlat = DSharp,
    E,
    F,
    FSharp, GFlat = FSharp,
    G,
    GSharp, AFlat = GSharp,
    A,
    ASharp, BFlat = ASharp,
    B,
  };
  
  MidiNote(Name note_, int8_t octave_)
  : m_note(note_)
  , m_octave(std::min<int8_t>(9,std::max<int8_t>(-1,octave_)))
  {
  
  }
  
  void setNote(Name note_){ m_note = note_; }
  
  uint8_t value()
  {
    return (m_octave+1)*12 + static_cast<uint8_t>(m_note);
  }
  
private:
  Name      m_note;
  int8_t    m_octave;
};


class MidiMessage
{
public:

  enum class Type : uint8_t 
  {
    NoteOff = 0x80,
    NoteOn = 0x90,
    PolyPressure = 0xA0,
    ControlChange = 0xB0,
    ProgramChange = 0xC0,
    ChannelPressure = 0xD0,
    PitchBend = 0xE0,
    SysexStart = 0xF0,
    MTC = 0xF1,
    SongPosition = 0xF2,
    SongSelect = 0xF3,
    // 0xF4 is reserved/unsupported
    // 0xF5 is reserved/unsupported
    TuneRequest = 0xF6,
    SysexEnd = 0xF7,
    TimingClock = 0xF8,
    // 0xF9 is reserved/unsupported
    Start = 0xFA,
    Continue = 0xFB,
    Stop = 0xFC,
    // 0xFD is reserved/unsupported
    ActiveSensing = 0xFE,
    Reset = 0xFF,
  };

  enum class Channel : uint8_t
  {
    Ch1,
    Ch2,
    Ch3,
    Ch4,
    Ch5,
    Ch6,
    Ch7,
    Ch8,
    Ch9,
    Ch10,
    Ch11,
    Ch12,
    Ch13,
    Ch14,
    Ch15,
    Ch16,
    Undefined,
  };

  MidiMessage(Type type_)
    : m_type(type_)
  {}

  virtual const tRawData& data() const = 0;
  Type getType() const { return m_type; }

private:

  Type  m_type;

};

//----------------------------------------------------------------------------------------------------------------------

/**
  \class MidiMessage
  \brief Class representing a MIDI message

*/

template<midi::MidiMessage::Type MsgType>
class MidiMessageBase : public midi::MidiMessage
{
public:
  /*
  MidiMessageBase(tRawData data_)
    : midi::MidiMessage(MsgType)
    , m_data( data_ )
  {

  }
  */
  MidiMessageBase(MidiMessage::Channel channel_, tRawData data_)
    : midi::MidiMessage(MsgType)
    , m_data{ static_cast<uint8_t>((static_cast<uint8_t>(channel_) | static_cast<uint8_t>(getType()))) }
  {
    m_data.insert(m_data.end(), data_.begin(), data_.end());
  }

  virtual ~MidiMessageBase() {}

  const tRawData& data() const override { return m_data; }

  MidiMessage::Channel getChannel() const 
  { 
    return m_data.size() == 0 ? MidiMessage::Channel::Undefined : static_cast<MidiMessage::Channel>(m_data[0] & 0x0F); 
  }

  bool operator == (const MidiMessageBase& other_) const
  {
    return (m_data.size() == other_.m_data.size()) && (std::equal(m_data.begin(),m_data.end(),other_.m_data.begin()));
  }

  bool operator != (const MidiMessageBase& other_) const
  {
    return !(operator == (other_));
  }
private:

  tRawData  m_data;
};

//----------------------------------------------------------------------------------------------------------------------

class NoteOff : public midi::MidiMessageBase<midi::MidiMessage::Type::NoteOff>
{
public:

  NoteOff(MidiMessage::Channel channel_, uint8_t note_, uint8_t velocity_)
  : MidiMessageBase(channel_, { M_MIDI_BYTE(note_), M_MIDI_BYTE(velocity_ )})
  {}

  uint8_t getNote() const { return data()[1]; }

  uint8_t getVelocity() const { return data()[2]; }

};

//----------------------------------------------------------------------------------------------------------------------

class NoteOn : public midi::MidiMessageBase<midi::MidiMessage::Type::NoteOn>
{
public:

  NoteOn(MidiMessage::Channel channel_, uint8_t note_, uint8_t velocity_)
    : MidiMessageBase(channel_, { M_MIDI_BYTE(note_), M_MIDI_BYTE(velocity_) })
  {}

  uint8_t getNote() const { return data()[1]; }

  uint8_t getVelocity() const { return data()[2]; }

};

//----------------------------------------------------------------------------------------------------------------------

class PolyPressure : public midi::MidiMessageBase<midi::MidiMessage::Type::PolyPressure>
{
public:

  PolyPressure(MidiMessage::Channel channel_, uint8_t note_, uint8_t pressure_)
    : MidiMessageBase(channel_, { M_MIDI_BYTE(note_), M_MIDI_BYTE(pressure_) })
  {}

  uint8_t getNote() const { return data()[1]; }

  uint8_t getPressure() const { return data()[2]; }
};

//----------------------------------------------------------------------------------------------------------------------

class ControlChange : public midi::MidiMessageBase<midi::MidiMessage::Type::ControlChange>
{
public:

  ControlChange(MidiMessage::Channel channel_, uint8_t control_, uint8_t value_)
    : MidiMessageBase(channel_, { M_MIDI_BYTE(control_), M_MIDI_BYTE(value_) })
  {}

  uint8_t getControl() const { return data()[1]; }

  uint8_t getValue() const { return data()[2]; }

};

//----------------------------------------------------------------------------------------------------------------------


class ProgramChange : public midi::MidiMessageBase<midi::MidiMessage::Type::ProgramChange>
{
public:

  ProgramChange(MidiMessage::Channel channel_, uint8_t program_)
    : MidiMessageBase(channel_, {M_MIDI_BYTE(program_)})
  {}

  uint8_t getProgram() const { return data()[1]; }

};

//----------------------------------------------------------------------------------------------------------------------


class ChannelPressure : public midi::MidiMessageBase<midi::MidiMessage::Type::ChannelPressure>
{
public:

  ChannelPressure(MidiMessage::Channel channel_, uint8_t pressure_)
    : MidiMessageBase(channel_, {M_MIDI_BYTE(pressure_)})
  {}

  uint8_t getPressure() const { return data()[1]; }

};

//----------------------------------------------------------------------------------------------------------------------

class PitchBend : public midi::MidiMessageBase<midi::MidiMessage::Type::PitchBend>
{
public:

  PitchBend(MidiMessage::Channel channel_, uint8_t pitchL_, uint8_t pitchH_)
    : MidiMessageBase(channel_, { M_MIDI_BYTE(pitchL_), M_MIDI_BYTE(pitchH_) })
  {}

  PitchBend(MidiMessage::Channel channel_, uint16_t pitch_)
    : MidiMessageBase(channel_, { M_MIDI_BYTE(pitch_), M_MIDI_BYTE(pitch_ >> 7) })
  {}

  uint16_t getPitch() const
  {
    return static_cast<uint16_t>(data()[1] | (data()[2] << 7));
  }
};

//----------------------------------------------------------------------------------------------------------------------

static tPtr<MidiMessage> parseMidiMessage(const tRawData& data_)
{
  size_t length = data_.size();
  if (length < 1)
  {
    return nullptr;
  }
  uint8_t status = data_[0];
  if ((status < 0x80) || (status == 0xF4) || (status == 0xF5) || (status == 0xF9) || (status == 0xFD))
  {
    return nullptr;
  }
  else if (status < 0xF0)
  {
    MidiMessage::Type type = static_cast<MidiMessage::Type>(status & 0xF0);
    MidiMessage::Channel channel = static_cast<MidiMessage::Channel>(status & 0x0F);

#define M_CHANNEL_MSG_2(idMsg)     \
  case MidiMessage::Type::idMsg:   \
    return length > 1 ? tPtr<idMsg>(new idMsg(channel, data_[1])) : nullptr;
#define M_CHANNEL_MSG_3(idMsg)     \
  case MidiMessage::Type::idMsg:   \
    return length > 2 ? tPtr<idMsg>(new idMsg(channel, data_[1], data_[2])) : nullptr;

    switch (type)
    {
      M_CHANNEL_MSG_3(NoteOff);
      M_CHANNEL_MSG_3(NoteOn);
      M_CHANNEL_MSG_3(PolyPressure);
      M_CHANNEL_MSG_3(ControlChange);
      M_CHANNEL_MSG_2(ProgramChange);
      M_CHANNEL_MSG_2(ChannelPressure);
      M_CHANNEL_MSG_3(PitchBend);
    }
#undef M_CHANNEL_MSG_2
#undef M_CHANNEL_MSG_3
  }
  else
  {

  }

}


class MidiMessageListener
{
public:

  using tCbNoteOff          = std::function<void(tPtr<NoteOff>)>;
  using tCbNoteOn           = std::function<void(tPtr<NoteOn>)>;
  using tCbPolyPressure     = std::function<void(tPtr<PolyPressure>)>;
  using tCbControlChange    = std::function<void(tPtr<ControlChange>)>;
  using tCbProgramChange    = std::function<void(tPtr<ProgramChange>)>;
  using tCbChannelPressure  = std::function<void(tPtr<ChannelPressure>)>;
  using tCbPitchBend        = std::function<void(tPtr<PitchBend>)>;

  virtual ~MidiMessageListener() {}

  void setCallbackNoteOff(tCbNoteOff cbNoteOff_) { m_cbNoteOff = cbNoteOff_; }
  void setCallbackNoteOn(tCbNoteOn cbNoteOn_) { m_cbNoteOn = cbNoteOn_; }
  void setCallbackPolyPressure(tCbPolyPressure cbPolyPressure_) { m_cbPolyPressure = cbPolyPressure_; }
  void setCallbackControlChangee(tCbControlChange cbControlChange_) { m_cbControlChange = cbControlChange_; }
  void setCallbackProgramChang(tCbProgramChange cbProgramChange_) { m_cbProgramChange = cbProgramChange_; }
  void setCallbackChannelPressure(tCbChannelPressure cbChannelPressure_) { m_cbChannelPressure = cbChannelPressure_; }
  void setCallbackPitchBend(tCbPitchBend cbPitchBend_) { m_cbPitchBend = cbPitchBend_; }

  void callbackNoteOff(tPtr<NoteOff> msg_) { if (m_cbNoteOff) { m_cbNoteOff(std::move(msg_)); } }
  void callbackNoteOff(tPtr<NoteOn> msg_) { if (m_cbNoteOn) { m_cbNoteOn(std::move(msg_)); } }
  void callbackNoteOff(tPtr<PolyPressure> msg_) { if (m_cbPolyPressure) { m_cbPolyPressure(std::move(msg_)); } }
  void callbackNoteOff(tPtr<ControlChange> msg_) { if (m_cbControlChange) { m_cbControlChange(std::move(msg_)); } }
  void callbackNoteOff(tPtr<ProgramChange> msg_){ if (m_cbProgramChange) { m_cbProgramChange(std::move(msg_)); } }
  void callbackNoteOff(tPtr<ChannelPressure> msg_) { if (m_cbChannelPressure) { m_cbChannelPressure(std::move(msg_)); } }
  void callbackNoteOff(tPtr<PitchBend> msg_) { if (m_cbPitchBend) { m_cbPitchBend(std::move(msg_)); } }

private:

  tCbNoteOff          m_cbNoteOff;
  tCbNoteOn           m_cbNoteOn;
  tCbPolyPressure     m_cbPolyPressure;
  tCbControlChange    m_cbControlChange;
  tCbProgramChange    m_cbProgramChange;
  tCbChannelPressure  m_cbChannelPressure;
  tCbPitchBend        m_cbPitchBend;
};


static void processMidi(MidiMessageListener* pListener_, const tRawData& data_)
{
  tPtr<MidiMessage> message = parseMidiMessage(data_);
  if (message)
  {
    switch (message->getType())
    {
      case MidiMessage::Type::NoteOff:   
      {
        pListener_->callbackNoteOff(tPtr<NoteOff>(dynamic_cast<NoteOff*>(message.release())));
      }
    }
  }
}

#undef M_MIDI_BYTE

} // midi
} // sl