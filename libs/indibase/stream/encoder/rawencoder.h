/*
    Copyright (C) 2017 by Jasem Mutlaq <mutlaqja@ikarustech.com>

    Encoder Interface

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#pragma once

#include "encoderinterface.h"
#include <vector>
namespace INDI
{

/**
 * @brief The RawEncoder class sends the image as-is (lossless) to the client.
 *
 * It supports compression via zlib (.stream.z)
 */
class RawEncoder : public EncoderInterface
{
    public:
        RawEncoder();
        ~RawEncoder();

        virtual bool upload(INDI::WidgetViewBlob *bp, const uint8_t *buffer, uint32_t nbytes, bool isCompressed = false) override;

    private:
        const char *getDeviceName();
        std::vector<uint8_t> compressedFrame;

};

}
