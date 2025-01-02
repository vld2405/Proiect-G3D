#ifndef CVERTEXBUFFEROBJECT_H
#define CVERTEXBUFFEROBJECT_H

#include <GL/glew.h>

class CVertexBufferObject {
public:
    void createVBO();
    void bindVBO();
    void addData(void* data, unsigned int dataSize);
    void uploadDataToGPU(GLenum usageHint);
    void releaseVBO();

private:
    GLuint vboID;
    unsigned int dataSize;
};

#endif
