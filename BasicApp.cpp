#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h" 
#include "cinder/gl/gl.h"
#include "DMXPro.hpp"

using namespace ci;
using namespace ci::app;

// We'll create a new Cinder Application by deriving from the App class.
class BasicApp : public App {
public:
    void mouseDrag(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void draw() override;
    void update() override;
    void setup() override;

private:
    std::vector<vec2> mPoints;
    DMXProRef mDmxDevice;
    float mPan = 0.0f;
    float mTilt = 0.0f;
    int startAddress = 360;
    Color mCurrentColor = Color(1.0f, 1.0f, 1.0f); // Default white color
};

void prepareSettings(BasicApp::Settings* settings)
{
    settings->setMultiTouchEnabled(false);
}

void BasicApp::mouseDrag(MouseEvent event)
{
    vec2 currentPos = event.getPos();
    float windowWidth = getWindowWidth();
    float windowHeight = getWindowHeight();

    if (currentPos.x < windowWidth / 2) {
        // Left half: Control direction (Pan and Tilt)
        float normalizedX = static_cast<float>(currentPos.x) / (windowWidth / 2);
        float normalizedY = static_cast<float>(currentPos.y) / windowHeight;

        mPan = normalizedX * 255.0f;
        mTilt = normalizedY * 255.0f;

        if (mDmxDevice) {
            mDmxDevice->setValue(static_cast<int>(mPan), startAddress + 0); // Pan
            mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2); // Tilt
        }
    }
    else {
        // Right half: Control color
        float normalizedY = static_cast<float>(currentPos.y) / windowHeight;

        if (normalizedY < 0.25f) {
            mCurrentColor = Color(1.0f, 0.0f, 0.0f); // Red
            if (mDmxDevice) {
                mDmxDevice->setValue(70, startAddress + 7); // R
                mDmxDevice->setValue(0, startAddress + 8);   // G
                mDmxDevice->setValue(0, startAddress + 9);   // B
                mDmxDevice->setValue(0, startAddress + 10);  // W
            }
        }
        else if (normalizedY < 0.5f) {
            mCurrentColor = Color(0.0f, 1.0f, 0.0f); // Green
            if (mDmxDevice) {
                mDmxDevice->setValue(0, startAddress + 7);
                mDmxDevice->setValue(255, startAddress + 8);
                mDmxDevice->setValue(0, startAddress + 9);
                mDmxDevice->setValue(0, startAddress + 10);
            }
        }
        else if (normalizedY < 0.75f) {
            mCurrentColor = Color(0.0f, 0.0f, 1.0f); // Blue
            if (mDmxDevice) {
                mDmxDevice->setValue(0, startAddress + 7);
                mDmxDevice->setValue(0, startAddress + 8);
                mDmxDevice->setValue(70, startAddress + 9);
                mDmxDevice->setValue(0, startAddress + 10);
            }
        }
        else {
            mCurrentColor = Color(1.0f, 1.0f, 1.0f); // White
            if (mDmxDevice) {
                mDmxDevice->setValue(0, startAddress + 7);
                mDmxDevice->setValue(0, startAddress + 8);
                mDmxDevice->setValue(0, startAddress + 9);
                mDmxDevice->setValue(70, startAddress + 10);
            }
        }
    }

    mPoints.push_back(currentPos);
}

void BasicApp::keyDown(KeyEvent event)
{
    if (event.getChar() == 'f') {
        setFullScreen(!isFullScreen());
    }
    else if (event.getCode() == KeyEvent::KEY_SPACE) {
        mPoints.clear();
    }
    else if (event.getCode() == KeyEvent::KEY_ESCAPE) {
        if (isFullScreen())
            setFullScreen(false);
        else
            quit();
    }
}

void BasicApp::draw()
{
    gl::clear(Color::gray(0.1f));
    gl::color(mCurrentColor);

    gl::begin(GL_LINE_STRIP);
    for (const vec2& point : mPoints) {
        gl::vertex(point);
    }
    gl::end();
}

void BasicApp::update()
{
    // Optional: Add any continuous updates if required
}

void BasicApp::setup()
{
    DMXPro::listDevices();
    std::vector<std::string> devices = DMXPro::getDevicesList();
    if (!devices.empty()) {
        mDmxDevice = DMXPro::create(devices[0]);
        mDmxDevice->setValue(5, startAddress + 5);
        mDmxDevice->setValue(70, startAddress + 10); // Initialize color
    }
}

CINDER_APP(BasicApp, RendererGl, prepareSettings)
