#pragma once

#include "ofMain.h"

enum class PivotSide {
	NONE,
	LEFT,
	RIGHT
};

struct Window {
	ofRectangle rect;
	float tiltAngle = 0.0f; // in radians
	float finalTiltAngle = 0.0f; // in radians
	PivotSide pivotSide = PivotSide::NONE;
};

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	// functions to draw the characters
	void drawTrapezoid(glm::vec2 pos, float angle, PivotSide pivot = PivotSide::NONE, float scale = 1.0f);
	void drawRectangle(glm::vec2 pos, float angle, float timeOfDay = 0.0f, float scale = 1.0f);
	void drawCrescent(glm::vec2 pos, float angle);

	// functions to animate
	void animateTrapezoid();
	void animateRectangle();
	void animateCrescent();
	void animateBackground();

	// function to draw the background
	void drawBackground(float timeOfDay);

	float c; // current time in the animation
	std::vector<Window> windows;
	ofTrueTypeFont font;

	// helper variables for animation
	glm::vec2 rectanglePos;
	float rectangleAngle;
	float crescentAngle;
	ofPolyline trapezoidFallAnimation;
	ofPolyline crescentAnimation;
	ofPolyline rectangleBigAnimation;
	ofColor bgColorDay = ofColor(158, 207, 218);
	ofColor bgColorNight = ofColor(22, 31, 63);
	ofColor rectNormalColor = ofColor::sandyBrown;
	ofColor rectNightColor = ofColor(22, 31, 63);
};
