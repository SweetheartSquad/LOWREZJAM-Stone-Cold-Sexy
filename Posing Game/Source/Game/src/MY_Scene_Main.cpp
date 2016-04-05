#pragma once

#include <MY_Scene_Main.h>
#include <RenderSurface.h>
#include <StandardFrameBuffer.h>
#include <RenderOptions.h>


#include <shader\ShaderComponentTexture.h>
#include <shader\ShaderComponentDiffuse.h>
#include <shader\ShaderComponentMVP.h>
#include <shader\ShaderComponentUvOffset.h>
#include <shader\ShaderComponentHsv.h>

#include <Peep.h>


#include <sweet\UI.h>
#include <NumberUtils.h>
#include <MY_Game.h>
#include <GameOver.h>
#include <Easing.h>

MY_Scene_Main::MY_Scene_Main(Game * _game) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface_1", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	confidence(100),
	score(0),
	whiteout(0),
	redout(0),
	ready(false),
	posing(false)
{
	// memory management
	screenSurface->incrementReferenceCount();
	screenSurfaceShader->incrementReferenceCount();
	screenFBO->incrementReferenceCount();
	
	screenSurface->setScaleMode(GL_NEAREST);
	screenSurface->uvEdgeMode = GL_CLAMP_TO_BORDER;

	// game
	bg = new NodeUI(uiLayer->world);
	poser = new NodeUI(uiLayer->world);
	fg = new NodeUI(uiLayer->world);
	
	bg->setRationalHeight(1.f, uiLayer);
	poser->setRationalHeight(1.f, uiLayer);
	fg->setRationalHeight(1.f, uiLayer);
	
	bg->setRationalWidth(1.f, uiLayer);
	poser->setRationalWidth(1.f, uiLayer);
	fg->setRationalWidth(1.f, uiLayer);
	
	uiLayer->addChild(bg,false);
	uiLayer->addChild(poser,false);
	uiLayer->addChild(fg,false);
	
	bg->background->setVisible(false);
	poser->background->mesh->setScaleMode(GL_NEAREST);
	//fg->background->mesh->setScaleMode(GL_NEAREST);
	
	bg->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("bg")->texture);
	poser->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("poser")->texture);
	//fg->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("fg")->texture);
	fg->background->setVisible(false);

	for(unsigned long int i = 1; i <= 3; ++i){
		NodeUI * cloud = new NodeUI(uiLayer->world);
		bg->addChild(cloud);
		cloud->setRationalHeight(1.f, bg);
		cloud->setRationalWidth(1.f, bg);
		cloud->background->mesh->setScaleMode(GL_NEAREST);
		cloud->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("cloud_" + std::to_string(i))->texture);

		clouds.push_back(cloud->background->mesh);
	}{
		NodeUI * bg = new NodeUI(uiLayer->world);
		this->bg->addChild(bg);
		bg->setRationalHeight(1.f, this->bg);
		bg->setRationalWidth(1.f, this->bg);
		bg->background->mesh->setScaleMode(GL_NEAREST);
		bg->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("bg")->texture);
	}

	// ui

	NodeUI * poseNode = new NodeUI(uiLayer->world);
	uiLayer->addChild(poseNode);
	poseNode->setRationalHeight(1.f, uiLayer);
	poseNode->setRationalWidth(1.f, uiLayer);
	poseNode->background->mesh->setScaleMode(GL_NEAREST);
	poseNode->setBackgroundColour(1,1,1, 0);
	poseTimeout = new Timeout(0.5, [this, poseNode](sweet::Event * _event){
		poseNode->setBackgroundColour(1,1,1,0);
	});
	poseTimeout->eventManager->addEventListener("progress", [this, poseNode](sweet::Event * _event){
		poseNode->setBackgroundColour(1,1,1, Easing::easeInCirc(_event->getFloatData("progress"), 1, -1, 1));
	});
	poseTimeout->eventManager->addEventListener("start", [this, poseNode](sweet::Event * _event){
		poseNode->setBackgroundColour(1,1,1,1);
		poseNode->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("uiPose_" + std::to_string(sweet::NumberUtils::randomInt(1,NUM_POSE_UIS)))->texture);
	});
	poseTimeout->name = "pose timeout";
	childTransform->addChild(poseTimeout, false);

	SliderControlled * confidenceSlider = new SliderControlled(uiLayer->world, &confidence, 0, 100, false);
	uiLayer->addChild(confidenceSlider, false);
	confidenceSlider->setRationalHeight(60/64.f, uiLayer);
	confidenceSlider->setRationalWidth(2/64.f, uiLayer);
	confidenceSlider->setMargin(2/64.f);
	confidenceSlider->boxSizing = kCONTENT_BOX;
	confidenceSlider->thumb->setVisible(false);
	confidenceSlider->fill->background->mesh->setScaleMode(GL_NEAREST);
	confidenceSlider->background->mesh->setScaleMode(GL_NEAREST);
	confidenceSlider->fill->setBackgroundColour(160/255.f * 1.25f, 72/255.f * 1.25f, 92/255.f * 1.25f);
	confidenceSlider->setBackgroundColour(160/255.f * 0.75f, 72/255.f * 0.75f, 92/255.f * 0.75f);

	uiLayer->invalidateLayout();


	peepTimeout = new Timeout(5.f, [this](sweet::Event * _event){
		peepTimeout->restart();
		addPeep();
	});
	peepTimeout->trigger();
	peepTimeout->start();
	childTransform->addChild(peepTimeout, false);
	peepTimeout->name = "peep timeout";

	scoreLabel = new TextLabelControlled(&score, 0.f, FLT_MAX, uiLayer->world, MY_ResourceManager::globalAssets->getFont("font")->font, textShader);
	uiLayer->addChild(scoreLabel);
	scoreLabel->setMarginLeft(5/64.f);
	scoreLabel->setMarginBottom(52/64.f);
	scoreLabel->boxSizing = kCONTENT_BOX;
	scoreLabel->setRationalHeight(1.f, uiLayer);
	scoreLabel->setRationalWidth(1.f, uiLayer);
}

