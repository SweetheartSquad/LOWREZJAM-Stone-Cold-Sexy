#pragma once

#include <MY_Scene_Base.h>

class RenderSurface;
class StandardFrameBuffer;
class ShaderComponentUvOffset;
class ShaderComponentHsv;
class Peep;
class TextLabelControlled;

#define NUM_POSES 3
#define NUM_POSE_UIS 1

class MY_Scene_Main : public MY_Scene_Base{
public:
	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;
	Timeout * peepTimeout, * poseTimeout;

	TextLabelControlled * scoreLabel;
	NodeUI * poser;
	NodeUI * fg;
	NodeUI * bg;
	std::vector<Peep * > peeps;

	std::vector<MeshInterface *> clouds;

	float confidence;
	bool posing;
	float score;
	float whiteout;
	float redout;
	bool ready;

	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;

	virtual void unload() override;
	virtual void load() override;

	MY_Scene_Main(Game * _game);
	~MY_Scene_Main();

	void addPeep();
};