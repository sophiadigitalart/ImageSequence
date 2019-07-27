#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// Settings
#include "VDSettings.h"
// Session
#include "VDSession.h"
// Log
#include "VDLog.h"
// Spout
#include "CiSpoutOut.h"
// warping
#include "Warp.h"

// UI
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include "VDUI.h"
#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ph::warping;
using namespace videodromm;

class ImageSequenceApp : public App {

public:
	ImageSequenceApp();
	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;
	void fileDrop(FileDropEvent event) override;
	void update() override;
	void draw() override;
	void cleanup() override;
	void toggleCursorVisibility(bool visible);
private:
	// Settings
	VDSettingsRef					mVDSettings;
	// Session
	VDSessionRef					mVDSession;
	// Log
	VDLogRef						mVDLog;
	// UI
	VDUIRef							mVDUI;
	// handle resizing for imgui
	void							resizeWindow();
	// imgui
	/*float							f = 0.0f;
	char							buf[64];
	unsigned int					i, j; */

	bool							mouseGlobal;

	string							mError;
	// fbo
	bool							mIsShutDown;
	Anim<float>						mRenderWindowTimer;
	void							positionRenderWindow();
	bool							mFadeInDelay;
	SpoutOut 						mSpoutOut;
	// warping
	fs::path						mSettings;
	gl::TextureRef					mImage;
	WarpList						mWarps;
	Area							mSrcArea;
};


ImageSequenceApp::ImageSequenceApp()
	: mSpoutOut("ImageSequence", app::getWindowSize())
{
	// Settings
	mVDSettings = VDSettings::create("ImageSequence");
	// Session
	mVDSession = VDSession::create(mVDSettings);
	mVDSession->setMode(2); // blue
	//mVDSettings->mCursorVisible = true;
	toggleCursorVisibility(mVDSettings->mCursorVisible);
	mVDSession->getWindowsResolution();

	mouseGlobal = false;
	mFadeInDelay = true;
	// UI
	mVDUI = VDUI::create(mVDSettings, mVDSession);
	// windows
	mIsShutDown = false;
	mRenderWindowTimer = 0.0f;
	timeline().apply(&mRenderWindowTimer, 1.0f, 2.0f).finishFn([&] { positionRenderWindow(); });
	// warping
	mSettings = getAssetPath("") / "warps.xml";
	if (fs::exists(mSettings)) {
		// load warp settings from file if one exists
		mWarps = Warp::readSettings(loadFile(mSettings));
	}
	else {
		// otherwise create a warp from scratch
		mWarps.push_back(WarpPerspectiveBilinear::create());
	}
	// load test image .loadTopDown()
	try {
		mImage = gl::Texture::create(loadImage(loadAsset("blue (2).jpg")), gl::Texture2d::Format().mipmap(true).minFilter(GL_LINEAR_MIPMAP_LINEAR));

		mSrcArea = mImage->getBounds();

		// adjust the content size of the warps
		Warp::setSize(mWarps, mImage->getSize());
	}
	catch (const std::exception &e) {
		console() << e.what() << std::endl;
	}
}
void ImageSequenceApp::resizeWindow()
{
	mVDUI->resize();
	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize(mWarps);
}
void ImageSequenceApp::positionRenderWindow() {
	mVDSettings->mRenderPosXY = ivec2(mVDSettings->mRenderX, mVDSettings->mRenderY);//20141214 was 0
	setWindowPos(mVDSettings->mRenderX, mVDSettings->mRenderY);
	setWindowSize(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight);
}
void ImageSequenceApp::toggleCursorVisibility(bool visible)
{
	if (visible)
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
}

