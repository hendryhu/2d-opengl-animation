#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	c = 0;

	// define the windows in the background
	windows.push_back({ofRectangle(0, 170, 358, 237)});
	windows.push_back({ofRectangle(472, 222, 601, 405)});
	windows.push_back({ofRectangle(1181, 592, 395, 101)});
	windows.push_back({ofRectangle(1181, 119, 190, 406)});  // trapezoid will sit on this window
	windows.push_back({ofRectangle(1441, 0, 100, 383)});
	windows.push_back({ofRectangle(1585, 148, 199, 383)});
	windows.push_back({ofRectangle(218, 444, 135, 183)});
	windows.push_back({ofRectangle(423, 67, 407, 105)});

	// calculate the tilt angles and pivot sides for each window some windows tilt left, some tilt right
	for(auto & window: windows) {
		window.finalTiltAngle = ofDegToRad(ofRandom(2.0f, 15.0f));
		float pivotChoice = ofRandom();
		if(pivotChoice > 0.5f) {
			window.pivotSide = PivotSide::LEFT;
		} else {
			window.pivotSide = PivotSide::RIGHT;
		}
	}

	font.load("../../src/HelveticaNeue.ttf", 32);

	// initialize helper variables
	rectanglePos = glm::vec2(-200, 600);
	rectangleAngle = 0;
	crescentAngle = 0;

	// initialize the trapezoid fall animation
	glm::vec2 startPos(1276, 525 - 45);
	glm::vec2 endPos(1276 - 100, 720);
	float arcHeight = 200.0f;

	int resolution = 100;
	for(int i = 0; i <= resolution; i++) {
		float t = i / static_cast<float>(resolution);
		float x = ofLerp(startPos.x, endPos.x, t);

		// linear path + arc
		float linearY = ofLerp(startPos.y, endPos.y, t);
		float arcY = arcHeight * 4 * (t - t * t);
		float y = linearY - arcY;
		trapezoidFallAnimation.addVertex(glm::vec3(x, y, 0));
	}

	// initialize the crescent animation
	glm::vec2 startPoint(800, 335);
	int numOvals = 5;
	float ovalWidth = ofGetWidth() / 2.0f; // Makes the oval half the screen's width
	float ovalHeight = 150.0f;
	float tiltAngle = ofDegToRad(-15.0f);

	crescentAnimation.clear();
	resolution = 200;
	for(int i = 0; i <= resolution; ++i) {
		// the path needs to start at the top of the oval
		float t = ofMap(i, 0, resolution, 0, numOvals * 2.0f * PI) - PI / 2.0f;  

		// parametric oval
		float x = cos(t) * ovalWidth;
		float y = sin(t) * ovalHeight;
		crescentAnimation.addVertex(glm::vec3(x, y, 0));
	}

	// rotate the entire path by the tilt angle
	// just some affine transformation math from COMP 3501
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), tiltAngle, glm::vec3(0, 0, 1));
	for(auto & p: crescentAnimation.getVertices()) {
		glm::vec4 rotatedPos = rotationMatrix * glm::vec4(p, 1.0f);
		p = glm::vec3(rotatedPos);
	}

	// move the entire path to the correct start location
	glm::vec2 firstPoint = crescentAnimation.getVertices()[0];
	glm::vec2 offset = startPoint - firstPoint;
	for(auto & p: crescentAnimation.getVertices()) {
		p += glm::vec3(offset.x, offset.y, 0);
	}

	// initialize the rectangle animation
	glm::vec2 rectStart(800, 600);
	resolution = 300;

	// pick a random place in the screen for the rectangle to walk to every 30 steps
	int numSegments = 3;
	int pointsPerSegment = 30;
	float border = 200.0f;  // don't walk too far

	for (int i = 0; i < numSegments; ++i) {
		glm::vec2 nextPos;
		// The last segment goes to the bottom right of the screen
		if (i == numSegments - 1) {
			nextPos = glm::vec2(ofGetWidth() / 2.0f, ofGetHeight() / 2.0f);
		} else {
			// Otherwise, pick a random point within the borders
			nextPos.x = ofRandom(border, ofGetWidth() - border);
			nextPos.y = ofRandom(border, ofGetHeight() - border);
		}

		// Add 30 points for the straight line path to the next position
		for (int j = 0; j < pointsPerSegment; ++j) {
			float segmentProgress = j / static_cast<float>(pointsPerSegment - 1);
			glm::vec2 point = rectStart + (nextPos - rectStart) * segmentProgress;			
			rectangleBigAnimation.addVertex(point.x, point.y, 0);
		}
		// The start of the next segment is the end of this one
		rectStart = nextPos;
	}

}

