#include <OBJ/ObjTrigFace.h>
#include <iostream>

void ObjTrigFace::PrintFaceIndices(unsigned int faceIndex) const
{
	std::cout << "Face " << faceIndex << ": "
		<< m_vertIndex[0] << ", "
		<< m_vertIndex[1] << ", "
		<< m_vertIndex[2] << std::endl;
}
