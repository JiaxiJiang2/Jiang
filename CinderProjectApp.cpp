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
    float mPan = 127.0f;  // Initial pan (horizontal)
    float mTilt = 127.0f; // Initial tilt (upwards)
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

        //  Force light ON at startup 
        mDmxDevice->setValue(70, startAddress + 10); // Master Dimmer (Brightness ON)
        mDmxDevice->setValue(70, startAddress + 7); // Red Channel (White Light)
        mDmxDevice->setValue(70, startAddress + 8); // Green Channel (White Light)
        mDmxDevice->setValue(70, startAddress + 9); // Blue Channel (White Light)

        console() << "DMX Light Forced ON at Startup (White Light)" << endl;
    }

    // GUI Controls
    mParams = params::InterfaceGl::create("Light Control", ivec2(250, 200));
    mParams->addParam("Pan", &mPan).min(0.0f).max(255.0f).step(1.0f);
    mParams->addParam("Tilt", &mTilt).min(0.0f).max(255.0f).step(1.0f);

    // WebSocket Server Setup
    mWebSocketServer = make_shared<WebSocketServer>();
    mWebSocketServer->listen(9002);
    console() << "WebSocket server started on port 9002" << endl;

    // Handle WebSocket messages
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
    mTilt = normalizedY * 255.0f;

    if (mDmxDevice) {
        mDmxDevice->setValue(static_cast<int>(mPan), startAddress);
        mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2);
    }
}

void CinderProjectApp::keyDown(KeyEvent event) {
    if (event.getChar() == 'f') {
        setFullScreen(!isFullScreen());
    }
    else if (event.getCode() == KeyEvent::KEY_ESCAPE) {
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

// Updates the DMX light color
void CinderProjectApp::setLightColor(const string& color) {
    if (mDmxDevice) {

        mDmxDevice->setValue(255, startAddress + 10);

        if (color == "red") {
            mDmxDevice->setValue(255, startAddress + 7);  // Red Channel
            mDmxDevice->setValue(0, startAddress + 8);
            mDmxDevice->setValue(0, startAddress + 9);
            mDmxDevice->setValue(0, startAddress + 10);
        }
        else if (color == "green") {
            mDmxDevice->setValue(0, startAddress + 7);
            mDmxDevice->setValue(255, startAddress + 8);  // Green Channel
            mDmxDevice->setValue(0, startAddress + 9);
            mDmxDevice->setValue(0, startAddress + 10);
        }
        else if (color == "blue") {
            mDmxDevice->setValue(0, startAddress + 7);
            mDmxDevice->setValue(0, startAddress + 8);
            mDmxDevice->setValue(255, startAddress + 9);  // Blue Channel
            mDmxDevice->setValue(0, startAddress + 10);
        }
        else if (color == "white") {
            mDmxDevice->setValue(0, startAddress + 7);
            mDmxDevice->setValue(0, startAddress + 8);
            mDmxDevice->setValue(0, startAddress + 9);
            mDmxDevice->setValue(255, startAddress + 10); // White Channel
        }
        else {
            console() << "Unknown color command: " << color << endl;
        }
        console() << "Light color set to: " << color << endl;
    }
}


// Handles WebSocket messages
void CinderProjectApp::processWebSocketMessage(const string& msg) {
    console() << "Processing WebSocket message: " << msg << endl;

    if (msg.find("\"type\":\"color_change\"") != string::npos) {
        size_t startPos = msg.find("\"color\":\"");
        if (startPos != string::npos) {
            startPos += 9;
            size_t endPos = msg.find("\"", startPos);
            if (endPos != string::npos) {
                string color = msg.substr(startPos, endPos - startPos);
                console() << "Extracted Color: " << color << endl;
                setLightColor(color);
                return;
            }
        }
    }
    else if (msg.find("\"type\":\"light_control\"") != string::npos) {
        size_t panPos = msg.find("\"pan\":");
        size_t tiltPos = msg.find("\"tilt\":");

        if (panPos != string::npos && tiltPos != string::npos) {
            panPos += 6;
            tiltPos += 7;

            float pan = stof(msg.substr(panPos, msg.find(",", panPos) - panPos));
            float tilt = stof(msg.substr(tiltPos, msg.find("}", tiltPos) - tiltPos));

            console() << "Parsed Pan: " << pan << ", Tilt: " << tilt << endl;
            updateLightDirection(pan, tilt);
            return;
        }
    }

    console() << "Invalid WebSocket message received: " << msg << endl;
}


// Updates light direction based on WebSocket data
void CinderProjectApp::updateLightDirection(float pan, float tilt) {
    mPan = pan;
    mTilt = tilt;

    if (mDmxDevice) {
        mDmxDevice->setValue(static_cast<int>(mPan), startAddress);
        mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2);
    }

    console() << "Updated light direction: Pan=" << mPan << ", Tilt=" << mTilt << endl;
}

CINDER_APP(CinderProjectApp, RendererGl)
