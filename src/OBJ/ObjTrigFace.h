#pragma once
class ObjTrigFace
{
public:
	ObjTrigFace(unsigned int index1, unsigned int index2, unsigned int index3)
	{
		m_vertIndex[0] = index1;
		m_vertIndex[1] = index2;
		m_vertIndex[2] = index3;
	}
	unsigned int GetIndexOfVertex(int i) const
	{
		return m_vertIndex[i];
	}
	void SetIndexOfVertex(int i, unsigned int index)
	{
		m_vertIndex[i] = index;
	}
private:
	unsigned int m_vertIndex[3];

};

