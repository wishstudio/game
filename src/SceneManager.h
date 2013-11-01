#pragma once

class SceneManager
{
public:
	SceneManager();
	virtual ~SceneManager();

	void update();

private:
	vector3di lastPosition;
};