//--------------------------------------------------------------
void ofApp::update() {
	c += 0.005f; // counter goes up
	// cout << c << "\n";
}

//--------------------------------------------------------------
void ofApp::draw() {

	// start of animation, show credits
	if(c < 3.0f) {
		ofSetBackgroundColor(0);
		ofSetColor(255);
		font.drawString("Hendry Hu", 300, 300);
		font.drawString("101263863", 300, 350);
		font.drawString(ofToString(c, 2), 10, 30);
		return;
	}

	// animate each part
	animateBackground();
	animateRectangle();
	animateTrapezoid();
	animateCrescent();

	// display the time counter in the top left corner
	ofSetColor(255);
	font.drawString(ofToString(c, 2), 10, 30);

	// if it's the end, show "The End"
	if(c > 37.0f) {
		ofSetColor(0, 150);
		ofSetColor(255);
		font.drawString("The End", ofGetWidth() / 2 - 70, ofGetHeight() / 2 + 10);
	}

}

// functions to draw static 2d characters
void ofApp::drawTrapezoid(const glm::vec2 pos, const float angle, const PivotSide pivot, float scale) {
	ofSetColor(ofColor::darkGreen);
	ofPushMatrix();
	ofTranslate(pos);

	if(pivot == PivotSide::NONE) {
		ofRotateRad(angle);
	}

	// if we have a pivot, then translate to the pivot point first before rotating
	else {
		glm::vec2 localCorner;
		if(pivot == PivotSide::LEFT) {
			localCorner = glm::vec2(-50, 45);
		} else {
			localCorner = glm::vec2(50, 45);
		}
		ofRotateRad(-angle);  // I genuinely have no idea why this needs to be negative
		ofTranslate(-localCorner);
	}

	ofScale(scale);

	ofDrawTriangle(glm::vec2(-40, -45), glm::vec2(40, -45), glm::vec2(50, 45));
	ofDrawTriangle(glm::vec2(-40, -45), glm::vec2(-50, 45), glm::vec2(50, 45));

	ofPopMatrix();
}

void ofApp::drawRectangle(const glm::vec2 pos, const float angle, const float timeOfDay, float scale) {
	// keep a copy of the position and angle for the crescent to use
	rectangleAngle = angle;
	rectanglePos = pos;

	ofColor rectColor = rectNormalColor.getLerped(rectNightColor, timeOfDay);
	ofSetColor(rectColor);
	ofPushMatrix();
	ofTranslate(pos);
	ofRotateRad(angle);
	ofScale(scale);
	ofDrawRectangle(-120, -200, 240, 400);  // height 400, width 240
	ofPopMatrix();
}

void ofApp::drawCrescent(const glm::vec2 pos, const float angle) {
	ofSetColor(ofColor::lightGoldenRodYellow);
	ofPushMatrix();
	ofTranslate(pos);
	ofRotateRad(angle);

	// the crescent is made of two arcs, one on top of the other
	ofBeginShape();
	float moonWidth = 60;
	float moonHeight = 30;
	float innerArcHeight = 16;
	int resolution = 16;

	// Top edge
	for(int i = 0; i <= resolution; i++) {
		float moonAngle = ofMap(i, 0, resolution, PI, 0);
		float x = (moonWidth / 2) + (moonWidth / 2) * cos(moonAngle);
		float y = moonHeight * sin(moonAngle);
		ofVertex(x, -y);
	}

	// Bottom edge, going backwards to close the shape
	for(int i = resolution; i >= 0; i--) {
		float moonAngle = ofMap(i, 0, resolution, PI, 0);
		float x = (moonWidth / 2) + (moonWidth / 2) * cos(moonAngle);
		float y = innerArcHeight * sin(moonAngle);
		ofVertex(x, -y);
	}
	ofTranslate(-moonWidth / 2, moonHeight / 2);
	ofEndShape(true);
	ofPopMatrix();

	// keep a copy of the angle for the background to use
	crescentAngle = angle / (2 * PI);
}