MY_Scene_Main::~MY_Scene_Main(){
	// memory management
	screenSurface->decrementAndDelete();
	screenSurfaceShader->decrementAndDelete();
	screenFBO->decrementAndDelete();
}


void MY_Scene_Main::update(Step * _step){
	float g = MY_ResourceManager::globalAssets->getAudio("BGM2")->sound->getGain(false) + (posing ? 0.05f : -0.05f);
	MY_ResourceManager::globalAssets->getAudio("BGM2")->sound->setGain(glm::clamp(g, 0.25f, 1.f));

	uiLayer->resize(0, 64, 0, 64);
	// Screen shader update
	// Screen shaders are typically loaded from a file instead of built using components, so to update their uniforms
	// we need to use the OpenGL API calls
	screenSurfaceShader->bindShader(); // remember that we have to bind the shader before it can be updated
	GLint test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "time");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, _step->time);
		checkForGlError(0);
	}test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "whiteout");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, whiteout);
		checkForGlError(0);
	}test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "redout");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, redout);
		checkForGlError(0);
	}


	
	for(unsigned long int i = 0; i < clouds.size(); ++i){
		if(_step->cycles % (30+i*5) == 0){
			MeshInterface * c = clouds.at(i);
			for(auto & v : c->vertices){
				v.u += 1/64.f;
			}
			c->dirty = true;
		}
	}

	if(MY_Game::resized){
		MY_ResourceManager::globalAssets->getFont("font")->font->resize(18);
		scoreLabel->invalidate();
		scoreLabel->updateText();
		scoreLabel->invalidateLayout();
	}
	//scoreLabel->setFont(MY_ResourceManager::globalAssets->getFont("font")->font, true);



	if(ready){
		if(mouse->leftJustPressed() || keyboard->keyJustDown(GLFW_KEY_SPACE)){
			// just started posing
			poser->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("poser-posing_" + std::to_string(sweet::NumberUtils::randomInt(1,NUM_POSES)))->texture);
			MY_ResourceManager::globalAssets->getAudio("in")->sound->setGain(0.5f);
			MY_ResourceManager::globalAssets->getAudio("in")->sound->play();
			redout = 1;
			posing = true;
		}else if(mouse->leftJustReleased() || keyboard->keyJustUp(GLFW_KEY_SPACE)){
			// just finished posing
			poser->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("poser")->texture);
			MY_ResourceManager::globalAssets->getAudio("out")->sound->setGain(0.5f);
			MY_ResourceManager::globalAssets->getAudio("out")->sound->play();
			redout = -1;
			posing = false;
		}
	}else{
		ready = mouse->leftJustReleased();
	}
	if(posing){
		confidence -= 0.1f + (score/5000);
	}else{
		confidence -= 0.01f + (score/5000);
	}

	// update peeps
	if(keyboard->keyJustDown(GLFW_KEY_P)){
		addPeep();
	}
	if(keyboard->keyJustDown(GLFW_KEY_L)){
		screenSurfaceShader->unload();
		screenSurfaceShader->loadFromFile(screenSurfaceShader->vertSource, screenSurfaceShader->fragSource);
		screenSurfaceShader->load();
	}
	
	for(signed long int i = peeps.size()-1; i >= 0; --i){
		Peep * p = peeps.at(i);
		if(p->walk){
			p->marginLeft.rationalSize += 1/64.f;
			p->autoResize();
			p->walk = false;
		}

		// take picture
		if(p->wantsTakePicture){
			p->takePicture();
			whiteout = 1.f;
			if(posing){
				confidence += 10;
				score += 10;
				p->scoreTimeout->restart();
				poseTimeout->restart();
				MY_ResourceManager::globalAssets->getAudio("flash")->sound->play();
			}else{
				MY_ResourceManager::globalAssets->getAudio("flash-wrong")->sound->play();
			}
		}


		// remove peeps off screen
		if(p->marginLeft.rationalSize >= 1.f){
			fg->removeChild(p);
			delete p;
			peeps.erase(peeps.begin() + i);
		}
	}
	
	confidence = glm::clamp(confidence, 0.f, 100.f);
	whiteout += -whiteout*0.25f;
	redout += -redout*0.25f;
	if(glm::abs(whiteout) < 0.1){
		whiteout = 0;
	}if(glm::abs(redout) < 0.1){
		redout = 0;
	}

	// lose state
	if(confidence <= FLT_EPSILON){
		game->scenes["gameover"] = new GameOver(game, score);
		game->switchScene("gameover", true);
	}





	// Scene update
	MY_Scene_Base::update(_step);
}

