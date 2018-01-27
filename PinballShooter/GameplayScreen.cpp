#include "GameplayScreen.h"
#include <SDL\SDL.h>
#include <iostream>
#include <Gutengine\IMainGame.h>
#include <Gutengine\ResourceManager.h>

#include <random>
#include <math.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#define b2_velocityThreshold = 0.0f

const float VISION_RADIUS = 20.0f;
const float WEAPON_ANGLE = 12.0f;
const float WEAPON_RANGE = 10.0f;
const float BLAST_POWER = 50.0f;
const float PI = 3.14159265359f;

float angleBetweenVectors(glm::vec2 a, glm::vec2 b) { 
	return glm::angle(glm::normalize(a), glm::normalize(b));
}

bool isWithinAngle(glm::vec2 a, glm::vec2 b, float angle) {
	return angle <= angleBetweenVectors(a, b) ? false : true;
}

void applyBlastImpulse(b2Body* body, b2Vec2 blastCenter, b2Vec2 applyPoint, float blastPower) {
	b2Vec2 blastDir = applyPoint - blastCenter;
	float distance = blastDir.Normalize();
	//ignore bodies exactly at the blast point - blast direction is undefined 
	if (distance == 0)
		return;
	float invDistance = 1 / distance;
	float impulseMag = blastPower * invDistance;
	body->ApplyLinearImpulse(impulseMag * blastDir, applyPoint, true );
}

GameplayScreen::GameplayScreen(Gutengine::Window * window) : m_window(window)
{
}


GameplayScreen::~GameplayScreen()
{
}

int GameplayScreen::getNextScreenIndex() const
{
	return SCREEN_INDEX_NO_SCREEN;
}

int GameplayScreen::getPreviousScreenIndex() const
{
	return SCREEN_INDEX_NO_SCREEN;
}

void GameplayScreen::build()
{
	std::cout << "Build: \n";
}

void GameplayScreen::destroy()
{
	std::cout << "Destroy: \n";
}

void GameplayScreen::onEntry()
{
	const int NUM_BOXES = 4;
	const int NUM_ENEMIES = 50;
	const int NUM_PINBALLS = 10;

	std::cout << "OnEntry: \n";

	// Initialize spritebatch
	m_spriteBatch.init();

	// Shader initialization
	initShaders();
	
	// Initialize debug Renderer
	m_debugRenderer.init();

	b2Vec2 gravity(0.0f, 0.0f); //< we don't want gravity
	m_world = std::make_unique<b2World>(gravity);
	m_world->SetContactListener(&m_contactListenerInstance);

	// make the level edges
	makeLevelEdges();

		// initialize random generator 
	std::mt19937 randGenerator;
		// could seed the random generator with time, but we wont
		// set distributions
	std::uniform_real_distribution<float> xDistPos(-10.0f, 10.0f);
	std::uniform_real_distribution<float> yDistPos(-10.0f, 10.0f);
	std::uniform_real_distribution<float> sizeDistribution(25.f, 45.f);
	std::uniform_int_distribution<int> ColorDist(0, 255);


	

	//make a bunch of new boxes and push them into the boxes vector container
	/*
	for (int i = 0; i < NUM_BOXES; i++) {
		// randomize color
		Gutengine::ColorRGBA8 randColor;
		randColor.r = ColorDist(randGenerator);
		randColor.g = ColorDist(randGenerator);
		randColor.b = ColorDist(randGenerator);
		randColor.a = 255;

		// create a new box with random pos(x,y), size(w,h) and color(r,g,b), set fixedRotation to false
		Box newBox;
		newBox.init(m_world.get(),
			glm::vec2(xDistPos(randGenerator), yDistPos(randGenerator)),
			glm::vec2(sizeDistribution(randGenerator), sizeDistribution(randGenerator)),
			Gutengine::ResourceManager::getTexture("Assets/bricks_top.png"),
			randColor,
			false,
			b2_dynamicBody);
		// push new created box to vector container
		m_boxes.push_back(newBox);
	}
	m_boxes[1].getBody()->SetFixedRotation(true);
	*/

	// make enemies
	for (int i = 0; i < NUM_ENEMIES; i++) {
		Enemy* newEnemy = new Enemy();
		newEnemy->init(m_world.get(),
			glm::vec2(xDistPos(randGenerator), yDistPos(randGenerator)),
			1.0f,
			Gutengine::ColorRGBA8(255, 255, 255, 255));
		m_enemies.push_back(newEnemy);
	}
	
	// make pinballs
	for (int i = 0; i < NUM_PINBALLS; i++) {
		Pinball* newBall = new Pinball();
		newBall->init(m_world.get(),
			glm::vec2(0.0f + i, 1.0f),
			2.0f,
			Gutengine::ColorRGBA8(255, 124, 124, 255));
		m_pinballs.push_back(newBall);
	}

	// initialize camera.
	m_camera.init(m_window->getScreenWidth(), m_window->getScreenHeight());
	m_camera.setScale(24.0f); // 24 pixels / meter

	// Initialize player
	m_player.init(m_world.get(), glm::vec2(0.0f, 0.0f), glm::vec2(2.f, 2.f), Gutengine::ColorRGBA8(255, 255, 255, 255));

}