// functions to animate the characters
// the "timeline" for the trapezoid
void ofApp::animateTrapezoid() {
	// trapezoid rocks back and forth on the windowsill
	if(c > 3.0f && c < 13.5f) {
		float seqTime = c - 3.0f;
		float oscillation = cos(seqTime * PI);
		float angle = ofMap(oscillation, -1.0, 1.0, ofDegToRad(-30.0), ofDegToRad(30.0));

		glm::vec2 leftPivotWorld(1276 - 50, 525);
		glm::vec2 rightPivotWorld(1276 + 50, 525);

		if(angle > 0) { // Tilting right, pivot on the bottom left
			drawTrapezoid(leftPivotWorld, angle, PivotSide::LEFT);
		} else { // Tilting left, pivot on the bottom right
			drawTrapezoid(rightPivotWorld, angle, PivotSide::RIGHT);
		}
	}

	// trapezoid is still on the windowsill
	if(c >= 13.5f && c < 15.0f) {
		glm::vec2 basePos(1276, 525 - 45);
		drawTrapezoid(basePos, 0);
	}

	// trapezoid jumps up and down rapidly on the windowsill
	if(c >= 15.0f && c < 16.0f) {
		float seqTime = c - 15.0f;

		// 4 jumps per second, this was just trial and error in Desmos
		float jumpProgress = abs(sin(seqTime * PI * 4)); 
		float jumpHeight = 60.0f;
		glm::vec2 basePos(1276, 525 - 45);
		glm::vec2 jumpPos(basePos.x, basePos.y - jumpProgress * jumpHeight);
		drawTrapezoid(jumpPos, 0);
	}

	// trapezoid is still on the windowsill, reacts with rectangle's landing by going up and down a bit
	if(c >= 16.0f && c < 20.0f) {
		glm::vec2 basePos(1276, 525 - 45);

		// rectangle lands at c = 18, c = 19 (ignore c = 20, as it will do its custom falling thingy)
		// motion takes 0.4 seconds
		if((c >= 18.0f && c < 18.4f) || (c >= 19.0f && c < 19.4f)) {
			float impactTime = fmod(c, 1.0f); // time since last whole second
			float impactProgress = ofMap(impactTime, 0.0f, 0.4f, 0.0f, 1.0f);
			float easedImpact = sin(impactProgress * PI); // easing

			float impactHeight = 15.0f;
			glm::vec2 impactPos(basePos.x, basePos.y - easedImpact * impactHeight);
			drawTrapezoid(impactPos, 0);
		} else {
			drawTrapezoid(basePos, 0);
		}
	}

	// trapezoid bounces out of the windowsill and onto the ground to the left
	if(c >= 20.0f && c < 22.0f) {
		float seqTime = c - 20.0f;
		float t;
    
		// going up (0.5 seconds)
		if(seqTime < 0.5f) {
			float upProgress = seqTime / 0.5f;
			// slow down as we approach the top
			float easedUpProgress = sin(upProgress * PI/2);
			t = easedUpProgress * 0.25f;
		} 
		// going down (1.5 seconds) 
		else {
			float downProgress = (seqTime - 0.5f) / 1.5f;
			// speed up as we approach the ground
			float easedDownProgress = 1.0f - cos(downProgress * PI/2);
			t = 0.25f + (easedDownProgress * 0.75f);
		}
    
		// Get position along the precomputed path
		glm::vec2 positionAtLine = trapezoidFallAnimation.getPointAtPercent(t);
    
		// Rotate a few times while falling
		float angle = ofMap(seqTime, 0.0, 2.0, 0, PI * 4);
    
		// Scale up to 2x size while falling
		float scale = ofMap(seqTime, 0.0, 2.0, 1.0, 2.0f);
    
		drawTrapezoid(positionAtLine, angle, PivotSide::NONE, scale);
	}

	// trapezoid is on the ground
	if(c >= 22.0f && c < 34.0f) {
		glm::vec2 finalPos = glm::vec2(trapezoidFallAnimation.getPointAtPercent(1.0f).x, trapezoidFallAnimation.getPointAtPercent(1.0f).y);
		drawTrapezoid(finalPos, 0, PivotSide::NONE, 2.0f);
	}

	// move towards the middle
	if (c >= 34.0f && c < 35.0f) {
		float seqTime = c - 34.0f;
		float progress = seqTime / 1.0f;
		float easedProgress = progress * progress * (3.0f - 2.0f * progress); // smoothstep
		glm::vec2 startPos = glm::vec2(trapezoidFallAnimation.getPointAtPercent(1.0f).x, trapezoidFallAnimation.getPointAtPercent(1.0f).y);
		glm::vec2 targetPos = glm::vec2(ofGetWidth() / 2, trapezoidFallAnimation.getPointAtPercent(1.0f).y);
		glm::vec2 newPos = glm::vec2(ofLerp(startPos.x, targetPos.x, easedProgress), ofLerp(startPos.y, targetPos.y, easedProgress));
		drawTrapezoid(newPos, 0, PivotSide::NONE, 2.0f);
	}

	// stays there forever
	if(c >= 35.0f) {
		glm::vec2 finalPos = glm::vec2(ofGetWidth() / 2, trapezoidFallAnimation.getPointAtPercent(1.0f).y);
		drawTrapezoid(finalPos, 0, PivotSide::NONE, 2.0f);
	}
}