void MY_Scene_Main::render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions){
	glm::uvec2 sd = sweet::getWindowDimensions();
	int max = glm::max(sd.x, sd.y);
	int min = glm::min(sd.x, sd.y);
	bool horz = sd.x == max;
	int offset = (max - min)/2;

	// keep our screen framebuffer up-to-date with the current viewport
	screenFBO->resize(64, 64);
	_renderOptions->setViewPort(0,0,64,64);
	_renderOptions->setClearColour(1,0,1,0);

	// bind our screen framebuffer
	FrameBufferInterface::pushFbo(screenFBO);
	// render the scene
	MY_Scene_Base::render(_matrixStack, _renderOptions);
	// unbind our screen framebuffer, rebinding the previously bound framebuffer
	// since we didn't have one bound before, this will be the default framebuffer (i.e. the one visible to the player)
	FrameBufferInterface::popFbo();

	// render our screen framebuffer using the standard render surface
	_renderOptions->setViewPort(horz ? offset : 0, horz ? 0 : offset, min, min);
	screenSurface->render(screenFBO->getTextureId());
}

void MY_Scene_Main::load(){
	MY_Scene_Base::load();	

	screenSurface->load();
	screenFBO->load();
}

void MY_Scene_Main::unload(){
	screenFBO->unload();
	screenSurface->unload();

	MY_Scene_Base::unload();	
}





void MY_Scene_Main::addPeep(){
	Peep * peep = new Peep(uiLayer->world);
	fg->addChild(peep);
	peep->setRationalHeight(64/64.f, uiLayer);
	peep->setRationalWidth(64/64.f, uiLayer);
	
	peep->marginLeft.setRationalSize(-64/64.f, &fg->width);
	peep->marginBottom.setRationalSize(sweet::NumberUtils::randomInt(-12, 0)/64.f, &fg->height);

	peeps.push_back(peep);
}