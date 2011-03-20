/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_XBOXDRV_DAEMON_HPP
#define HEADER_XBOXDRV_XBOXDRV_DAEMON_HPP

#include <libudev.h>
#include <boost/scoped_ptr.hpp>
#include <glib.h>

#include "controller_slot_config.hpp"
#include "controller_slot_ptr.hpp"
#include "controller_ptr.hpp"

class Options;
class UInput;
class USBGSource;
struct XPadDevice;

class XboxdrvDaemon
{
private:
  static XboxdrvDaemon* s_current;

  const Options& m_opts;
  
  struct udev* m_udev;
  struct udev_monitor* m_monitor;

  boost::scoped_ptr<USBGSource> m_usb_gsource;
 
  typedef std::vector<ControllerSlotPtr> ControllerSlots;
  ControllerSlots m_controller_slots;
  
  typedef std::vector<ControllerPtr> Controllers;
  Controllers m_inactive_controllers;

  std::auto_ptr<UInput> m_uinput;
  GMainLoop* m_gmain;
  
private:
  static void on_sigint(int);
  static XboxdrvDaemon* current() { return s_current; }

public:
  XboxdrvDaemon(const Options& opts);
  ~XboxdrvDaemon();

  void run();

  std::string status();
  void shutdown();

private:
  void create_pid_file();
  void init_uinput();
  void init_udev();
  void init_udev_monitor();

  void init_g_usb();
  void init_g_udev();
  void init_g_dbus();

  ControllerSlotPtr find_free_slot(udev_device* dev);

  void enumerate_udev_devices();
  void cleanup_threads();
  void process_match(struct udev_device* device);
  void print_info(struct udev_device* device);
  void launch_controller_thread(udev_device* dev,
                                const XPadDevice& dev_type, 
                                uint8_t busnum, uint8_t devnum);
  int get_free_slot_count() const;
  void check_thread_status();
  
  void connect(ControllerSlotPtr slot, ControllerPtr controller);
  ControllerPtr disconnect(ControllerSlotPtr slot);

  void on_connect(ControllerSlotPtr slot);
  void on_disconnect(ControllerSlotPtr slot);

  void wakeup();

  bool on_wakeup();
  bool on_udev_data(GIOChannel* channel, GIOCondition condition);

private:
  static gboolean on_udev_data_wrap(GIOChannel* channel, GIOCondition condition, gpointer data);
  static gboolean on_wakeup_wrap(gpointer data);

private:
  XboxdrvDaemon(const XboxdrvDaemon&);
  XboxdrvDaemon& operator=(const XboxdrvDaemon&);
};

#endif

/* EOF */