void ImageSequenceApp::fileDrop(FileDropEvent event)
{
	mVDSession->fileDrop(event);
}
void ImageSequenceApp::update()
{
	mVDSession->setFloatUniformValueByIndex(mVDSettings->IFPS, getAverageFps());
	mVDSession->update();
	mImage = mVDSession->getInputTexture(mVDSession->getMode());
	//gl::Texture::create(loadImage(loadAsset("blue (2).jpg")), gl::Texture2d::Format().loadTopDown().mipmap(true).minFilter(GL_LINEAR_MIPMAP_LINEAR));
// nothing mImage->setTopDown(false);
	mSrcArea = mImage->getBounds();

	// adjust the content size of the warps
	Warp::setSize(mWarps, mImage->getSize());
}
void ImageSequenceApp::cleanup()
{
	if (!mIsShutDown)
	{
		mIsShutDown = true;
		CI_LOG_V("shutdown");
		// save warp settings
		Warp::writeSettings(mWarps, writeFile(mSettings));
		// save settings
		mVDSettings->save();
		mVDSession->save();
		quit();
	}
}
void ImageSequenceApp::mouseMove(MouseEvent event)
{
	if (!Warp::handleMouseMove(mWarps, event)) {
		if (!mVDSession->handleMouseMove(event)) {
			// let your application perform its mouseMove handling here
		}
	}
}
void ImageSequenceApp::mouseDown(MouseEvent event)
{
	if (!Warp::handleMouseDown(mWarps, event)) {
		if (!mVDSession->handleMouseDown(event)) {
			// let your application perform its mouseDown handling here
			if (event.isRightDown()) {
			}
		}
	}
}
void ImageSequenceApp::mouseDrag(MouseEvent event)
{
	if (!Warp::handleMouseDrag(mWarps, event)) {
		if (!mVDSession->handleMouseDrag(event)) {
			// let your application perform its mouseDrag handling here
		}
	}
}
void ImageSequenceApp::mouseUp(MouseEvent event)
{
	if (!Warp::handleMouseUp(mWarps, event)) {
		if (!mVDSession->handleMouseUp(event)) {
			// let your application perform its mouseUp handling here
		}
	}
}

void ImageSequenceApp::keyDown(KeyEvent event)
{
	if (!Warp::handleKeyDown(mWarps, event)) {
		if (!mVDSession->handleKeyDown(event)) {
			switch (event.getCode()) {
			case KeyEvent::KEY_ESCAPE:
				// quit the application
				quit();
				break;
			case KeyEvent::KEY_c:
				// mouse cursor
				mVDSettings->mCursorVisible = !mVDSettings->mCursorVisible;
				toggleCursorVisibility(mVDSettings->mCursorVisible);
				break;			

			case KeyEvent::KEY_w:
				// toggle warp edit mode
				Warp::enableEditMode(!Warp::isEditModeEnabled());
				break;
			case KeyEvent::KEY_a:
				// toggle drawing a random region of the image
				if (mSrcArea.getWidth() != mImage->getWidth() || mSrcArea.getHeight() != mImage->getHeight())
					mSrcArea = mImage->getBounds();
				else {
					int x1 = Rand::randInt(0, mImage->getWidth() - 150);
					int y1 = Rand::randInt(0, mImage->getHeight() - 150);
					int x2 = Rand::randInt(x1 + 150, mImage->getWidth());
					int y2 = Rand::randInt(y1 + 150, mImage->getHeight());
					mSrcArea = Area(x1, y1, x2, y2);
				}
				break;
			}
		}
	}
}
void ImageSequenceApp::keyUp(KeyEvent event)
{
	if (!Warp::handleKeyUp(mWarps, event)) {
		if (!mVDSession->handleKeyUp(event)) {
		}
	}
}

void ImageSequenceApp::draw()
{
	gl::clear(Color::black());
	if (mFadeInDelay) {
		mVDSettings->iAlpha = 0.0f;
		if (getElapsedFrames() > mVDSession->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mVDSettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}

	//gl::setMatricesWindow(toPixels(getWindowSize()),false);
	//gl::setMatricesWindow(1280, 720, false); // must match windowSize
	//gl::setMatricesWindow(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight, false); // must match windowSize
	gl::setMatricesWindow(mVDSession->getIntUniformValueByIndex(mVDSettings->IOUTW), mVDSession->getIntUniformValueByIndex(mVDSettings->IOUTH), false); // must match windowSize
	//gl::draw(mVDSession->getMixTexture(), getWindowBounds());
	if (mImage) {
		for (auto &warp : mWarps) {
			warp->draw(mImage, mSrcArea);
		}
	}
	// Spout Send
	mSpoutOut.sendViewport();
	// imgui
	if (mVDSession->showUI()) {
		mVDUI->Run("UI", (int)getAverageFps());
		if (mVDUI->isReady()) {
		}
	}

	getWindow()->setTitle(mVDSettings->sFps + " fps ImageSequence");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(1280, 720);
}

CINDER_APP(ImageSequenceApp, RendererGl, prepareSettings)
