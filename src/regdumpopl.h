/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2007 Simon Peter, <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * regdumpopl.h - Fake OPL dumping register values, by Andrea Zoppi <texzk@email.it>
 */

#ifndef H_ADPLUG_REGDUMPOPL
#define H_ADPLUG_REGDUMPOPL

#include "opl.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <ios>

#define REGDUMP_DEF_PATH "regdumpopl.bin"

class CRegDumpOpl : public Copl
{
 public:
  CRegDumpOpl(const std::string &regDumpPath = REGDUMP_DEF_PATH)
    : Copl(),
      regDumpStream(regDumpPath, std::ios::binary),
      tickDelay(0)
    {
    }

  virtual ~CRegDumpOpl()
    {
    }

  virtual void write(int reg, int val)
    {
      uint8_t dumpBytes[3];

      if (tickDelay)
      {
        const uint32_t delayMask = 0x00800000UL;
        uint32_t dumpValue = (tickDelay | delayMask);
        dumpBytes[0] = (uint8_t)((dumpValue >> 16) & 0xFF);
        dumpBytes[1] = (uint8_t)((dumpValue >>  8) & 0xFF);
        dumpBytes[2] = (uint8_t)((dumpValue >>  0) & 0xFF);
        regDumpStream.write(reinterpret_cast<const char *>(dumpBytes),
                            sizeof(dumpBytes));
        tickDelay = 0;
      }

      if ((reg >= 0x0000) && (reg <= 0x7FFF) &&
          (val >= 0x00) && (val <= 0xFF))
      {
        uint32_t dumpValue = (uint32_t)((reg << 8) | val);
        dumpBytes[0] = (uint8_t)((dumpValue >> 16) & 0xFF);
        dumpBytes[1] = (uint8_t)((dumpValue >>  8) & 0xFF);
        dumpBytes[2] = (uint8_t)((dumpValue >>  0) & 0xFF);
        regDumpStream.write(reinterpret_cast<const char *>(dumpBytes),
                            sizeof(dumpBytes));
      }
    }

  virtual void init(void)
    {
      tickDelay = 0;
    }

  virtual void update(short *buf, int samples)
    {
      if ((currChip == 0) && (samples > 0))
      {
        uint32_t tickDelta = (uint32_t)samples;
        tickDelay += tickDelta;
        const uint32_t tickMax = 0x007FFFFFUL;

        while (tickDelay >= tickMax)
        {
          uint8_t dumpBytes[3];
          const uint32_t delayMask = 0x00800000UL;
          uint32_t dumpValue = (tickMax | delayMask);
          dumpBytes[0] = (uint8_t)((dumpValue >> 16) & 0xFF);
          dumpBytes[1] = (uint8_t)((dumpValue >>  8) & 0xFF);
          dumpBytes[2] = (uint8_t)((dumpValue >>  0) & 0xFF);
          regDumpStream.write(reinterpret_cast<const char *>(dumpBytes),
                              sizeof(dumpBytes));
          tickDelay -= tickMax;
        }
      }

      while (samples--)
      {
        *buf++ = 0;
        *buf++ = 0;
      }
    }

 protected:
  uint32_t tickDelay;
  std::ofstream regDumpStream;
};

#endif
