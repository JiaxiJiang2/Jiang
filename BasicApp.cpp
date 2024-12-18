#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h" 
#include "cinder/gl/gl.h"
#include "DMXPro.hpp"

using namespace ci;
using namespace ci::app;

// We'll create a new Cinder Application by deriving from the App class.
class BasicApp : public App {
public:
	// Cinder will call 'mouseDrag' when the user moves the mouse while holding one of its buttons.
	// See also: mouseMove, mouseDown, mouseUp and mouseWheel.
	void mouseDrag(MouseEvent event) override;

	// Cinder will call 'keyDown' when the user presses a key on the keyboard.
	// See also: keyUp.
	void keyDown(KeyEvent event) override;

	// Cinder will call 'draw' each time the contents of the window need to be redrawn.
	void draw() override;
	void update() override;

	void setup() override;

private:
	// This will maintain a list of points which we will draw line segments between
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
	// Record mouse position
	vec2 currentPos = event.getPos();

	// Map mouse position to DMX range (0..255)
	float normalizedX = static_cast<float>(currentPos.x) / getWindowWidth();
	float normalizedY = static_cast<float>(currentPos.y) / getWindowHeight();

	mPan = normalizedX * 255.0f; // Map X to Pan
	mTilt = normalizedY * 255.0f; // Map Y to Tilt

	// Update DMX channel values for Pan and Tilt
	if (mDmxDevice) {
		mDmxDevice->setValue(static_cast<int>(mPan), startAddress + 0); // Update Pan channel
		mDmxDevice->setValue(static_cast<int>(mTilt), startAddress + 2); // Update Tilt channel
	}

	// Calculate direction if there's a previous point
	if (!mPoints.empty()) {
		vec2 lastPos = mPoints.back();
		vec2 direction = currentPos - lastPos;

		// Determine direction: up, down, left, right
		if (fabs(direction.x) > fabs(direction.y)) {
			// Horizontal movement
			if (direction.x > 0) {
				mCurrentColor = Color(1.0f, 0.0f, 0.0f); // Red (right)
				if (mDmxDevice) {
					mDmxDevice->setValue(70, startAddress + 7); // R
					mDmxDevice->setValue(0, startAddress + 8);   // G
					mDmxDevice->setValue(0, startAddress + 9);   // B
					mDmxDevice->setValue(0, startAddress + 10);  // W
				}
			}
			else {
				mCurrentColor = Color(0.0f, 1.0f, 0.0f); // Green (left)
				if (mDmxDevice) {
					mDmxDevice->setValue(0, startAddress + 7);
					mDmxDevice->setValue(255, startAddress + 8);
					mDmxDevice->setValue(0, startAddress + 9);
					mDmxDevice->setValue(0, startAddress + 10);
				}
			}
		}
		else {
			// Vertical movement
			if (direction.y > 0) {
				mCurrentColor = Color(1.0f, 1.0f, 1.0f); // White (down)
				if (mDmxDevice) {
					mDmxDevice->setValue(0, startAddress + 7);
					mDmxDevice->setValue(0, startAddress + 8);
					mDmxDevice->setValue(0, startAddress + 9);
					mDmxDevice->setValue(70, startAddress + 10); 
				}
			}
			else {
				mCurrentColor = Color(0.0f, 0.0f, 1.0f); // Blue (up)
				if (mDmxDevice) {
					mDmxDevice->setValue(0, startAddress + 7);
					mDmxDevice->setValue(0, startAddress + 8);
					mDmxDevice->setValue(70, startAddress + 9);
					mDmxDevice->setValue(0, startAddress + 10);
				}
			}
		}
	}

	// Save the current point
	mPoints.push_back(currentPos);
}

void BasicApp::keyDown(KeyEvent event)
{
	if (event.getChar() == 'f') {
		// Toggle full screen when the user presses the 'f' key.
		setFullScreen(!isFullScreen());
	}
	else if (event.getCode() == KeyEvent::KEY_SPACE) {
		// Clear the list of points when the user presses the space bar.
		mPoints.clear();
	}
	else if (event.getCode() == KeyEvent::KEY_ESCAPE) {
		// Exit full screen, or quit the application, when the user presses the ESC key.
		if (isFullScreen())
			setFullScreen(false);
		else
			quit();
	}
}

void BasicApp::draw()
{
	// Clear the contents of the window. This call will clear
	// both the color and depth buffers. 
	gl::clear(Color::gray(0.1f));

	// Set the current draw color based on mouse direction
	gl::color(mCurrentColor);

	// Draw a line through all the points in the list
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
		mDmxDevice->setValue(70, startAddress + 10); //init color
	}
}

// This line tells Cinder to actually create and run the application.
CINDER_APP(BasicApp, RendererGl, prepareSettings)