// the "timeline" for the rectangle
void ofApp::animateRectangle() {
	// rectangle walks in from the left, pauses a bit in front of the trapezoid
	if(c > 5.0f && c < 10.0f) {
		float seqTime = c - 5.0f;
		float progress = ofMap(seqTime, 0.0f, 5.0f, 0.0f, 1.0f);
		float easedProgress = 1.0f - pow(1.0f - progress, 3.0f); // ease out cubic

		glm::vec2 start(-200, 600);
		glm::vec2 end(800, 600);
		glm::vec2 basePos = start + (end - start) * easedProgress;

		// bob up and down with noise
		float bobNoise = ofNoise(seqTime * 1.0f + 1000.0f) * 30.0f;
		bobNoise *= (1.0f - easedProgress);
		glm::vec2 bobbedPos(basePos.x, basePos.y + bobNoise);

		// tilt a bit with noise
		float angleNoise = ofNoise(seqTime * 1.0f + 2000.0f);
		angleNoise = ofDegToRad(ofMap(angleNoise, 0.0f, 1.0f, 0.0f, 30.0f));
		angleNoise *= (1.0f - easedProgress);

		drawRectangle(bobbedPos, angleNoise);
	}

	// rectangle just bobs up and down in front of the trapezoid
	if(c >= 10.0f && c < 17.0f) {
		float seqTime = c - 10.0f;
		glm::vec2 basePos(800, 600);
		float bobNoise = ofNoise(seqTime * 1.0f + 3000.0f) * 30.0f;

		// fade in the bobbing motion over 1 second to not make the rectangle teleport
		if(c < 11.0f) {
			float fadeIn = ofMap(c, 10.0f, 11.0f, 0.0f, 1.0f);
			bobNoise *= fadeIn;
		}

		// fade out motion, same thing
		if(c > 16.0f) {
			float fadeOut = ofMap(c, 16.0f, 17.0f, 1.0f, 0.0f);
			bobNoise *= fadeOut;
		}

		glm::vec2 bobbedPos(basePos.x, basePos.y + bobNoise);
		drawRectangle(bobbedPos, 0);
	}

	// rectangle jumps up and down slowly
	// see the trapezoid jumping code for explanation, it's the same thing
	if(c >= 17.0f && c < 20.0f) {
		float seqTime = c - 17.0f;
		float jumpProgress = abs(sin(seqTime * PI));
		float jumpHeight = 60.0f;
		glm::vec2 basePos(800, 600);
		glm::vec2 jumpPos(basePos.x, basePos.y - jumpProgress * jumpHeight);
		drawRectangle(jumpPos, 0);
	}

	// rectangle stays still
	if(c >= 20.0f && c < 31.0f) {
		glm::vec2 basePos(800, 600);
		drawRectangle(basePos, 0);
	}

	// rectangle follows the path and becomes big
	if (c >= 31.0f && c <= 34.0f) {
		float seqTime = c - 31.0f;
		float progress = ofMap(seqTime, 0.0f, 3.0f, 0.0f, 1.0f);
		float easedProgress = progress * progress * (3.0f - 2.0f * progress); // smoothstep
		progress = easedProgress;
		glm::vec2 positionAtLine = rectangleBigAnimation.getPointAtPercent(easedProgress);

		// slowly rotate to 90 degrees
		float angle = ofMap(progress, 0.0, 1.0, 0, PI / 2.0f);

		drawRectangle(positionAtLine, angle, progress, 1 + progress * 3.0f);
	}

	// rectangle stays still at (0,0)
	if(c > 34.0f) {
		drawRectangle(glm::vec2(0, 0), PI / 2.0f, 1.0f, 10.0f);
	}
}

