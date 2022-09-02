#pragma once

struct Effect
{
	virtual bool Init() = 0;
	virtual bool Update(float time) = 0;
	virtual bool HandleEvent(const SDL_Event* ev, float time) = 0;
	virtual void Render() = 0;
	virtual ~Effect() {}
};
