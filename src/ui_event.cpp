/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ui_event.hpp"

#include "uinput.hpp"

UIEvent
UIEvent::create(int device_id, int type, int code) 
{
  UIEvent ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = device_id;
  ev.m_device_id_resolved = false;
  ev.type      = type;
  ev.code      = code;
  return ev;
}

UIEvent
UIEvent::invalid()
{
  UIEvent ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = DEVICEID_INVALID;
  ev.m_device_id_resolved = false;
  ev.type      = -1;
  ev.code      = -1;
  return ev;    
}

bool
UIEvent::is_valid() const 
{
  return 
    m_device_id != DEVICEID_INVALID &&
    type != -1 &&
    code != -1;
}

bool
UIEvent::operator<(const UIEvent& rhs)  const
{
  if (m_device_id == rhs.m_device_id)
  {
    if (type == rhs.type)
    {
      return code < rhs.code;
    }
    else if (type > rhs.type)
    {
      return false;
    }
    else // if (type < rhs.type)
    {
      return true;
    }
  }
  else if (m_device_id > rhs.m_device_id)
  {
    return false;
  }
  else // if (device_id < rhs.device_id)
  {
    return true;
  }
}

void
UIEvent::resolve_device_id(int slot, bool extra_devices)
{
  if (m_slot_id == SLOTID_AUTO)
  {
    m_slot_id = slot;
  }

  if (m_device_id == DEVICEID_AUTO)
  {
    switch(type)
    {
      case EV_KEY:
        if (uInput::is_mouse_button(code))
        {
          m_device_id = DEVICEID_MOUSE;
        }
        else if (uInput::is_keyboard_button(code))
        {
          m_device_id = DEVICEID_KEYBOARD;
        }
        else
        {
          m_device_id = DEVICEID_JOYSTICK;
        }
        break;

      case EV_REL:
        m_device_id = DEVICEID_MOUSE;
        break;

      case EV_ABS:
        m_device_id = DEVICEID_JOYSTICK;
        break;
    }
  }

  m_device_id_resolved = true;
}

/** Takes "1-BTN_A" splits it into "1", "BTN_A" */
void split_event_name(const std::string& str, std::string* event_str, int* device_id)
{
  std::string::size_type p = str.find('-');
  if (p == std::string::npos)
  {
    *event_str = str;
    *device_id = DEVICEID_AUTO;
  }
  else
  {
    *event_str = str.substr(p+1);
    std::string device = str.substr(0, p);

    if (device == "auto")
    {
      *device_id = DEVICEID_AUTO;
    }
    else if (device == "mouse")
    {
      *device_id = DEVICEID_MOUSE;
    }
    else if (device == "keyboard")
    {
      *device_id = DEVICEID_KEYBOARD;
    }
    else
    {
      *device_id = boost::lexical_cast<int>(device);
    }
  }
}

uint32_t
UIEvent::get_device_id() const
{
  assert(m_device_id_resolved);

  return (m_slot_id << 16) | m_device_id;
}

/* EOF */