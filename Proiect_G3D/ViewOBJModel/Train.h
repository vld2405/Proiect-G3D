#pragma once

#include "Model.h"

class Train : public Model
{
public:

	Train(string const& path, bool bSmoothNormals, bool gamma = false);
	virtual void Draw(Shader& shader);

private:
};

