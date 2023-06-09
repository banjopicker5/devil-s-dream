#include "indicom.h"
#include "indilogger.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::StrEq;

#include "ccd_simulator.h"

char _me[] = "MockCCDSimDriver";
char *me = _me;
class MockCCDSimDriver: public CCDSim
{
    public:
        MockCCDSimDriver(): CCDSim()
        {
            initProperties();
            ISGetProperties(me);
        }

        void testProperties()
        {
            auto p = getNumber("SIMULATOR_SETTINGS");
            ASSERT_NE(p, nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_XRES"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_YRES"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_XSIZE"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_YSIZE"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_MAXVAL"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_SATURATION"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_LIMITINGMAG"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_NOISE"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_SKYGLOW"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_OAGOFFSET"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_POLAR"), nullptr);
            ASSERT_NE(p.findWidgetByName("SIM_POLARDRIFT"), nullptr);
        }

        void testGuideAPI()
        {
            // At init, current RA and DEC are undefined - message will not appear because the test passes
            EXPECT_TRUE(isnan(currentRA)) << "Field 'currentRA' is undefined when initializing CCDSim.";
            EXPECT_TRUE(isnan(currentDE)) << "Field 'currentDEC' is undefined when initializing CCDSim.";

            // Guide rate is fixed
            EXPECT_EQ(GuideRate, 7 /* arcsec/s */);

            // Initial guide offsets are zero
            EXPECT_EQ(guideNSOffset, 0);
            EXPECT_EQ(guideWEOffset, 0);

            double const arcsec = 1.0 / 3600.0;

            // Guiding in DEC stores offset in arcsec for next simulation step
            EXPECT_EQ(GuideNorth(1000.0), IPS_OK);
            EXPECT_NEAR(guideNSOffset, +7 * arcsec, 1 * arcsec);
            EXPECT_EQ(GuideSouth(1000.0), IPS_OK);
            EXPECT_NEAR(guideNSOffset, +0 * arcsec, 1 * arcsec);
            EXPECT_EQ(GuideSouth(1000.0), IPS_OK);
            EXPECT_NEAR(guideNSOffset, -7 * arcsec, 1 * arcsec);
            EXPECT_EQ(GuideNorth(1000.0), IPS_OK);
            EXPECT_NEAR(guideNSOffset, +0 * arcsec, 1 * arcsec);

            // RA guiding rate depends on declination, we need a valid one
            currentDE = 0;

            // Guiding in RA stores offset in arcsec for next simulation step
            // There is an adjustemnt based on declination - here zero from previous test
            EXPECT_EQ(GuideWest(1000.0), IPS_OK);
            EXPECT_NEAR(guideWEOffset, +7 * arcsec, 15 * arcsec);
            EXPECT_EQ(GuideEast(1000.0), IPS_OK);
            EXPECT_NEAR(guideWEOffset, +0 * arcsec, 15 * arcsec);
            EXPECT_EQ(GuideEast(1000.0), IPS_OK);
            EXPECT_NEAR(guideWEOffset, -7 * arcsec, 15 * arcsec);
            EXPECT_EQ(GuideWest(1000.0), IPS_OK);
            EXPECT_NEAR(guideWEOffset, +0 * arcsec, 15 * arcsec);

            // TODO: verify DEC-biased RA guiding rate
            // TODO: verify property-based guiding API
        }

        void testDrawStar()
        {
            int const xres = 65;
            int const yres = 65;
            int const maxval = pow(2, 8) - 1;

            // Setup a 65x65, 16-bit depth, 4.6u square pixel sensor
            auto p = getNumber("SIMULATOR_SETTINGS");
            ASSERT_NE(p, nullptr);
            p.findWidgetByName("SIM_XRES")->setValue((double) xres);
            p.findWidgetByName("SIM_YRES")->setValue((double) yres);
            // There is no way to set depth, it is hardcoded at 16-bit - so set maximum value instead
            p.findWidgetByName("SIM_MAXVAL")->setValue((double) maxval);
            p.findWidgetByName("SIM_XSIZE")->setValue(4.6);
            p.findWidgetByName("SIM_YSIZE")->setValue(4.6);

            // Setup a saturation magnitude (max ADUs in one second) and limit magnitude (zero ADU whatever the exposure)
            p.findWidgetByName("SIM_SATURATION")->setValue(0.0);
            p.findWidgetByName("SIM_LIMITINGMAG")->setValue(30.0);

            // Setup some parameters to simplify verifications
            p.findWidgetByName("SIM_SKYGLOW")->setValue(0.0);
            p.findWidgetByName("SIM_NOISE")->setValue(0.0);
            this->seeing = 1.0f; // No way to control seeing from properties

            // Setup
            ASSERT_TRUE(setupParameters());

            // Assert our parameters
            ASSERT_EQ(PrimaryCCD.getBPP(), 16) << "Simulator CCD depth is hardcoded at 16 bits";
            ASSERT_EQ(PrimaryCCD.getXRes(), xres);
            ASSERT_EQ(PrimaryCCD.getYRes(), yres);
            ASSERT_EQ(PrimaryCCD.getPixelSizeX(), 4.6f);
            ASSERT_EQ(PrimaryCCD.getPixelSizeY(), 4.6f);
            ASSERT_NE(PrimaryCCD.getFrameBuffer(), nullptr) << "SetupParms creates the frame buffer";

            // Assert our simplifications
            EXPECT_EQ(this->seeing, 1.0f);
            EXPECT_EQ(this->ImageScalex, 1.0f);
            EXPECT_EQ(this->ImageScaley, 1.0f);
            EXPECT_EQ(this->m_SkyGlow, 0.0f);
            EXPECT_EQ(this->m_MaxNoise, 0.0f);

            // Validate our expectations about flux
            EXPECT_EQ(this->m_MaxVal, maxval);
            EXPECT_NEAR(this->flux(this->m_SaturationMag), maxval, 0.001);
            EXPECT_NEAR(this->flux(this->m_LimitingMag), 1.0, 0.001);

            // The CCD frame is NOT initialized after this call, so manually clear the buffer
            memset(this->PrimaryCCD.getFrameBuffer(), 0, this->PrimaryCCD.getFrameBufferSize());

            // Draw a star at the center row/column of the sensor
            // If we expose a magnitude of 0 for 1 second, we get max ADUs at center, gaussian decrement away by 4 pixels and zero elsewhere
            DrawImageStar(&PrimaryCCD, 0.0f, xres / 2 + 1, xres / 2 + 1, 1.0f);

            // Get a pointer to the 16-bit frame buffer
            uint16_t const * const fb = reinterpret_cast<uint16_t*>(PrimaryCCD.getFrameBuffer());

            // Look at center, and up to 3 pixels away, and find activated photosites there - there is no skyglow nor noise in our parameters
            int const center = xres / 2 + 1 + (yres / 2 + 1) * xres;

            // The choice of the gaussian of unitary integral makes the center less than maximum
            double const sigma = 1.0 / (2 * sqrt(2 * log(2)));
            double const fa0 = 1.0 / (sigma * sqrt(2 * 3.1416));

            // Center photosite
            uint16_t const ADU_at_center = static_cast<uint16_t>(fa0 * maxval);
            EXPECT_EQ(fb[center], ADU_at_center) << "Recorded flux of magnitude 0.0 for 1 second at center is " << ADU_at_center <<
                                                 " ADUs";

            // Up, left, right and bottom photosites at one pixel
            uint16_t const ADU_at_1pix = static_cast<uint16_t>(fa0 * maxval * exp(-(1 * 1 + 0 * 0) / (2 * sigma * sigma)));
            EXPECT_EQ(fb[center - xres], ADU_at_1pix);
            EXPECT_EQ(fb[center - 1], ADU_at_1pix);
            EXPECT_EQ(fb[center + 1], ADU_at_1pix);
            EXPECT_EQ(fb[center + xres], ADU_at_1pix);

            // Up, left, right and bottom photosites at two pixels
            uint16_t const ADU_at_2pix = static_cast<uint16_t>(fa0 * maxval * exp(-(2 * 2 + 0 * 0) / (2 * sigma * sigma)));
            EXPECT_EQ(fb[center - xres * 2], ADU_at_2pix);
            EXPECT_EQ(fb[center - 1 * 2], ADU_at_2pix);
            EXPECT_EQ(fb[center + 1 * 2], ADU_at_2pix);
            EXPECT_EQ(fb[center + xres * 2], ADU_at_2pix);

            // Up, left, right and bottom photosite neighbors at three pixels
            uint16_t const ADU_at_3pix = static_cast<uint16_t>(fa0 * maxval * exp(-(3 * 3 + 0 * 0) / (2 * sigma * sigma)));
            EXPECT_EQ(fb[center - xres * 3], ADU_at_3pix);
            EXPECT_EQ(fb[center - 1 * 3], ADU_at_3pix);
            EXPECT_EQ(fb[center + 1 * 3], ADU_at_3pix);
            EXPECT_EQ(fb[center + xres * 3], ADU_at_3pix);

            // Up, left, right and bottom photosite neighbors at four pixels
            EXPECT_EQ(fb[center - xres * 4], 0.0);
            EXPECT_EQ(fb[center - 1 * 4], 0.0);
            EXPECT_EQ(fb[center + 1 * 4], 0.0);
            EXPECT_EQ(fb[center + xres * 4], 0.0);

            // Conclude with a random benchmark
            auto const before = std::chrono::steady_clock::now();
            int const loops = 200000;
            for (int i = 0; i < loops; i++)
            {
                float const m = (15.0f * rand()) / RAND_MAX;
                float const x = static_cast<float>(xres * rand()) / RAND_MAX;
                float const y = static_cast<float>(yres * rand()) / RAND_MAX;
                float const e = (100.0f * rand()) / RAND_MAX;
                DrawImageStar(&PrimaryCCD, m, x, y, e);
            }
            auto const after = std::chrono::steady_clock::now();
            auto const duration = std::chrono::duration_cast <std::chrono::nanoseconds> (after - before).count() / loops;
            std::cout << "[          ] DrawStarImage - randomized no-noise no-skyglow benchmark: " << duration << "ns per call" <<
                      std::endl;
        }
};

TEST(CCDSimulatorDriverTest, test_properties)
{
    MockCCDSimDriver().testProperties();
}

TEST(CCDSimulatorDriverTest, test_guide_api)
{
    MockCCDSimDriver().testGuideAPI();
}

TEST(CCDSimulatorDriverTest, test_draw_star)
{
    MockCCDSimDriver().testDrawStar();
}

int main(int argc, char **argv)
{
    INDI::Logger::getInstance().configure("", INDI::Logger::file_off,
                                          INDI::Logger::DBG_ERROR, INDI::Logger::DBG_ERROR);

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
