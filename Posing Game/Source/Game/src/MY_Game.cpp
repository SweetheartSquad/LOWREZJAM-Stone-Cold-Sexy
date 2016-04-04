#pragma once

#include <MY_Game.h>

#include <MY_ResourceManager.h>

#include <MY_Scene_Menu.h>


MY_Game::MY_Game() :
	Game("menu", new MY_Scene_Menu(this), true) // initialize our game with a menu scene
{
	sweet::setCursorMode(GLFW_CURSOR_NORMAL);
	
	MY_ResourceManager::globalAssets->getAudio("BGM")->sound->play(true);
	MY_ResourceManager::globalAssets->getAudio("BGM")->sound->setGain(0);
}

MY_Game::~MY_Game(){}

void MY_Game::addSplashes(){
	// add default splashes
	//Game::addSplashes();

	// add custom splashes
	//addSplash(new Scene_Splash(this, MY_ResourceManager::globalAssets->getTexture("DEFAULT")->texture, MY_ResourceManager::globalAssets->getAudio("DEFAULT")->sound));
}

void MY_Game::update(Step * _step){
	MY_ResourceManager::globalAssets->getAudio("BGM")->sound->update(_step);

	float g = MY_ResourceManager::globalAssets->getAudio("BGM")->sound->getGain(false);
	if(g < 1.f){
		MY_ResourceManager::globalAssets->getAudio("BGM")->sound->setGain(glm::min(g + 0.001f, 1.f));
	}

	glm::uvec2 sd = sweet::getWindowDimensions();
	if(sd.x != sd.y){
		int s = glm::min(sd.x, sd.y);
		glfwSetWindowSize(sweet::currentContext, s, s);
	}
	Game::update(_step);
}