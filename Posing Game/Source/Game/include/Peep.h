#pragma once

#include <NodeUI.h>

class Timeout;

class Peep : public NodeUI{
public:

	Timeout * walkTimeout, * picTexTimeout, * picTimeout1, * picTimeout2, * speedTimeout, * scoreTimeout;

	NodeUI * plusOne;

	bool walk;
	bool wantsTakePicture;

	Peep(BulletWorld * _world);

	virtual void update(Step * _step) override;

	void takePicture();
};