void GameplayScreen::onExit()
{
	std::cout << "OnExit: \n";
	m_debugRenderer.dispose();
}

void GameplayScreen::update()
{
	b2Vec2 playerPos;
	
	// Update the player
	m_player.update(m_game->inputManager, m_camera);

	playerPos.x = m_player.getCircle().getBody()->GetPosition().x;
	playerPos.y = m_player.getCircle().getBody()->GetPosition().y;

	// Update the camera
	m_camera.update();

	// Update the enemies
	for (auto itr : m_enemies)
		itr->update(m_game->inputManager, m_camera, glm::vec2(playerPos.x, playerPos.y));

	// Update the balls
	for (auto itr : m_pinballs)
		itr->update(m_game->inputManager, m_camera);


	 // Check collision list and delete enemies, etc.
	for (b2Contact* contact = m_world->GetContactList(); contact;  contact = contact->GetNext()) {
	}

	// RAYCASTING
	glm::vec2 mousePosition = m_camera.convertScreenToWorld(m_game->inputManager.getMouseCoords());

	// clear the results vector
	m_callbackResults.clear();
	// ray cast to each enemy;
	b2Vec2 enemyPosition;
	for (auto enemyItr : m_enemies) {
		
		enemyPosition.x = enemyItr->getCircle().getBody()->GetPosition().x;
		enemyPosition.y = enemyItr->getCircle().getBody()->GetPosition().y;
		
		if (isWithinAngle(m_player.getPosition() - mousePosition, m_player.getPosition() - enemyItr->getPosition(), WEAPON_ANGLE)) 
		{
			if (glm::distance(m_player.getPosition(), enemyItr->getPosition()) <= WEAPON_RANGE) 
			{
				m_world->RayCast(&m_rcCallback, playerPos, enemyPosition);
				m_callbackResults.push_back(m_rcCallback.m_fixture->GetBody());
			}
		}
	}

	// raycast to each pinball
	b2Vec2 pinballPosition;
	for (auto ballItr : m_pinballs) {

		pinballPosition.x = ballItr->getCircle().getBody()->GetPosition().x;
		pinballPosition.y = ballItr->getCircle().getBody()->GetPosition().y;

		if (isWithinAngle(m_player.getPosition() - mousePosition, m_player.getPosition() - ballItr->getPosition(), WEAPON_ANGLE))

		{
			if (glm::distance(m_player.getPosition(), ballItr->getPosition()) <= WEAPON_RANGE)
			{
				m_world->RayCast(&m_rcCallback, playerPos, pinballPosition);
				m_callbackResults.push_back(m_rcCallback.m_fixture->GetBody());
			}
		}
	}


	checkInput();
	
	// update The physics simulation in BOX2D world
	m_world->Step(1.0f / 60.0f, 6, 2);
}

