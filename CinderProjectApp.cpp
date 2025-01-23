#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "DMXPro.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public App {
public:
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void update() override;
    void draw() override;

private:
    vector<vec2> mPointsLeft;
    vector<vec2> mPointsRight;
    DMXProRef mDmxDevice;
    float mPan = 0.0f;
    float mTilt = 0.0f;
    int startAddress = 360;
    Color mCurrentColor = Color(1.0f, 1.0f, 1.0f);
    params::InterfaceGlRef mParams;
};

void CinderProjectApp::setup() {
    DMXPro::listDevices();
    vector<string> devices = DMXPro::getDevicesList();
    if (!devices.empty()) {
        mDmxDevice = DMXPro::create(devices[0]);
        mDmxDevice->setValue(5, startAddress + 5);
    }

    mParams = params::InterfaceGl::create("Light Control", ivec2(200, 150));
    mParams->addParam("Pan Value", &mPan).min(0.0f).max(255.0f).step(1.0f).updateFn([this]() {
        if (mDmxDevice) {
            mDmxDevice->setValue(static_cast<int>(mPan), startAddress + 0);
        }
        });

    mParams->addParam("Tilt Value", &mTilt).min(0.0f).max(255.0f).step(1.0f).updateFn([this]() {
        if (mDmxDevice) {
            mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2);
        }
        });
}

void CinderProjectApp::mouseDown(MouseEvent event) {
    vec2 pos = event.getPos();
    console() << "Mouse down at: " << pos << endl;

    if (pos.x < getWindowWidth() / 2) {
        mPointsLeft.push_back(pos);
    }
    else {
        mPointsRight.push_back(pos);
    }
}

void CinderProjectApp::mouseDrag(MouseEvent event) {
    vec2 currentPos = event.getPos();
    float windowWidth = getWindowWidth();

    if (currentPos.x < windowWidth / 2) {
        float normalizedX = static_cast<float>(currentPos.x) / (windowWidth / 2);
        float normalizedY = static_cast<float>(currentPos.y) / getWindowHeight();

        mPan = normalizedX * 128.0f;
        mTilt = normalizedY * 128.0f;

        if (mDmxDevice) {
            mDmxDevice->setValue(static_cast<int>(mPan), startAddress + 0);
            mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2);
        }

        mPointsLeft.push_back(currentPos);
    }
    else {
        mPointsRight.push_back(currentPos);
    }
}

void CinderProjectApp::keyDown(KeyEvent event) {
    if (event.getChar() == 'f') {
        setFullScreen(!isFullScreen());
    }
    else if (event.getCode() == KeyEvent::KEY_SPACE) {
        mPointsLeft.clear();
        mPointsRight.clear();
    }
    else if (event.getCode() == KeyEvent::KEY_ESCAPE) {
        if (isFullScreen()) setFullScreen(false);
        else quit();
    }
}

void CinderProjectApp::update() {
}

void CinderProjectApp::draw() {
    gl::clear(Color(0, 0, 0));
    gl::color(Color::white());
    gl::drawLine(vec2(getWindowWidth() / 2, 0), vec2(getWindowWidth() / 2, getWindowHeight()));

    gl::color(mCurrentColor);
    gl::begin(GL_LINE_STRIP);
    for (const vec2& point : mPointsRight) {
        gl::vertex(point);
    }
    gl::end();

    mParams->draw();
}

CINDER_APP(CinderProjectApp, RendererGl)
