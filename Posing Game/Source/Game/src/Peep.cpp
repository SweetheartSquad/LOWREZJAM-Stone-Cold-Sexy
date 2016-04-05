#pragma once

#include <Peep.h>
#include <MY_ResourceManager.h>

#include <Timeout.h>
#include <NumberUtils.h>

Peep::Peep(BulletWorld * _world) :
	NodeUI(_world),
	wantsTakePicture(false),
	walk(true)
{
	boxSizing = kCONTENT_BOX;

	background->mesh->setScaleMode(GL_NEAREST);
	background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("peep")->texture);
	
	picTexTimeout = new Timeout(0.5f, [this](sweet::Event * _event){
		background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("peep")->texture);
	});
	childTransform->addChild(picTexTimeout, false);
	picTexTimeout->name = "pic tex timeout";

	picTimeout1 = new Timeout(sweet::NumberUtils::randomFloat(3.5f, 10.f), [this](sweet::Event * _event){
		if(marginLeft.rationalSize > -0.5 && marginLeft.rationalSize < 0.5f){
			background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("peep-ready")->texture);
			picTimeout2->restart();
		}else{
			picTimeout1->restart();
		}
	});
	picTimeout1->start();
	childTransform->addChild(picTimeout1, false);
	picTimeout1->name = "pic 1 timeout";

	picTimeout2 = new Timeout(0.45f, [this](sweet::Event * _event){
		background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("peep-flash")->texture);
		wantsTakePicture = true;
		picTimeout1->restart();
		picTexTimeout->restart();
	});
	childTransform->addChild(picTimeout2, false);
	picTimeout2->name = "pic 2 timeout";

	walkTimeout = new Timeout(sweet::NumberUtils::randomFloat(0.1, 0.25f), [this](sweet::Event * _event){
		walkTimeout->restart();
		walk = true;
	});
	walkTimeout->start();
	childTransform->addChild(walkTimeout, false);
	walkTimeout->name = "walk timeout";

	speedTimeout = new Timeout(sweet::NumberUtils::randomFloat(0.5, 2.5f), [this](sweet::Event * _event){
		speedTimeout->restart();
		walkTimeout->targetSeconds = sweet::NumberUtils::randomFloat(0.1, 0.25f);
	});
	speedTimeout->start();
	childTransform->addChild(speedTimeout, false);
	speedTimeout->name = "speed timeout";


	// score
	plusOne = new NodeUI(world);
	addChild(plusOne);
	plusOne->setRationalWidth(1.f, this);
	plusOne->setRationalHeight(1.f, this);
	plusOne->marginBottom.setRationalSize(0.f, &this->height);
	plusOne->setVisible(false);
	plusOne->boxSizing = kCONTENT_BOX;
	plusOne->background->mesh->setScaleMode(GL_NEAREST);
	plusOne->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("plus")->texture);

	scoreTimeout = new Timeout(0.75f, [this](sweet::Event * _event){
		plusOne->setVisible(false);
	});
	scoreTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		plusOne->marginBottom.rationalSize += 0.1f/64.f;
		plusOne->autoResize();
		plusOne->setBackgroundColour(1,1,1, 1.f - _event->getFloatData("progress"));
	});
	scoreTimeout->eventManager->addEventListener("start", [this](sweet::Event * _event){
		plusOne->marginBottom.rationalSize = 0;
		plusOne->setVisible(true);
	});
	childTransform->addChild(scoreTimeout, false);
}

void Peep::update(Step * _step){
	NodeUI::update(_step);
}

void Peep::takePicture(){
	MY_ResourceManager::globalAssets->getAudio("flash")->sound->play();
	wantsTakePicture = false;
}