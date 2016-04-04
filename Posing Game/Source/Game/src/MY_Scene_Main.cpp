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

MY_Scene_Main::MY_Scene_Main(Game * _game) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface_1", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	confidence(100)
{
	// memory management
	screenSurface->incrementReferenceCount();
	screenSurfaceShader->incrementReferenceCount();
	screenFBO->incrementReferenceCount();

	screenSurface->setScaleMode(GL_NEAREST);

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
	
	bg->background->mesh->setScaleMode(GL_NEAREST);
	poser->background->mesh->setScaleMode(GL_NEAREST);
	//fg->background->mesh->setScaleMode(GL_NEAREST);
	
	bg->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("bg")->texture);
	poser->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("poser")->texture);
	//fg->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("fg")->texture);
	fg->background->setVisible(false);


	// ui
	SliderControlled * confidenceSlider = new SliderControlled(uiLayer->world, &confidence, 0, 100);
	uiLayer->addChild(confidenceSlider, false);
	confidenceSlider->setRationalHeight(2/64.f, uiLayer);
	confidenceSlider->setRationalWidth(60/64.f, uiLayer);
	confidenceSlider->setMargin(2/64.f);
	confidenceSlider->boxSizing = kCONTENT_BOX;
	confidenceSlider->thumb->setVisible(false);
	
	uiLayer->invalidateLayout();


	peepTimeout = new Timeout(5.f, [this](sweet::Event * _event){
		peepTimeout->restart();
		addPeep();
	});
	peepTimeout->start();
	childTransform->addChild(peepTimeout, false);
}

MY_Scene_Main::~MY_Scene_Main(){
	// memory management
	screenSurface->decrementAndDelete();
	screenSurfaceShader->decrementAndDelete();
	screenFBO->decrementAndDelete();
}


void MY_Scene_Main::update(Step * _step){
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
	}






	posing = mouse->leftDown();

	if(posing){
		confidence -= 0.1f;
		// just started posing
		if(mouse->leftJustPressed()){
			poser->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("poser-posing")->texture);
		}
	}else{
		// just finished posing
		if(mouse->leftJustReleased()){
			poser->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("poser")->texture);
		}
	}

	// update peeps
	if(keyboard->keyJustDown(GLFW_KEY_P)){
		addPeep();
	}
	
	for(signed long int i = peeps.size()-1; i >= 0; --i){
		Peep * p = peeps.at(i);
		if(p->walk){
			p->marginLeft.rationalSize += 1/64.f;
			p->autoResize();
			p->walk = false;
		}

		if(p->takePicture){
			p->takePicture = false;
		}


		// remove peeps off screen
		if(p->marginLeft.rationalSize >= 1.f){
			fg->removeChild(p);
			delete p;
			peeps.erase(peeps.begin() + i);
		}
	}


	// lose state
	if(confidence <= FLT_EPSILON){
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