void GameplayScreen::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); ///< Clear gl color and depth buffers
	glClearColor(0.0f, 0.25f, 0.0f, 1.0f); ///< set Clear color to Solid red

	m_textureProgram.use();

	// upload texture uniform
	GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
	glUniform1d(textureUniform, 0);
	glActiveTexture(GL_TEXTURE0);

	//camera matrix
	glm::mat4 projectionMatrix = m_camera.getCameraMatrix();
	GLint pUniform = m_textureProgram.getUniformLocation("P");
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

	m_spriteBatch.begin();

	// player collider
	auto p = m_player.getCircle();
	// player position
	glm::vec2 playerPosition = glm::vec2(m_player.getCircle().getBody()->GetPosition().x,
										 m_player.getCircle().getBody()->GetPosition().y);

	// mouse position
	glm::vec2 mouseWorldPosition = m_camera.convertScreenToWorld(m_game->inputManager.getMouseCoords());

	// draw all the boxes
	/*
	for (auto b : m_boxes) {
		b.draw(m_spriteBatch);
	}
	*/

	// draw all the pinballs
	for (auto itr : m_pinballs) {
		itr->draw(m_spriteBatch);
	}

	// draw the player
	m_player.draw(m_spriteBatch);

	// draw the enemies
	for (auto itr : m_enemies) {
		itr->draw(m_spriteBatch);
	}

	m_spriteBatch.end();

	m_spriteBatch.renderBatch();
	m_textureProgram.unuse();

	// Debug rendering
	if (m_renderDebug) {
		glm::vec4 destRect;
		// Render obstacle box collision boxes
		/*
		for (auto& b : m_boxes) {
			destRect.x = b.getBody()->GetPosition().x - b.getDimensions().x / 2.0f;
			destRect.y = b.getBody()->GetPosition().y - b.getDimensions().y / 2.0f;
			destRect.z = b.getDimensions().x;
			destRect.w = b.getDimensions().y;
			m_debugRenderer.drawBox(destRect, Gutengine::ColorRGBA8(255, 255, 255, 255), b.getBody()->GetAngle());
		}
		*/
		// render enemy collision boxes

		for (auto& itr : m_enemies) {
			glm::vec2 enemyPosition = glm::vec2(itr->getCircle().getBody()->GetPosition().x,
												itr->getCircle().getBody()->GetPosition().y);

			if (isWithinAngle(playerPosition - mouseWorldPosition, playerPosition - enemyPosition, WEAPON_ANGLE)) {
				if (glm::distance(playerPosition, enemyPosition) <= WEAPON_RANGE) {

					m_debugRenderer.drawCircle(glm::vec2(itr->getCircle().getBody()->GetPosition().x, itr->getCircle().getBody()->GetPosition().y),
						Gutengine::ColorRGBA8(255, 0, 0, 255),
						itr->getCircle().getDimensions().x / 2.0f);
				}
			}
		}

		// draw Origin lines
		m_debugRenderer.drawLine(glm::vec2(0.0f, 0.0f), glm::vec2(0.25f, 0.0f), Gutengine::ColorRGBA8(0, 0, 255, 255));
		m_debugRenderer.drawLine(m_camera.convertScreenToWorld(glm::vec2(0.0f, 0.0f)),
								 m_camera.convertScreenToWorld(glm::vec2(24.0f, 24.0f)),
								 Gutengine::ColorRGBA8(0, 0, 255, 255));


		//render player collision Circle
		m_debugRenderer.drawCircle( playerPosition,
									Gutengine::ColorRGBA8(255, 255, 255, 255),
									p.getDimensions().x / 2.0f);
		

		// draw arc lines ////////////////////////

		m_debugRenderer.drawLine(playerPosition,
								 playerPosition + glm::normalize(mouseWorldPosition - playerPosition) * WEAPON_RANGE,
								 Gutengine::ColorRGBA8(255, 0, 0, 255));

		m_debugRenderer.drawLine(playerPosition,
			playerPosition + glm::rotate(glm::normalize(mouseWorldPosition - playerPosition), WEAPON_ANGLE) * WEAPON_RANGE,
			Gutengine::ColorRGBA8(255, 0, 255, 255));

		m_debugRenderer.drawLine(playerPosition,
			playerPosition + glm::rotate(glm::normalize(mouseWorldPosition - playerPosition), -WEAPON_ANGLE) * WEAPON_RANGE,
			Gutengine::ColorRGBA8(255, 255, 0, 255));

		m_debugRenderer.drawLine(playerPosition + glm::normalize(mouseWorldPosition - playerPosition) * WEAPON_RANGE,
			playerPosition + glm::rotate(glm::normalize(mouseWorldPosition - playerPosition), WEAPON_ANGLE) * WEAPON_RANGE,
			Gutengine::ColorRGBA8(255, 255, 0, 255));

		m_debugRenderer.drawLine(playerPosition + glm::normalize(mouseWorldPosition - playerPosition) * WEAPON_RANGE,
			playerPosition + glm::rotate(glm::normalize(mouseWorldPosition - playerPosition), -WEAPON_ANGLE) * WEAPON_RANGE,
			Gutengine::ColorRGBA8(255, 255, 0, 255));

		// ----------------------------///////////
		// draw lines raycast results

		for (auto itr : m_callbackResults) {
			m_debugRenderer.drawLine
			(glm::vec2(itr->GetPosition().x, itr->GetPosition().y),
				glm::vec2(p.getBody()->GetPosition().x, p.getBody()->GetPosition().y),
				Gutengine::ColorRGBA8(126, 126, 126, 126));
		}
		// drawing end
		
		m_debugRenderer.end();
		m_debugRenderer.render(projectionMatrix, 2.0f);
		// debug rendering - end.
	}
}