// the "timeline" for the crescent
void ofApp::animateCrescent() {
	// jump up and down on top of the rectangle
	if(c > 3.0f && c < 10.0f) {
		float seqTime = c - 3.0f;
		float jumpProgress = abs(sin(seqTime * PI));

		// it needs to follow the rectangle's head
		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		float jumpHeight = 60.0f;
		glm::vec2 jumpPos(basePosX, basePosY - jumpProgress * jumpHeight);

		float angle = rectangleAngle;  // tilt with the rectangle
		drawCrescent(jumpPos, angle);
	}

	// stays still for a bit on top of the rectangle
	if(c >= 10.0f && c < 12.0f) {
		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		glm::vec2 basePos(basePosX, basePosY);
		drawCrescent(basePos, 0);
	}

	// jumps up and down rapidly on top of the rectangle
	// see the trapezoid jumping code for explanation, it's the same thing
	if(c >= 12.0f && c < 13.0f) {
		float seqTime = c - 12.0f;
		float jumpProgress = abs(sin(seqTime * PI * 4)); // 4 jumps per second

		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		float jumpHeight = 80.0f;
		glm::vec2 jumpPos(basePosX, basePosY - jumpProgress * jumpHeight);

		drawCrescent(jumpPos, 0);
	}

	// stays still on top of the rectangle, reacts with rectangle's landing by going up and down a bit
	if(c >= 13.0f && c < 22.0f) {
		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		glm::vec2 basePos(basePosX, basePosY);

		// rectangle lands at c = 18, c = 19, c = 20, so the crescent reacts at those times too
		if((c >= 18.0f && c < 18.5f) || (c >= 19.0f && c < 19.5f) || (c >= 20.0f && c < 20.5f)) {
			float impactTime = fmod(c, 1.0f); // time since last whole second
			float impactProgress = ofMap(impactTime, 0.0f, 0.5f, 0.0f, 1.0f);
			float easedImpact = sin(impactProgress * PI); // easing
			float impactHeight = 30.0f;
			glm::vec2 impactPos(basePos.x, basePos.y - easedImpact * impactHeight);
			drawCrescent(impactPos, 0);
		} else {
			drawCrescent(basePos, 0);
		}
	}

	// after the trapezoid hits the ground, crescent does a big jump on top of the rectangle, and ends up upside down
	if(c >= 22.0f && c < 25.0f) {
		float seqTime = c - 22.0f;
		float progress = ofMap(seqTime, 0.0f, 3.0f, 0.0f, 1.0f);
		float easedProgress = sin(progress * PI); // easing

		// still needs to follow the rectangle's head
		float basePosY = rectanglePos.y - 200 - 15;
		float jumpHeight = 200.0f;
		float basePosX = rectanglePos.x;
		glm::vec2 jumpPos(basePosX, basePosY - easedProgress * jumpHeight);

		// flips a few times in the air
		float angle = ofMap(progress, 0.0, 1.0, 0, PI * 7);
		drawCrescent(jumpPos, angle);
	}

	// stays on top of the rectangle 
	if(c >= 25.0f && c < 26.0f) {
		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		glm::vec2 basePos(basePosX, basePosY);
		drawCrescent(basePos, PI); // upside down, nighttime
	}

	// jumps up and down once more
	// see the trapezoid jumping code for explanation, it's the same thing
	if(c >= 26.0f && c < 26.5f) {
		float seqTime = c - 26.0f;
		float jumpProgress = abs(sin(seqTime * PI * 2));
		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		float jumpHeight = 60.0f;
		glm::vec2 jumpPos(basePosX, basePosY - jumpProgress * jumpHeight);
		drawCrescent(jumpPos, PI);
	}

	// stays for another 1.5 seconds
	if(c >= 26.5f && c < 28.0f) {
		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		glm::vec2 basePos(basePosX, basePosY);
		drawCrescent(basePos, PI);
	}

	// jumps up and down once more
	// I don't need to say it again, it's the same thing
	if(c >= 28.0f && c < 28.5f) {
		float seqTime = c - 28.0f;
		float jumpProgress = abs(sin(seqTime * PI * 2));
		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		float jumpHeight = 60.0f;
		glm::vec2 jumpPos(basePosX, basePosY - jumpProgress * jumpHeight);
		drawCrescent(jumpPos, PI);
	}

	// stays for another 1.5 seconds
	if(c >= 28.5f && c < 30.0f) {
		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		glm::vec2 basePos(basePosX, basePosY);
		drawCrescent(basePos, PI);
	}

	// moves up a bit
	if(c >= 30.0f && c < 30.5f) {
		float seqTime = c - 30.0f;
		float progress = seqTime / 0.5f;
		float easedProgress = progress * progress * (3.0f - 2.0f * progress); // smoothstep

		// relative to the rectangle's head
		float basePosX = rectanglePos.x;
		float basePosY = rectanglePos.y - 200 - 15;
		float targetX = crescentAnimation.getPointAtPercent(0.0f).x;
		float targetY = crescentAnimation.getPointAtPercent(0.0f).y;
		glm::vec2 newPos = glm::vec2(ofLerp(basePosX, targetX, easedProgress), ofLerp(basePosY, targetY, easedProgress));

		// angle goes from PI to PI/2
		float angle = ofMap(easedProgress, 0.0f, 1.0f, PI, PI / 2.0f);

		drawCrescent(newPos, angle);
	}

	// spins around the screen superfast
	if (c >= 30.5f && c < 34.0f) {
		float seqTime = c - 30.5f;
		float progress = seqTime / 3.5f;
		float easedProgress = progress * progress * (3.0f - 2.0f * progress); // smoothstep
		glm::vec3 pos = crescentAnimation.getPointAtPercent(easedProgress);
		drawCrescent(pos, PI / 2.0f);
	}

	// moves left and up a bit
	if(c >= 34.0f && c < 35.0f) {
		float seqTime = c - 34.0f;
		float progress = seqTime / 1.0f;
		float easedProgress = progress * progress * (3.0f - 2.0f * progress); // smoothstep
		float basePosX = crescentAnimation.getPointAtPercent(1.0f).x;
		float basePosY = crescentAnimation.getPointAtPercent(1.0f).y;
		glm::vec2 newPos(basePosX - easedProgress * 320.0f, basePosY - easedProgress * 100.0f);
		drawCrescent(newPos, PI / 2.0f + PI / 6.0f * easedProgress);
	}

	// stays there forever
	if(c >= 35.0f) {
		float basePosX = crescentAnimation.getPointAtPercent(1.0f).x - 320.0f;
		float basePosY = crescentAnimation.getPointAtPercent(1.0f).y - 100.0f;
		glm::vec2 basePos(basePosX, basePosY);
		drawCrescent(basePos, PI / 2.0f + PI / 6.0f);
	}
}

