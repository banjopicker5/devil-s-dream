/*
    USB Focus V3
    Copyright (C) 2016 G. Schmidt
    Copyright (C) 2018-2023 Jarno Paananen

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

#include "indifocuser.h"

/***************************** USB Focus V3 Commands **************************/

#define UFOCREADPARAM "SGETAL"
#define UFOCDEVID "SWHOIS"
#define UFOCREADPOS "FPOSRO"
#define UFOCREADTEMP "FTMPRO"
#define UFOCMOVEOUT "O%05u"
#define UFOCMOVEIN "I%05u"
#define UFOCABORT "FQUITx"
#define UFOCSETMAX "M%05u"
#define UFOCSETSPEED "SMO%03u"
#define UFOCSETTCTHR "SMA%03u"
#define UFOCSETSDIR "SMROTH"
#define UFOCSETRDIR "SMROTT"
#define UFOCSETFSTEPS "SMSTPF"
#define UFOCSETHSTEPS "SMSTPD"
#define UFOCSETSTDEG "FLA%03u"
#define UFOCGETSIGN "FTAXXA"
#define UFOCSETSIGN "FZAXX%1u"
#define UFOCSETAUTO "FAMODE"
#define UFOCSETMANU "FMMODE"
#define UFOCRESET "SEERAZ"

/**************************** USB Focus V3 Constants **************************/

#define UFOID "UFO"

#define UFORSACK '*'
#define UFORSAUTO "AP"
#define UFORSDONE "DONE"
#define UFORSERR "ER=1"
#define UFORSRESET "EEPROM RESET"

#define UFOPSDIR 0   // standard direction
#define UFOPRDIR 1   // reverse direction
#define UFOPFSTEPS 0 // full steps
#define UFOPHSTEPS 1 // half steps
#define UFOPPSIGN 0  // positive temp. comp. sign
#define UFOPNSIGN 1  // negative temp. comp. sign

#define UFOPSPDERR 0 // invalid speed
#define UFOPSPDAV 2  // average speed
#define UFOPSPDSL 3  // slow speed
#define UFOPSPDUS 4  // ultra slow speed

#define UFORESLEN 32 // maximum length of returned response with some margin (28 characters is actual maximum)
#define UFOCMDLEN 6  // length of all commands

/******************************************************************************/

class USBFocusV3 : public INDI::Focuser
{
    public:
        USBFocusV3();
        virtual ~USBFocusV3() = default;

        typedef enum
        {
            FOCUS_HALF_STEP,
            FOCUS_FULL_STEP
        } FocusStepMode;

        virtual bool Handshake() override;
        bool getControllerStatus();
        virtual const char *getDefaultName() override;
        virtual bool initProperties() override;
        virtual bool updateProperties() override;
        virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
        virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
        virtual IPState MoveAbsFocuser(uint32_t targetTicks) override;
        virtual IPState MoveRelFocuser(FocusDirection dir, uint32_t ticks) override;
        virtual bool SetFocuserSpeed(int speed) override;
        virtual bool AbortFocuser() override;
        virtual void TimerHit() override;
        //virtual bool saveConfigItems(FILE *fp) override;

        virtual bool SetFocuserBacklash(int32_t steps) override;

    private:
        bool sendCommand(const char *cmd, char *response);
        bool sendCommandSpecial(const char *cmd, char *response);

        pthread_mutex_t cmdlock;

        void GetFocusParams();
        bool reset();
        bool updateStepMode();
        bool updateRotDir();
        bool updateTemperature();
        bool updatePosition();
        bool updateMaxPos();
        bool updateTempCompSettings();
        bool updateTempCompSign();
        bool updateSpeed();
        bool updateFWversion();

        bool Ack();
        bool Resync();

        bool MoveFocuserUF(FocusDirection dir, unsigned int rticks);
        bool setStepMode(FocusStepMode mode);
        bool setRotDir(unsigned int dir);
        bool setMaxPos(unsigned int maxp);
        bool setSpeed(unsigned short drvspeed);
        bool setAutoTempCompThreshold(unsigned int thr);
        bool setTemperatureCoefficient(unsigned int coefficient);
        bool setTempCompSign(unsigned int sign);
        bool setTemperatureCompensation(bool enable);
        float CalcTimeLeft(timeval, float);

        unsigned int direction{ 0 }; // 0 standard, 1 reverse
        unsigned int stepmode{ 0 };  // 0 full steps, 1 half steps
        unsigned int speed{ 0 };     // 2 average, 3 slow, 4 ultra slow
        unsigned int stepsdeg{ 0 };  // steps per degree for temperature compensation
        unsigned int tcomp_thr{ 0 }; // temperature compensation threshold
        unsigned int firmware{ 0 };  // firmware version
        unsigned int maxpos{ 0 };    // maximum step position (0..65535)

        double targetPos{ 0 };
        double backlashTargetPos{ 0 };
        double lastPos{ 0 };
        int backlashSteps{ 0 };
        bool backlashIn{ false };
        bool backlashMove{ false };
        bool moving{ false };

        double lastTemperature{ 0 };
        unsigned int currentSpeed{ 0 };

        struct timeval focusMoveStart
        {
            0, 0
        };
        float focusMoveRequest{ 0 };

        INDI::PropertyNumber TemperatureNP{1};
        INDI::PropertySwitch StepModeSP{2};
        INDI::PropertySwitch RotDirSP{2};
        INDI::PropertyNumber MaxPositionNP{1};
        INDI::PropertyNumber TemperatureSettingNP{2};
        INDI::PropertySwitch TempCompSignSP{2};
        INDI::PropertySwitch TemperatureCompensateSP{2};
        INDI::PropertySwitch ResetSP{1};
        INDI::PropertyNumber FWversionNP{1};
};
