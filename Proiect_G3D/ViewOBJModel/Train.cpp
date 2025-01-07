#include "Train.h"

Train::Train(string const& path, bool bSmoothNormals, bool gamma) :
    Model(path, bSmoothNormals, gamma)
{}

void Train::Draw(Shader& shader)
{
    m_model.Draw(shader);
}