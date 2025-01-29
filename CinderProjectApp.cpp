#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "DMXPro.hpp"
#include "WebSocketServer.h"
#include <vector>
#include <string>
#include <iostream>

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public App {
public:
    void setup() override;
    void mouseDrag(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void update() override;
    void draw() override;

private:
    vector<pair<vec2, double>> mPointsWithTime;
    DMXProRef mDmxDevice;
    float mPan = 127.0f;  // Initial pan set to horizontal
    float mTilt = 127.0f; // Initial tilt set to horizontal
    int startAddress = 360;
    Color mCurrentColor = Color(1.0f, 1.0f, 1.0f); // Default white color
    params::InterfaceGlRef mParams;
    shared_ptr<WebSocketServer> mWebSocketServer;

    void setLightColor(const string& color);
    void updateLightDirection(float pan, float tilt);
    void processWebSocketMessage(const string& msg);
};

void CinderProjectApp::setup() {
    console() << "Initializing DMXPro devices..." << endl;

    // Initialize DMXPro device
    DMXPro::listDevices();
    vector<string> devices = DMXPro::getDevicesList();
    if (!devices.empty()) {
        mDmxDevice = DMXPro::create(devices[0]);
        // Set initial light direction to horizontal
        mDmxDevice->setValue(static_cast<int>(mPan), startAddress);
        mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2);
    }

    // Set up GUI parameters for manual control
    mParams = params::InterfaceGl::create("Light Control", ivec2(250, 200));
    mParams->addParam("Pan", &mPan).min(0.0f).max(255.0f).step(1.0f);
    mParams->addParam("Tilt", &mTilt).min(127.0f).max(255.0f).step(1.0f); // Restrict tilt >= 127

    // Start WebSocket server
    mWebSocketServer = make_shared<WebSocketServer>();
    mWebSocketServer->listen(9002);
    console() << "WebSocket server started on port 9002" << endl;

    // Connect message handler for WebSocket
    mWebSocketServer->connectMessageEventHandler([this](const string& msg) {
        console() << "Received WebSocket message: " << msg << endl;
        processWebSocketMessage(msg);
    });

    console() << "Setup complete." << endl;
}

void CinderProjectApp::mouseDrag(MouseEvent event) {
    vec2 currentPos = event.getPos();
    mPointsWithTime.push_back(make_pair(currentPos, getElapsedSeconds()));

    float normalizedX = static_cast<float>(currentPos.x) / getWindowWidth();
    float normalizedY = static_cast<float>(currentPos.y) / getWindowHeight();

    mPan = normalizedX * 255.0f;
    mTilt = max(normalizedY * 255.0f, 127.0f); // Ensure tilt >= 127

    if (mDmxDevice) {
        mDmxDevice->setValue(static_cast<int>(mPan), startAddress);
        mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2);
    }
}

void CinderProjectApp::keyDown(KeyEvent event) {
    if (event.getChar() == 'f') {
        setFullScreen(!isFullScreen());
    } else if (event.getCode() == KeyEvent::KEY_ESCAPE) {
        quit();
    }
}

void CinderProjectApp::update() {
    if (mWebSocketServer) {
        mWebSocketServer->poll();
    }
}

void CinderProjectApp::draw() {
    gl::clear(Color(0, 0, 0));
    gl::color(mCurrentColor);

    gl::begin(GL_LINE_STRIP);
    for (const auto& point : mPointsWithTime) {
        gl::vertex(point.first);
    }
    gl::end();

    mParams->draw();
}

// Updates light color based on the received message
void CinderProjectApp::setLightColor(const string& color) {
    if (mDmxDevice) {
        if (color == "red") {
            mDmxDevice->setValue(255, startAddress + 1);  // Red channel
            mDmxDevice->setValue(0, startAddress + 2);
            mDmxDevice->setValue(0, startAddress + 3);
            mCurrentColor = Color(1, 0, 0);
        } else if (color == "green") {
            mDmxDevice->setValue(0, startAddress + 1);
            mDmxDevice->setValue(255, startAddress + 2);  // Green channel
            mDmxDevice->setValue(0, startAddress + 3);
            mCurrentColor = Color(0, 1, 0);
        } else if (color == "blue") {
            mDmxDevice->setValue(0, startAddress + 1);
            mDmxDevice->setValue(0, startAddress + 2);
            mDmxDevice->setValue(255, startAddress + 3);  // Blue channel
            mCurrentColor = Color(0, 0, 1);
        } else if (color == "white") {
            mDmxDevice->setValue(255, startAddress + 1);
            mDmxDevice->setValue(255, startAddress + 2);
            mDmxDevice->setValue(255, startAddress + 3);  // White light
            mCurrentColor = Color(1, 1, 1);
        } else {
            console() << "Unknown color command: " << color << endl;
        }
        console() << "Light color set to: " << color << endl;
    }
}

// Updates light direction (Pan/Tilt) based on hand tracking
void CinderProjectApp::updateLightDirection(float pan, float tilt) {
    mPan = pan;
    mTilt = max(tilt, 127.0f);  // Ensure tilt >= 127

    if (mDmxDevice) {
        mDmxDevice->setValue(static_cast<int>(mPan), startAddress);
        mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2);
    }

    console() << "Updated light direction: Pan=" << mPan << ", Tilt=" << mTilt << endl;
}

// Processes WebSocket messages and performs actions based on the message type
void CinderProjectApp::processWebSocketMessage(const string& msg) {
    if (msg.find("color_change") != string::npos) {
        string color;
        sscanf(msg.c_str(), "{\"type\":\"color_change\",\"color\":\"%s\"}", color.data());
        setLightColor(color);
    } else if (msg.find("light_control") != string::npos) {
        float pan, tilt;
        sscanf(msg.c_str(), "{\"type\":\"light_control\",\"pan\":%f,\"tilt\":%f}", &pan, &tilt);
        updateLightDirection(pan, tilt);
    } else {
        console() << "Invalid WebSocket message received: " << msg << endl;
    }
}

CINDER_APP(CinderProjectApp, RendererGl)
