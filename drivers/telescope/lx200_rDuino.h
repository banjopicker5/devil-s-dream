#pragma once

#include "indiguiderinterface.h"
#include "inditelescope.h"
#include "lx200driver.h"
#include "indicom.h"

#define RB_MAX_LEN 64
#define INITIAL_GUIDE_RATE 0.50

/* RA DEC Motor Select */
#define devOn(fd,x)                      setCommandInt(fd,x, ":Dn")
#define devOff(fd,x)                     setCommandInt(fd,x, ":Df")

class LX200_rDuino : public INDI::Telescope, public INDI::GuiderInterface
{
    public:
        LX200_rDuino();
        ~LX200_rDuino() override = default;

        virtual const char *getDefaultName() override;
        virtual const char *getDriverName() override;
        virtual bool Handshake() override;
        virtual bool ReadScopeStatus() override;
        virtual void ISGetProperties(const char *dev) override;
        virtual bool initProperties() override;
        virtual bool updateProperties() override;

        virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
        virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
        virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;

    protected:
        virtual bool MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command) override;
        virtual bool MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command) override;
        virtual bool Abort() override;
        virtual bool Goto(double, double) override;
        virtual bool Sync(double ra, double dec) override;
        virtual void getBasicData();   // Initial function to get data after connection is successful
        virtual bool Park() override;
        virtual bool UnPark() override;
        virtual bool SetCurrentPark() override;
        virtual bool SetTrackEnabled(bool enabled) override;
        virtual bool updateLocation(double latitude, double longitude, double elevation) override;
        virtual void debugTriggered(bool enable) override;
        virtual bool SetTrackMode(uint8_t mode) override;
        //GUIDE: guiding functions
        virtual IPState GuideNorth(uint32_t ms) override;
        virtual IPState GuideSouth(uint32_t ms) override;
        virtual IPState GuideEast(uint32_t ms) override;
        virtual IPState GuideWest(uint32_t ms) override;
        virtual bool saveConfigItems(FILE *fp) override;


    private:
        bool Move(TDirection dir, TelescopeMotionCommand command);
        bool selectSlewRate(int index);
        bool isSlewComplete();
        void slewError(int slewCode);
        void mountSim();
        bool SetGuideRate(float);

        bool getLocalDate(char *dateString);
        bool setLocalDate(uint8_t days, uint8_t months, uint16_t years);

        bool getSiteIndex(int *ndxP);
        bool getSlewRate(int *srP);
        bool setSite(int ndx);       // used instead of selectSite from lx200 driver
        bool getSiteElevation(int *elevationP);
        bool setSiteElevation(double elevation);
        bool getLocation(void);     // read sites from rDuino

        // Get Local time in 24 hour format from mount. Expected format is HH:MM:SS
        bool getLocalTime(char *timeString);
        bool setLocalTime24(uint8_t hour, uint8_t minute, uint8_t second);

        // Return UTC Offset from mount in hours.
        bool setUTCOffset(double offset);
        bool getUTFOffset(double * offset);

        void handleStatusChange(void);
        void SendPulseCmd(int8_t direction, uint32_t duration_msec);
        void sendCommand(const char *cmd);
        void updateMountStatus(char);
        void updateSlewRate(void);

        // Send Mount time and location settings to client
        bool sendScopeTime();
        bool sendScopeLocation();

        // User interface

        INumber SlewAccuracyN[2];
        INumberVectorProperty SlewAccuracyNP;

        ISwitchVectorProperty HomePauseSP;
        ISwitch HomePauseS[3];

        ISwitchVectorProperty SetHomeSP;
        ISwitch SetHomeS[2];

        ITextVectorProperty VersionTP;
        IText VersionT[5] {};

        ISwitch SlewRateS[5];
        ISwitchVectorProperty SlewRateSP;

        INumber GuideRateN[1];
        INumberVectorProperty GuideRateNP;

        ISwitch TATrackModeS[3];
        ISwitchVectorProperty TATrackModeSP;

        // Site Management
        ISwitchVectorProperty SiteSP;
        ISwitch SiteS[4];
        int currentSiteNum {0}; // on rDuino, sites are numbered 0 to 3, not 1 to 4 like on the Meade standard

        // Site Name
        ITextVectorProperty SiteNameTP;
        IText SiteNameT[1] {};

        // Error Status
        ITextVectorProperty ErrorStatusTP;
        IText ErrorStatusT[1] {};

        // devOn 
        ISwitch SetDevS[2];
        ISwitchVectorProperty SetDevSP;
        int currentDevNum; //Dev1


        double targetRA = 0, targetDEC = 0;
        double currentRA = 0, currentDEC = 0;
        uint32_t DBG_SCOPE = 0;
        char OSStat[RB_MAX_LEN];
        char OldOSStat[RB_MAX_LEN];
        const char *statusCommand;           // :GU# for version 1.1, :GXI# for 1.2 and later
        const char *guideSpeedCommand;       // :SXR0

        const char *MOTOR_SELECT_TAB    = "Motor Select"; //Motor Select


};
