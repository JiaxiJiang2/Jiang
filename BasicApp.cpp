#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "DMXPro.hpp"

/*fs::path path = app::getAssetPath("fixtures.json"); // get file in "assets/"-folder
if (path.empty()) { // if not available then create empty one
	path = app::getAssetPath("").string() + "dmx/fixtures.json";
	writeJson(path, ""); // touch
}

ci::Json fixtureDescriptions = ci::loadJson(loadFile(D:/Study/4 semester/Cinder - master/samples/BasicApp/DMX))["devices"];
*/

using namespace ci;
using namespace ci::app;

// We'll create a new Cinder Application by deriving from the App class.
class BasicApp : public App {
  public:
	// Cinder will call 'mouseDrag' when the user moves the mouse while holding one of its buttons.
	// See also: mouseMove, mouseDown, mouseUp and mouseWheel.
	void mouseDrag( MouseEvent event ) override;

	// Cinder will call 'keyDown' when the user presses a key on the keyboard.
	// See also: keyUp.
	void keyDown( KeyEvent event ) override;

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
};

void prepareSettings( BasicApp::Settings* settings )
{
	settings->setMultiTouchEnabled( false );
}

void BasicApp::mouseDrag(MouseEvent event)
{
	// 记录当前鼠标位置 record mouse position
	mPoints.push_back(event.getPos());

	// 将鼠标位置映射Position Mapping到 DMX 范围 (0..255)
	float normalizedX = static_cast<float>(event.getX()) / getWindowWidth();
	float normalizedY = static_cast<float>(event.getY()) / getWindowHeight();

	mPan = normalizedX * 255.0f; // mapping将X映射到 Pan
	mTilt = normalizedY * 255.0f; // mapping将Y映射到 Tilt

	//每一个灯的mapping的轨道不一样（参考json文件）

	// 更新 DMX 通道值
	if (mDmxDevice) {
		mDmxDevice->setValue(mPan, startAddress + 0); // 更新 Pan chanel
		mDmxDevice->setValue(mTilt, startAddress + 2); // 更新 Tilt chanel
	}
}


void BasicApp::keyDown( KeyEvent event )
{

	if( event.getChar() == 'f' ) {
		// Toggle full screen when the user presses the 'f' key.
		setFullScreen( ! isFullScreen() );
	}
	else if( event.getCode() == KeyEvent::KEY_SPACE ) {
		// Clear the list of points when the user presses the space bar.
		mPoints.clear();
	}
	else if( event.getCode() == KeyEvent::KEY_ESCAPE ) {
		// Exit full screen, or quit the application, when the user presses the ESC key.
		if( isFullScreen() )
			setFullScreen( false );
		else
			quit();
	}
}

void BasicApp::draw()
{


	// Clear the contents of the window. This call will clear
	// both the color and depth buffers. 
	gl::clear( Color::gray( 0.1f ) );

	// Set the current draw color to orange by setting values for
	// red, green and blue directly. Values range from 0 to 1.
	// See also: gl::ScopedColor
	gl::color( 1.0f, 0.5f, 0.25f );

	// We're going to draw a line through all the points in the list
	// using a few convenience functions: 'begin' will tell OpenGL to
	// start constructing a line strip, 'vertex' will add a point to the
	// line strip and 'end' will execute the draw calls on the GPU.
	gl::begin( GL_LINE_STRIP );
	for( const vec2 &point : mPoints ) {
		gl::vertex( point );
	}
	gl::end();
}

void BasicApp::update()
{
	/*if (mDmxDevice) {
		
		mPan += 0.2f; // 0..255 灯水平旋转 Light horizontal rotation
		mPan = fmodf(mPan, 255.0f);
		mDmxDevice->setValue(mPan, startAddress + 0);
	}*/



}

void BasicApp::setup()
{
	DMXPro::listDevices();
	std::vector<std::string> devices = DMXPro::getDevicesList();
	if (devices.size() > 0) {
		mDmxDevice = DMXPro::create(devices[0]);
		mDmxDevice->setValue(5, startAddress + 5);
		mDmxDevice->setValue(70, startAddress + 8);//颜色chanel 参考json里

	}
}

// This line tells Cinder to actually create and run the application.
CINDER_APP( BasicApp, RendererGl, prepareSettings )
