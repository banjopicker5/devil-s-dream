/*
    Copyright (C) 2017 by Jasem Mutlaq <mutlaqja@ikarustech.com>

    INDI Raw Encoder

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

#include "rawencoder.h"
#include "stream/streammanager.h"
#include "indiccd.h"

#include <zlib.h>

namespace INDI
{

RawEncoder::RawEncoder()
{
    name = "RAW";
}

RawEncoder::~RawEncoder()
{
}

const char *RawEncoder::getDeviceName()
{
    return currentDevice->getDeviceName();
}

bool RawEncoder::upload(INDI::WidgetViewBlob *bp, const uint8_t *buffer, uint32_t nbytes, bool isCompressed)
{
    // Do we want to compress ?
    if (isCompressed)
    {
        // Compress frame
        compressedFrame.resize(nbytes + nbytes / 64 + 16 + 3);
        uLongf compressedBytes = compressedFrame.size();

        int ret = compress2(compressedFrame.data(), &compressedBytes, buffer, nbytes, 4);
        if (ret != Z_OK)
        {
            // this should NEVER happen
            LOGF_ERROR("internal error - compression failed: %d", ret);
            return false;
        }

        // Send it compressed
        bp->setBlob(compressedFrame.data());
        bp->setBlobLen(compressedBytes);
        bp->setSize(nbytes);
        bp->setFormat(".stream.z");
    }
    else
    {
        // Send it uncompressed
        bp->setBlob((const_cast<uint8_t *>(buffer)));
        bp->setBlobLen(nbytes);
        bp->setSize(nbytes);
        bp->setFormat(".stream");
    }

    return true;
}
}
