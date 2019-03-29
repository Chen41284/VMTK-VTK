#pragma once
/*
* ���ߣ��¼���
* ���ڣ�2019-01-13
* ���ܣ�ʵ�� Mesh Saliency���ĵĹ��� ���ĵ�ַ��https://dl.acm.org/citation.cfm?id=1073244
* ���룺ComputeSaliency   �ı���Yupan Liu Github : https://github.com/climberpi/Mesh-Saliency
        ComputeSaliency2  �ı���rohan-sawhney Github:https://github.com/rohan-sawhney/mesh-saliency
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <float.h>
#include <algorithm>
#include <functional>
#include <math.h>
#include <vector>
#include <queue>
#include <thread>

struct vec3;
struct mat3;
struct mat2;

class MeshSaliency
{
public:
	/*���붥��������������ڵ��ļ���ַ*/
	explicit MeshSaliency(std::string filePath);
	explicit MeshSaliency(const char* plyfilePath);
	~MeshSaliency();
	void ComputeSaliency();
	void ComputeSaliency_Lee();
	void ComputeSaliency_OneLevel();
	void ComputeSaliency_MultiCore();
	void ComputeSaliency2();
	void setFaceReduceOne();
	void WriteSaliencyPoints(const char* filepath);
	void WriteLevelPoints(std::string filePath[]);
	void WriteLocalMaxPoints(std::string filePath[]);
	void WriteOneLevel(const char* SaliencyPath, const char* LocalMaxPath);
	void WriteOneLevel(const char* SaliencyAndLocalMaxPath);
	inline float* getNormal() { return normals; };
	inline float* getMeanCurvature() { return meanCurvature; };
	inline float* getSmoothSaliency() { return smoothSaliency; };
	inline float** getSaliency() { return Saliency; };
	inline float* getPoints() { return Points; };
	inline int* getFaces() { return Faces; };
	inline int getVertexNumber() { return vertexCnt; };
	inline int getFaceNumber() { return faceCnt; };
	void Print();
private:
	float* normals;             //����ķ���
	float* meanCurvature;       //�����ƽ������
	float* smoothSaliency;      //�����������
	float* Saliency[7];         //����Ĳ�ͬˮƽ��������, Saliency[0], Saliency[1]����
	float* Points;              //����Ķ�������
	int* Faces;                 //������������
	int vertexCnt;              //���������
	int faceCnt;                //�������
	float xMin, yMin, zMin;     //��С�Ķ�������
	float xMax, yMax, zMax;     //���Ķ�������
	float SmoothFactor[7];      //ƽ��ϵ��
	bool* LocalMax[5];          //�ֲ����ֵ�� , �ж��ǲ������ֵ�㼴��

	/*��Ⱦ�ܵ����㺯��*/
	/*climberpi �Ĵ������һ,�㷨����û��*/
	void ComputeNormals();
	void ComputeMeanCurvature();
	void ComputeSupressedSaliency();

	/*climberpi �Ĵ�����Ķ�,����ԭ�������޸�*/
	float ComputeGiven_Curvature(mat2 &Mvi);
	float Curvature(float&a, float&b, float& c, float d);
	void ComputeNormals_Taubin();
	void ComputeMeanCurvature_Taubin();
	void ComputeSaliency_Taubin();
	void Saliency_MultiCore();
	static void LevelSaliency(int level, float Epsilon, int vertexCnt, bool *localMax,
		const float *Points, const float *meanCurvature, float* Saliency, float *SmoothFactor);
	static void LevelSaliencyArray(int level, float Epsilon, int vertexCnt, bool *localMax,
		const float *Points, const float *meanCurvature, float* Saliency, float *SmoothFactor);

	/*rohan-sawhney ����ĸ���*/
	void ComputeMeanCurvature2();
	float dualArea();
};

struct vec3 {
	vec3();
	vec3(float x, float y, float z);
	vec3  operator+  (const vec3& rhs);
	vec3  operator+  (float rhs);
	vec3& operator+= (const vec3& rhs);
	vec3  operator-  (const vec3& rhs);
	vec3  operator-  (float rhs);
	vec3& operator-= (const vec3& rhs);
	vec3  operator*  (float rhs);
	vec3& operator*= (float rhs);
	vec3  operator/  (float rhs);
	vec3& operator=  (const vec3& rhs);
	float v[3];
};


/* stored like this:
a d g
b e h
c f i */
struct mat3 {
	mat3();
	mat3(float a, float b, float c,
		float d, float e, float f,
		float g, float h, float i);
	vec3 operator* (const vec3& rhs);
	mat3 operator* (const mat3& rhs); 
	mat3 operator* (const float& rhs); 
	mat3 operator+ (const mat3& rhs);
	mat3 operator- (const mat3& rhs); 
	mat3& operator= (const mat3& rhs); 
	float m[9];
};

/* ���׾���Ĵ洢
a c
b d
 */
struct mat2 {
	mat2();
	mat2(float a, float b,
		float c,float d);
	mat2 operator* (const mat2& rhs);
	mat2 operator* (const float& rhs);
	mat2 operator+ (const mat2& rhs);
	mat2 operator- (const mat2& rhs);
	mat2& operator= (const mat2& rhs);
	float m[4];
};


mat3 identity_mat3();
float length(const vec3& v);
vec3 cross(const vec3& a, const vec3& b);
mat3 wedge(const vec3& a, const vec3& b);
vec3 normalise(const vec3& v);
float dot(const vec3& a, const vec3& b);
float get_squared_dist(vec3 from, vec3 to);
float get_Euc_dist(vec3 from, vec3 to);
mat3 transpose(const mat3& mm);