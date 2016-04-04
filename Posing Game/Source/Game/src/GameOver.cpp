#pragma once

#include <GameOver.h>
#include <shader/ComponentShaderText.h>
#include <sweet/UI.h>

#include <MY_Scene_Menu.h>

GameOver::GameOver(Game * _game, float _score) :
	MY_Scene_Base(_game),
	done(false)
{
	std::string pic = "ENDING_";

	if(_score > 500){
		pic += "5";
	}else if(_score > 500){
		pic += "4";
	}else if(_score > 500){
		pic += "3";
	}else if(_score > 500){
		pic += "2";
	}else if(_score > 500){
		pic += "1";
	}


	{
	VerticalLinearLayout * vl = new VerticalLinearLayout(uiLayer->world);
	uiLayer->addChild(vl);
	vl->setRationalHeight(1.f, uiLayer);
	vl->setRationalWidth(1.f, uiLayer);
	vl->horizontalAlignment = kCENTER;
	vl->verticalAlignment = kMIDDLE;
	NodeUI * menu = new NodeUI(uiLayer->world);
	vl->addChild(menu);
	menu->setRationalHeight(1.f, vl);
	menu->setSquareWidth(1.f);
	menu->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture(pic)->texture);
	menu->background->mesh->setScaleMode(GL_NEAREST);
	}

	Timeout * doneTimeout = new Timeout(0.5f, [this](sweet::Event * _event){
		done = true;
	});
	doneTimeout->start();
	childTransform->addChild(doneTimeout, false);
}

void GameOver::update(Step * _step){
	if(done){
		if(mouse->leftJustPressed()){
			dynamic_cast<MY_Scene_Menu *>(game->scenes["menu"])->readyTimeout->restart();
			dynamic_cast<MY_Scene_Menu *>(game->scenes["menu"])->ready = false;
			game->switchScene("menu", true);
		}
	}

	MY_Scene_Base::update(_step);
}