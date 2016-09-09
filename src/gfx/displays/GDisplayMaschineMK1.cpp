/*
        ##########    Copyright (C) 2015 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #####*/

#include "gfx/displays/GDisplayMaschineMK1.h"

#include "util/Functions.h"

//--------------------------------------------------------------------------------------------------

namespace
{
const uint16_t kMASMK1_displayWidth = 255;        // Width of the display in pixels
const uint16_t kMASMK1_displayHeight = 64;        // Height of the display in pixels
const uint16_t kMASMK1_nOfDisplayDataChunks = 22; // N. of display data chunks
} // namespace

//--------------------------------------------------------------------------------------------------

namespace sl
{
namespace cabl
{

//--------------------------------------------------------------------------------------------------

GDisplayMaschineMK1::GDisplayMaschineMK1()
  : GDisplay(kMASMK1_displayWidth, kMASMK1_displayHeight, kMASMK1_nOfDisplayDataChunks)
{
  initialize();
}

//--------------------------------------------------------------------------------------------------

void GDisplayMaschineMK1::initializeImpl()
{
  buffer().resize(canvasWidthInBytesImpl() * height());
  black();
}

//--------------------------------------------------------------------------------------------------

uint16_t GDisplayMaschineMK1::canvasWidthInBytesImpl() const
{
  uint16_t canvasWitdhInBytes = static_cast<uint16_t>((width() / 3.0f) * 2);
  return canvasWitdhInBytes;
}

//--------------------------------------------------------------------------------------------------

void GDisplayMaschineMK1::white()
{
  fill(0x00);
  setDirty();
}

//--------------------------------------------------------------------------------------------------

void GDisplayMaschineMK1::black()
{
  fill(0xff);
  setDirty();
}

//--------------------------------------------------------------------------------------------------

void GDisplayMaschineMK1::setPixelImpl(
  uint16_t x_, uint16_t y_, const util::ColorRGB& color_, bool bSetDirtyChunk_)
{
  if (x_ >= width() || y_ >= height() || color_.transparent())
  {
    return;
  }

  util::ColorRGB oldColor = pixelImpl(x_, y_);
  unsigned byteIndex = (canvasWidthInBytes() * y_) + ((x_ / 3) * 2);
  uint8_t pixelValue{0};
  if (color_.blendMode() == BlendMode::Invert)
  {
    util::ColorRGB newColor = oldColor;
    newColor.invert();
    pixelValue = (static_cast<uint8_t>((newColor.mono() / 255.0) * 31 + 0.5f));
  }
  else
  {
    pixelValue = (static_cast<uint8_t>((color_.mono() / 255.0) * 31 + 0.5f));
  }

  switch (x_ % 3)
  {
    case 0:
      buffer()[byteIndex] |= 0xF8;
      buffer()[byteIndex] &= ~(pixelValue << 3);
      break;
    case 1:
      buffer()[byteIndex] |= 0x07;
      buffer()[byteIndex + 1] |= 0xC0;
      buffer()[byteIndex] &= ~(pixelValue >> 2);
      buffer()[byteIndex + 1] &= ~(pixelValue << 6);
      break;
    case 2:
      buffer()[byteIndex + 1] |= 0x1F;
      buffer()[byteIndex + 1] &= ~(pixelValue);
      break;
  }

  if (bSetDirtyChunk_ && oldColor != color_)
  {
    setDirtyChunk(y_);
  }
}

//--------------------------------------------------------------------------------------------------

util::ColorRGB GDisplayMaschineMK1::pixelImpl(uint16_t x_, uint16_t y_) const
{
  if (x_ >= width() || y_ >= height())
  {
    return {};
  }

  uint8_t blockIndex = x_ % 3; // 5 bits per pixel, 2 bytes pack 3 pixels
  unsigned byteIndex = (canvasWidthInBytes() * y_) + ((x_ / 3) * 2);
  uint8_t pixelValue{0};
  switch (blockIndex)
  {
    case 0:
      pixelValue = ~(static_cast<uint8_t>((((buffer()[byteIndex] & 0xF8) >> 3) / 31.0) * 255));
      break;
    case 1:
      pixelValue = ~(static_cast<uint8_t>(
        ((((buffer()[byteIndex] & 0x07) << 2) | (buffer()[byteIndex + 1] & 0xC0) >> 6) / 31.0)
        * 255));
      break;
    case 2:
      pixelValue = ~(static_cast<uint8_t>(((buffer()[byteIndex + 1] & 0x1F) / 31.0) * 255));
      break;
  }

  return {pixelValue};
}

//--------------------------------------------------------------------------------------------------

} // namespace cabl
} // namespace sl