// the "timeline" for the background
void ofApp::animateBackground() {
	// normally draw windows until the rectangle lands for the third time
	if(c < 20.0f) {
		for(auto & window: windows) {
			window.tiltAngle = 0.0f;
		}
		drawBackground(1.0f);
	}

	// after the rectangle lands for the third time, the windows tilt down all at once
	// it will tilt over 0.5 seconds, then bounce up and down (time decaying) for 1.5 seconds
	if(c >= 20.0f && c < 22.0f) {
		float seqTime = c - 20.0f;
		float angleProgress = ofMap(seqTime, 0.0f, 2.0f, 0.0f, 1.0f);
		angleProgress = abs(sin(angleProgress * PI * 0.5f)); // ease out sine
		for(auto & window: windows) {
			window.tiltAngle = window.finalTiltAngle * angleProgress;
		}
		drawBackground(1.0f);
	}

	// windows stay tilted, the time of day is dependent on the rotation of the moon
	if(c >= 22.0f && c < 28.0f) {
		for(auto & window: windows) {
			window.tiltAngle = window.finalTiltAngle;
		}

		// crescentAngle is between 0 and 2*PI
		// when the crescentAngle is upright (0), it is day (1)
		// when the crescentAngle is upside-down (PI), it is night (0)
		float timeOfDay = (cos(crescentAngle * 2 * PI) + 1) / 2.0f;
		drawBackground(timeOfDay);
	}

	// for the rest of the animation, the windows stay tilted and the time of day is night
	if(c >= 28.0f) {
		for(auto & window: windows) {
			window.tiltAngle = window.finalTiltAngle;
		}
		drawBackground(0.0f);
	}
}

