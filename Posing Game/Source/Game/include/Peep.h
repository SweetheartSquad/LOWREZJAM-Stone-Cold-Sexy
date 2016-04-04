#pragma once

#include <NodeUI.h>

class Timeout;

class Peep : public NodeUI{
public:

	Timeout * walkTimeout, * picTexTimeout, * picTimeout, * speedTimeout;

	bool walk;
	bool takePicture;

	Peep(BulletWorld * _world);

	virtual void update(Step * _step) override;
};