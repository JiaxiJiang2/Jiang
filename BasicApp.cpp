#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h" 
#include "cinder/gl/gl.h"
#include "DMXPro.hpp"

using namespace ci;
using namespace ci::app;

class BasicApp : public App {
public:
    void mouseDrag(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void draw() override;
    void update() override;
    void setup() override;

private:
    std::vector<vec2> mPointsLeft;
    std::vector<vec2> mPointsRight;
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

        mPan = normalizedX * 128.0f;
        mTilt = normalizedY * 128.0f;

        if (mDmxDevice) {
            mDmxDevice->setValue(static_cast<int>(mPan), startAddress + 0); // Pan
            mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2); // Tilt
        }

        mPointsLeft.push_back(currentPos);
    }
    else {
        // Right half: Control color based on line angle
        if (!mPointsRight.empty()) {
            vec2 lastPoint = mPointsRight.back();
            vec2 direction = currentPos - lastPoint;
            float angle = glm::degrees(atan2(-direction.y, direction.x));

            if (angle < 0) {
                angle += 360.0f;
            }

            if (angle >= 0.0f && angle < 90.0f) {
                mCurrentColor = Color(1.0f, 1.0f, 1.0f); // White
                if (mDmxDevice) {
                    mDmxDevice->setValue(0, startAddress + 7);
                    mDmxDevice->setValue(0, startAddress + 8);
                    mDmxDevice->setValue(0, startAddress + 9);
                    mDmxDevice->setValue(70, startAddress + 10);
                }
            }
            else if (angle >= 90.0f && angle < 180.0f) {
                mCurrentColor = Color(0.0f, 0.0f, 1.0f); // Blue
                if (mDmxDevice) {
                    mDmxDevice->setValue(0, startAddress + 7);
                    mDmxDevice->setValue(0, startAddress + 8);
                    mDmxDevice->setValue(70, startAddress + 9);
                    mDmxDevice->setValue(0, startAddress + 10);
                }
            }
            else if (angle >= 180.0f && angle < 270.0f) {
                mCurrentColor = Color(1.0f, 0.0f, 0.0f); // Red
                if (mDmxDevice) {
                    mDmxDevice->setValue(70, startAddress + 7);
                    mDmxDevice->setValue(0, startAddress + 8);
                    mDmxDevice->setValue(0, startAddress + 9);
                    mDmxDevice->setValue(0, startAddress + 10);
                }
            }
            else if (angle >= 270.0f && angle < 360.0f) {
                mCurrentColor = Color(0.0f, 1.0f, 0.0f); // Green
                if (mDmxDevice) {
                    mDmxDevice->setValue(0, startAddress + 7);
                    mDmxDevice->setValue(255, startAddress + 8);
                    mDmxDevice->setValue(0, startAddress + 9);
                    mDmxDevice->setValue(0, startAddress + 10);
                }
            }
        }

        mPointsRight.push_back(currentPos);
    }
}

void BasicApp::keyDown(KeyEvent event)
{
    if (event.getChar() == 'f') {
        setFullScreen(!isFullScreen());
    }
    else if (event.getCode() == KeyEvent::KEY_SPACE) {
        mPointsLeft.clear();
        mPointsRight.clear();
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

    // Draw dividing line
    gl::color(Color::white());
    gl::drawLine(vec2(getWindowWidth() / 2, 0), vec2(getWindowWidth() / 2, getWindowHeight()));

    // Draw lines for left points
    gl::color(Color(0.8f, 0.8f, 0.8f));
    gl::begin(GL_LINE_STRIP);
    for (const vec2& point : mPointsLeft) {
        gl::vertex(point);
    }
    gl::end();

    // Draw lines for right points
    gl::color(mCurrentColor);
    gl::begin(GL_LINE_STRIP);
    for (const vec2& point : mPointsRight) {
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
