/*******************************************************************************
  Copyright(c) 2022 Ludovic Pollet. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/

#ifndef INDI_SERVER_CONTROLLER_H_
#define INDI_SERVER_CONTROLLER_H_ 1

#include <string>
#include <unistd.h>

#include "ProcessController.h"

/**
 * Interface to the indiserver process.
 *
 * Allows starting it, sending it signals and inspecting exit code
 */
class IndiServerController : public ProcessController
{
        bool fifo;
    public:
        IndiServerController();
        ~IndiServerController();
        void setFifo(bool enable);
        void start(const std::vector<std::string> & args);

        void startDriver(const std::string & driver);

        void addDriver(const std::string & path);

        std::string getUnixSocketPath() const;
        int getTcpPort() const;
};


#endif // INDI_SERVER_CONTROLLER_H_