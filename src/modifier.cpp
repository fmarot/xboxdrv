/* 
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include "modifier.hpp"

void apply_button_map(XboxGenericMsg& msg, std::vector<ButtonMapping>& lst)
{
  XboxGenericMsg newmsg = msg;

  for(std::vector<ButtonMapping>::iterator i = lst.begin(); i != lst.end(); ++i)
    set_button(newmsg, i->lhs, 0);

  for(std::vector<ButtonMapping>::iterator i = lst.begin(); i != lst.end(); ++i)
    set_button(newmsg, i->rhs, get_button(msg, i->lhs) || get_button(newmsg, i->rhs));

  msg = newmsg;  
}

void apply_axis_map(XboxGenericMsg& msg, std::vector<AxisMapping>& lst)
{
  XboxGenericMsg newmsg = msg;

  for(std::vector<AxisMapping>::iterator i = lst.begin(); i != lst.end(); ++i)
    {
      set_axis(newmsg, i->lhs, 0);
    }

  for(std::vector<AxisMapping>::iterator i = lst.begin(); i != lst.end(); ++i)
    {
      int lhs  = get_axis(msg,    i->lhs);
      int nrhs = get_axis(newmsg, i->rhs);

      if (i->invert)
        {
          if (i->lhs == XBOX_AXIS_LT ||
              i->lhs == XBOX_AXIS_RT)
            {
              lhs = 255 - lhs;
            }
          else
            {
              lhs = -lhs;
            }
        }

      set_axis(newmsg, i->rhs, std::max(std::min(nrhs + lhs, 32767), -32768));
    }
  msg = newmsg;
}

ButtonMapping 
ButtonMapping::from_string(const std::string& str)
{
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    {
      if (*i == '=')
        {
          ButtonMapping mapping;
          mapping.lhs = string2btn(std::string(str.begin(), i));
          mapping.rhs = string2btn(std::string(i+1, str.end()));
          
          if (mapping.lhs == XBOX_BTN_UNKNOWN ||
              mapping.rhs == XBOX_BTN_UNKNOWN)
            throw std::runtime_error("Couldn't convert string \"" + str + "\" to button mapping");

          return mapping;
        }
    }
  throw std::runtime_error("Couldn't convert string \"" + str + "\" to button mapping");
}

AxisMapping
AxisMapping::from_string(const std::string& str)
{
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    {
      if (*i == '=')
        {
          AxisMapping mapping;

          std::string lhs(str.begin(), i);
          std::string rhs(i+1, str.end());

          if (lhs.empty() || rhs.empty())
            throw std::runtime_error("Couldn't convert string \"" + str + "\" to axis mapping");

          if (lhs[0] == '-')
            {
              mapping.invert = true;
              mapping.lhs = string2axis(lhs.substr(1));
            }
          else
            {
              mapping.invert = false;
              mapping.lhs = string2axis(lhs);
            }

          mapping.rhs = string2axis(rhs);

          if (mapping.lhs == XBOX_AXIS_UNKNOWN ||
              mapping.rhs == XBOX_AXIS_UNKNOWN)
            throw std::runtime_error("Couldn't convert string \"" + str + "\" to axis mapping");

          return mapping;
        }
    }
  throw std::runtime_error("Couldn't convert string \"" + str + "\" to axis mapping");
}

RelativeAxisMapping
RelativeAxisMapping::from_string(const std::string& str)
{
  /* Format of str: A={SPEED} */
  std::string::size_type i = str.find('=');
  if (i == std::string::npos)
    {
      throw std::runtime_error("Couldn't convert string \"" + str + "\" to RelativeAxisMapping");
    }
  else
    {
      RelativeAxisMapping mapping;
      mapping.axis  = string2axis(str.substr(0, i));
      mapping.speed = boost::lexical_cast<int>(str.substr(i+1, str.size()-i));
      // FIXME: insert some error checking here
      return mapping;
    }
}


AutoFireMapping 
AutoFireMapping::from_string(const std::string& str)
{
  /* Format of str: A={ON-DELAY}[:{OFF-DELAY}]
     Examples: A=10 or A=10:50 
     if OFF-DELAY == nil then ON-DELAY = OFF-DELAY 
  */
  std::string::size_type i = str.find_first_of('=');
  if (i == std::string::npos)
    {
      throw std::runtime_error("Couldn't convert string \"" + str + "\" to AutoFireMapping");
    }
  else
    {
      AutoFireMapping mapping; 
      mapping.button    = string2btn(str.substr(0, i));
      mapping.frequency = atoi(str.substr(i+1, str.size()-i).c_str())/1000.0f;
      return mapping;
    }
}

/* EOF */