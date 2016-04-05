#pragma once

#include <MY_Game.h>

#include <MY_ResourceManager.h>

#include <MY_Scene_Menu.h>

bool MY_Game::resized = false;

MY_Game::MY_Game() :
	Game("menu", new MY_Scene_Menu(this), true) // initialize our game with a menu scene
{
	sweet::setCursorMode(GLFW_CURSOR_NORMAL);
	
	MY_ResourceManager::globalAssets->getAudio("BGM1")->sound->play(true);
	MY_ResourceManager::globalAssets->getAudio("BGM1")->sound->setGain(0);
	MY_ResourceManager::globalAssets->getAudio("BGM2")->sound->play(true);
	MY_ResourceManager::globalAssets->getAudio("BGM2")->sound->setGain(0);
}

MY_Game::~MY_Game(){}

void MY_Game::addSplashes(){
	// add default splashes
	//Game::addSplashes();

	// add custom splashes
	//addSplash(new Scene_Splash(this, MY_ResourceManager::globalAssets->getTexture("DEFAULT")->texture, MY_ResourceManager::globalAssets->getAudio("DEFAULT")->sound));
}

void MY_Game::update(Step * _step){
	MY_ResourceManager::globalAssets->getAudio("BGM1")->sound->update(_step);
	MY_ResourceManager::globalAssets->getAudio("BGM2")->sound->update(_step);

	float g = MY_ResourceManager::globalAssets->getAudio("BGM1")->sound->getGain(false) + 0.01f;
	if(g < 1.f){
		MY_ResourceManager::globalAssets->getAudio("BGM1")->sound->setGain(glm::min(g, 1.f));
	}g = MY_ResourceManager::globalAssets->getAudio("BGM2")->sound->getGain(false) + 0.01f;
	if(g < 0.25f){
		MY_ResourceManager::globalAssets->getAudio("BGM2")->sound->setGain(glm::min(g, 0.25f));
	}

	glm::uvec2 sd = sweet::getWindowDimensions();
	int s = glm::min(sd.x, sd.y);
	if(sd.x != sd.y){
		glfwSetWindowSize(sweet::currentContext, s, s);
		
	}
	if(s != lastSize){
		resized = true;
		lastSize = s;
	}
	Game::update(_step);
	resized = false;
}