// function to draw the background
void ofApp::drawBackground(const float timeOfDay) {
	ofSetBackgroundColor(118, 136, 155);
	for(const auto & window: windows) {
		ofRectangle rect = window.rect;
		PivotSide pivot = window.pivotSide;
		float angle = window.tiltAngle;

		// interpolate background color based on time of day (0 = night, 1 = day)
		ofColor bgColor = bgColorNight.getLerped(bgColorDay, timeOfDay);
		ofSetColor(bgColor);

		ofPushMatrix();
		ofTranslate(rect.x + rect.width / 2, rect.y + rect.height / 2);

		// tilt the window depending on the pivot side and angle (this is for when the rectangle lands for the third time)
		if(pivot == PivotSide::NONE) {
			ofRotateRad(angle);
		} else {
			glm::vec2 localCorner;
			if(pivot == PivotSide::LEFT) {
				localCorner = glm::vec2(-rect.width / 2, -rect.height / 2);
				angle = -angle; // left pivot needs to rotate clockwise
			} else {
				localCorner = glm::vec2(rect.width / 2, -rect.height / 2);
			}
			ofTranslate(-localCorner);
			ofRotateRad(angle);
			ofTranslate(localCorner);
		}
		ofDrawRectangle(-rect.width / 2, -rect.height / 2, rect.width, rect.height);
		ofPopMatrix();
	}
}

// everything below is unused, you can stop looking!
//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	// space bar = reset animation
	if(key == ' ') {
		c = 0;
	}

	// right arrow = skip forward 1 second
	if(key == OF_KEY_RIGHT) {
		c += 1.0f;
	}

	// left arrow = skip backward 1 second
	if(key == OF_KEY_LEFT) {
		c -= 1.0f;
		c = std::max<float>(c, 0);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {}
