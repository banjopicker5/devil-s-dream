/*
   INDI Developers Manual
   Tutorial #5 - Snooping

   Dome

   Refer to README, which contains instruction on how to build this driver, and use it
   with an INDI-compatible client.

*/

/** \file dome.h
    \brief Construct a dome device that the user may operate to open or close the dome shutter door. This driver is \e snooping on the Rain Detector rain property status.
    If rain property state is alert, we close the dome shutter door if it is open, and we prevent the user from opening it until the rain threat passes.
    \author Jasem Mutlaq

    \example dome.h
    The dome driver \e snoops on the rain detector signal and watches whether rain is detected or not. If it is detector and the dome is closed, it performs
    no action, but it also prevents you from opening the dome due to rain. If the dome is open, it will automatically starts closing the shutter. In order
    snooping to work, both drivers must be started by the same indiserver (or chained INDI servers):
    \code indiserver tutorial_dome tutorial_rain \endcode
    The dome driver keeps a copy of RainL light property from the rain driver. This makes it easy to parse the property status once an update from the rain
    driver arrives in the dome driver. Alternatively, you can directly parse the XML root element in ISSnoopDevice(XMLEle *root) to extract the required data.
*/

#pragma once

#include <defaultdevice.h>
#include <indipropertyswitch.h>
#include <indipropertylight.h>

class Dome : public INDI::DefaultDevice
{
    public:
        Dome() = default;

    public:
        void closeShutter();
        void openShutter();

    protected:
        // General device functions
        bool Connect() override;
        bool Disconnect() override;
        const char *getDefaultName() override;
        bool initProperties() override;
        bool updateProperties() override;

    private:
        INDI::PropertySwitch mShutterSwitch {2};
        INDI::PropertyLight  mRainLight {1};
};
