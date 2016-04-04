#pragma once

#include <Game.h>

class MY_Game : public Game{
public:
	static bool resized;

	MY_Game();
	~MY_Game();

	void addSplashes() override;

	virtual void update(Step * _step) override;
};