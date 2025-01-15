#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h" // 引入Cinder参数窗口的支持
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
    float mPan = 0.0f; //  Pan 值
    float mTilt = 0.0f; // 当前 Tilt 值
    float mPanOffset = 0.0f; // 相对 Pan 偏移量
    float mTiltOffset = 0.0f; // 相对 Tilt 偏移量
    int startAddress = 360;
    Color mCurrentColor = Color(1.0f, 1.0f, 1.0f); // Default white color

    // 参数窗口
    params::InterfaceGlRef mParams;
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

void BasicApp::setup() {
    // 初始化 DMX 设备
    DMXPro::listDevices();
    std::vector<std::string> devices = DMXPro::getDevicesList();
    if (!devices.empty()) {
        mDmxDevice = DMXPro::create(devices[0]);
        mDmxDevice->setValue(5, startAddress + 5);
    }

    // 创建参数窗口
    mParams = params::InterfaceGl::create("Light Control", ivec2(200, 150));

    // 新增 Pan 和 Tilt 数值输入框
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



void BasicApp::update() {
    // 可以在这里添加需要持续更新的代码
}

void BasicApp::draw() {
    gl::clear(Color::gray(0.1f)); // 清空背景

    // 绘制分割线
    gl::color(Color::white());
    gl::drawLine(vec2(getWindowWidth() / 2, 0), vec2(getWindowWidth() / 2, getWindowHeight()));

    // 绘制左侧点的轨迹
    gl::color(Color(0.8f, 0.8f, 0.8f));
    gl::begin(GL_LINE_STRIP);
    for (const vec2& point : mPointsLeft) {
        gl::vertex(point);
    }
    gl::end();

    // 绘制右侧点的轨迹
    gl::color(mCurrentColor);
    gl::begin(GL_LINE_STRIP);
    for (const vec2& point : mPointsRight) {
        gl::vertex(point);
    }
    gl::end();

    mParams->draw(); // 绘制参数窗口
}

CINDER_APP(BasicApp, RendererGl, prepareSettings)