void GameplayScreen::checkInput()
{
	SDL_Event evnt;
	while (SDL_PollEvent(&evnt)) {
		m_game->onSDLEvent(evnt);
	}
	
	// Weapon: blast all enemies in arc
	if (m_game->inputManager.isKeyPressed(SDL_BUTTON_LEFT)) {
		// get Player position, which will be the center of the blast
		glm::vec2 playerPosition = glm::vec2(	m_player.getCircle().getBody()->GetPosition().x,
												m_player.getCircle().getBody()->GetPosition().y);
		
		for (auto itr : m_callbackResults) {
				applyBlastImpulse(itr,
					b2Vec2(playerPosition.x, playerPosition.y),
					b2Vec2(itr->GetPosition().x, itr->GetPosition().y),
					BLAST_POWER);
			
		}
	}
	if (m_game->inputManager.isKeyPressed(SDLK_q)) {

		if (m_renderDebug)
			m_renderDebug = false;
		else
			m_renderDebug = true;
	}
}

void GameplayScreen::initShaders()
{
	// Compile our color shader
	m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
	m_textureProgram.addAttribute("vertexPosition");
	m_textureProgram.addAttribute("vertexColor");
	m_textureProgram.addAttribute("vertexUV");
	m_textureProgram.linkShaders();
}

void GameplayScreen::makeLevelEdges()
{
	makeEdge(0.0f, 25.0f, 50.0f, 10.0f); // top edge
	makeEdge(0.0f, -25.0f, 50.0f, 10.0f); // bottom edge
	makeEdge(-35.0f, 0.0f,  10.0f, 50.0f); // left edge
	makeEdge(35.0f, 0.0f,  10.0f, 50.0f); // right edge
}

void GameplayScreen::makeEdge(float x, float y, float w, float h) {
	// make the level edges
	b2BodyDef BarrierDef;
	BarrierDef.position.Set(x, y);

	//allocates barrier in memory
	b2Body* Barrier = m_world->CreateBody(&BarrierDef);
	// make the ground fixture - bind the shape of the body.
	b2PolygonShape box;
	box.SetAsBox(w, h);
	Barrier->CreateFixture(&box, 0.0f); // density is 0.0f so they can't be moved

	//make level edges - end
}