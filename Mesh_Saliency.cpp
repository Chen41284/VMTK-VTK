#include "Mesh_Saliency.h"
#include <vtkPLYReader.h>

/*
 * ���붥�����������
 * ��ʽ��
 *   XXX -- ���������������
 *   XXX -- �������������
 *   A B C -- ��������꣬������
 *   ......
 *   D E F -- ÿ����Ķ�����
 * ע�������Ķ����Ŵ�0��ʼ
 */ 
MeshSaliency::MeshSaliency(std::string filePath)
{
	std::ifstream infile(filePath.c_str());
	infile >> vertexCnt;  //���������
	infile >> faceCnt;    //�������
	xMin = FLT_MAX, yMin = FLT_MAX, zMin = FLT_MAX;
	xMax = FLT_MIN, yMax = FLT_MIN, zMax = FLT_MIN;
	Points = new float[3 * vertexCnt];
	Faces = new int[3 * faceCnt];
	normals = new float[3 * vertexCnt];
	/*���㷨�ߵĳ�ʼ��*/
	for (int i = 0; i < 3 * vertexCnt; i++)
	{
		normals[i] = 0.0f;
	}
	/*��ͬ��ˮƽ��������*/
	for (int i = 0; i < 7; i++)
	{
		Saliency[i] = new float[vertexCnt];
		SmoothFactor[i] = 0.0f;
	}
	/*��ͬˮƽ�ȵľֲ����ֵ��*/
	for (int i = 0; i < 5; i++)
	{
		LocalMax[i] = new bool[vertexCnt];
	}
	meanCurvature = new float[vertexCnt];
	smoothSaliency = new float[vertexCnt];
	/*���ʺ������Եĳ�ʼ��*/
	for (int i = 0; i < vertexCnt; i++)
	{
		meanCurvature[i] = 0.0f;
		smoothSaliency[i] = 0.0f;
		Saliency[0][i] = 0.0f, Saliency[1][i] = 0.0f, Saliency[2][i] = 0.0f;
		Saliency[3][i] = 0.0f, Saliency[4][i] = 0.0f, Saliency[5][i] = 0.0f;
		Saliency[6][i] = 0.0f;
		LocalMax[0][i] = LocalMax[1][i] = LocalMax[2][i] = LocalMax[3][i] = LocalMax[4][i] = false;
	}
	/*���붥������*/
	for (int i = 0; i < vertexCnt; i++)
	{
		infile >> Points[i * 3] >> Points[i * 3 + 1] >> Points[i * 3 + 2]; 
		xMin = fmin(xMin, Points[i * 3]), xMax = fmax(xMax, Points[i * 3]);
		yMin = fmin(yMin, Points[i * 3 + 1]), yMax = fmax(yMax, Points[i * 3 + 1]);
		zMin = fmin(zMin, Points[i * 3 + 2]), zMax = fmax(zMax, Points[i * 3 + 2]);
	}
	/*�����������*/
	for (int i = 0; i < faceCnt; i++)
	{
		infile >> Faces[i * 3] >> Faces[i * 3 + 1] >> Faces[i * 3 + 2];
	}

	
	infile.close();
}

/*
 * ����PLY�ļ�
 */
MeshSaliency::MeshSaliency(const char* plyfilePath)
{
	vtkSmartPointer<vtkPLYReader> reader =
		vtkSmartPointer<vtkPLYReader>::New();
	reader->SetFileName(plyfilePath);
	reader->Update();
	vtkSmartPointer<vtkPolyData> polydata = reader->GetOutput();
	vertexCnt = polydata->GetNumberOfPoints();  //���������
	faceCnt = polydata->GetNumberOfCells();    //�������
	xMin = FLT_MAX, yMin = FLT_MAX, zMin = FLT_MAX;
	xMax = FLT_MIN, yMax = FLT_MIN, zMax = FLT_MIN;
	Points = new float[3 * vertexCnt];
	Faces = new int[3 * faceCnt];
	normals = new float[3 * vertexCnt];
	/*���㷨�ߵĳ�ʼ��*/
	for (int i = 0; i < 3 * vertexCnt; i++)
	{
		normals[i] = 0.0f;
	}
	/*��ͬ��ˮƽ��������*/
	for (int i = 0; i < 7; i++)
	{
		Saliency[i] = new float[vertexCnt];
		SmoothFactor[i] = 0.0f;
	}
	/*��ͬˮƽ�ȵľֲ����ֵ��*/
	for (int i = 0; i < 5; i++)
	{
		LocalMax[i] = new bool[vertexCnt];
	}
	meanCurvature = new float[vertexCnt];
	smoothSaliency = new float[vertexCnt];
	/*���ʺ������Եĳ�ʼ��*/
	for (int i = 0; i < vertexCnt; i++)
	{
		meanCurvature[i] = 0.0f;
		smoothSaliency[i] = 0.0f;
		Saliency[0][i] = 0.0f, Saliency[1][i] = 0.0f, Saliency[2][i] = 0.0f;
		Saliency[3][i] = 0.0f, Saliency[4][i] = 0.0f, Saliency[5][i] = 0.0f;
		Saliency[6][i] = 0.0f;
		LocalMax[0][i] = LocalMax[1][i] = LocalMax[2][i] = LocalMax[3][i] = LocalMax[4][i] = false;
	}
	for (vtkIdType i = 0; i < polydata->GetNumberOfPoints(); i++)
	{
		double p[3];
		polydata->GetPoint(i, p);
		Points[i * 3] = p[0]; Points[i * 3 + 1] = p[1]; Points[i * 3 + 2] = p[2];
		xMin = fmin(xMin, Points[i * 3]), xMax = fmax(xMax, Points[i * 3]);
		yMin = fmin(yMin, Points[i * 3 + 1]), yMax = fmax(yMax, Points[i * 3 + 1]);
		zMin = fmin(zMin, Points[i * 3 + 2]), zMax = fmax(zMax, Points[i * 3 + 2]);
	}
	for (vtkIdType i = 0; i < polydata->GetNumberOfCells(); i++)
	{
		vtkSmartPointer<vtkCell> cell = polydata->GetCell(i);
		Faces[i * 3] = cell->GetPointId(0);
		Faces[i * 3 + 1] = cell->GetPointId(1);
		Faces[i * 3 + 2] = cell->GetPointId(2);
	}
}

/*
 * ����������ݼ�1, ���еĶ����ž���0��ʼ/
 */
void MeshSaliency::setFaceReduceOne()
{
	for (int i = 0; i < faceCnt; i++)
	{	
			Faces[i * 3]--;
			Faces[i * 3 + 1]--;
			Faces[i * 3 + 2]--;
	}
}

/*
 *�����������ͷ��ڴ�ռ�
 */
MeshSaliency::~MeshSaliency()
{
	delete[] normals;             
	delete[] meanCurvature;     
	delete[] smoothSaliency; 
	for (int i = 0; i < 7; i++)
		delete[] Saliency[i];
	for (int i = 0; i < 5; i++)
		delete[] LocalMax[i];
	delete[] Points;             
	delete[] Faces;              
}

/*
 * ������������ÿ�������������
 */
void MeshSaliency::ComputeSaliency()
{
	ComputeNormals();
	ComputeMeanCurvature();
	ComputeSupressedSaliency();
}


/*
 * ��������Ķ���ķ�������
 */
void MeshSaliency::ComputeNormals()
{
	/*����ÿ����ķ��ߣ�����ӵ���Ӧ������������*/
	for (int i = 0; i < faceCnt; i++)
	{
		int idx[3];
		for (int k = 0; k < 3; k++)
			idx[k] = Faces[i * 3 + k];
		float vA[3], vB[3], vC[3];
		for (int j = 0; j < 3; j++)
		{
			vA[j] = Points[idx[0] * 3 + j];
			vB[j] = Points[idx[1] * 3 + j];
			vC[j] = Points[idx[2] * 3 + j];
		}
		//��ıߵ�����
		vec3 faceVec1 = vec3(vB[0] - vA[0], vB[1] - vA[1], vB[2] - vA[2]);
		vec3 faceVec2 = vec3(vC[0] - vB[0], vC[1] - vB[1], vC[2] - vB[2]);
		vec3 crossProd = cross(faceVec1, faceVec2);
		/*ͬһ�������������ķ�����ͬ����Ҫ��Ӳ�ͬ��ķ���*/
		for (int k = 0; k < 3; k++)
		{
			normals[idx[k] * 3 + 0] += crossProd.v[0];
			normals[idx[k] * 3 + 1] += crossProd.v[1];
			normals[idx[k] * 3 + 2] += crossProd.v[2];
		}
	}

	/*��ÿ������ķ��߽��й�һ��*/
	for (int i = 0; i < vertexCnt; i++)
	{
		float norm = 0.0f;
		for (int k = 0; k < 3; k++)
			norm += normals[i * 3 + k] * normals[i * 3 + k];
		if (norm <= 0.0f)
		{
			std::cout << "vertex " << i << std::endl;
			printf("%13.6f %13.6f %13.6f\n", normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			continue;
		}
		for (int k = 0; k < 3; k++)
			normals[i * 3 + k] /= sqrt(norm);
	}

	/*����*/
	for (int i = 0; i < vertexCnt; i++)
	{
		if (isnan(normals[i * 3]) || isnan(normals[i * 3 + 1]) || isnan(normals[i * 3 + 2]))
		{
			printf("normals %d: %13.6f %13.6f %13.6f\n", i, normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
		}
	}
}

/*
 * ������������ĸ�˹����
 */
void MeshSaliency::ComputeMeanCurvature()
{
	/*����ÿ���������״����*/
	mat3* shapeOperators = NULL;
	float* vertexArea = NULL;
	shapeOperators = new mat3[vertexCnt + 1];
	vertexArea = new float[vertexCnt + 1];
	/*��ʼ��*/
	for (int i = 0; i < vertexCnt; i++)
	{
		vertexArea[i] = 0.0f;
		for (int j = 0; j < 9; j++)
			shapeOperators[i].m[j] = 0.0f;
	}
	/*����ÿ����ķ��ߣ�����ӵ���Ӧ������������*/
	for (int f = 0; f < faceCnt; f++)
	{
		int idx[3];
		for (int k = 0; k < 3; k++)
			idx[k] = Faces[f * 3 + k];
		float vA[3], vB[3], vC[3];
		for (int j = 0; j < 3; j++)
		{
			vA[j] = Points[idx[0] * 3 + j];
			vB[j] = Points[idx[1] * 3 + j];
			vC[j] = Points[idx[2] * 3 + j];
		}
		//ÿ��������
		vec3 faceVec1 = vec3(vB[0] - vA[0], vB[1] - vA[1], vB[2] - vA[2]);
		vec3 faceVec2 = vec3(vC[0] - vB[0], vC[1] - vB[1], vC[2] - vB[2]);
		vec3 vecArea = cross(faceVec1, faceVec2);
		float faceArea = sqrt(vecArea.v[0] * vecArea.v[0] +
			vecArea.v[1] * vecArea.v[1] +
			vecArea.v[2] * vecArea.v[2]);

		for (int t = 0; t < 3; t++)
		{
			int i = Faces[f * 3 + t];
			int j = Faces[f * 3 + (t + 1) % 3];  //ѭ��3����

			if (i < 0 || i > vertexCnt)
			{
				std::cout << "���� " << i << "��Χ���� " << std::endl;
			}
			if (j < 0 || j > vertexCnt)
			{
				std::cout << "���� " << j << "��Χ���� " << std::endl;
			}
			/*��ȡ����i�붥��j�ķ�������*/
			vec3 Ni = vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			vec3 Nj = vec3(normals[j * 3], normals[j * 3 + 1], normals[j * 3 + 2]);
			/*��ö���i�붥��j��λ��*/
			vec3 Vi = vec3(Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
			vec3 Vj = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);

			/*���ڶ���i��������״���ӵ���ز���*/
			vec3 Tij = (identity_mat3() - wedge(Ni, Ni) ) * (Vi - Vj);
			Tij = normalise(Tij);
			float kappa_ij = 2 * dot(Ni, Vj - Vi);
			if (isnan(kappa_ij))
			{
				std::cout << " face " << f << " " << std::endl;
			}
			if (get_squared_dist(Vi, Vj) == 0.0f)
			{
				std::cout << "face " << f << " " << std::endl;
				std::cout << "vertice " << i << " and " << j << std::endl;
			}
			kappa_ij = kappa_ij / get_squared_dist(Vi, Vj);
			/*ά������Vi����״����*/
			shapeOperators[i] = shapeOperators[i] + (wedge(Tij, Tij) * (kappa_ij * faceArea));
			vertexArea[i] += faceArea;

			/*���ڶ���j��������״���ӵ���ز���*/
			vec3 Tji = (identity_mat3() - wedge(Nj, Nj)) * (Vj - Vi);
			Tji = normalise(Tji);
			float kappa_ji = 2 * dot(Nj, Vi - Vj);
			if (isnan(kappa_ji))
			{
				std::cout << " face " << f << " " << std::endl;
			}
			kappa_ji /= get_squared_dist(Vi, Vj);
			/*ά������Vj����״����*/
			shapeOperators[j] = shapeOperators[j] + (wedge(Tji, Tji) * (kappa_ji * faceArea));

			vertexArea[j] += faceArea;

			/*if (isnan(kappa_ij) || isnan(kappa_ij))
			{
				std::cout << " face " << f << " " << std::endl;
			}*/
		}
	}

	
	for (int i = 0; i < vertexCnt; i++) {
		shapeOperators[i] = shapeOperators[i] * (1.0f / vertexArea[i]);
		for (int j = 0; j < 9; j++)
		{
			if (isnan(shapeOperators[i].m[j]))
			{
				std::cout << "vertex " << i << " shapeOperators " << "m " << j  << std::endl;
			}
		}
		
	}

	// �Խǻ���״�������󣬻��ƽ������
	meanCurvature = new float[vertexCnt];
	for (int k = 0; k < vertexCnt; k++) {
		vec3 E1 = vec3(1.0f, 0.0f, 0.0f);
		vec3 Nk = vec3(normals[k * 3], normals[k * 3 + 1], normals[k * 3 + 2]);
		bool isMinus = get_squared_dist(E1, Nk) > get_squared_dist(E1 * (-1.0f), Nk);
		vec3 Wk;
		// ͨ��Householder�任���жԽǻ�, ������Խǻ�
		if (!isMinus) // ��֤Wk������
			Wk = E1 + Nk;
		else
			Wk = E1 - Nk;
		Wk = normalise(Wk);  
		mat3 Qk = identity_mat3() - (wedge(Wk, Wk) * 2.0f);
		mat3 Mk = transpose(Qk) * shapeOperators[k] * Qk;
		/*ͨ��Mk����ļ�������ƽ������*/
		meanCurvature[k] = (Mk.m[4] + Mk.m[8]);
		///*test*/
		//std::cout << "vertex " << k << std::endl;
		//for (int i = 0; i < 3; i++)
		//{
		//	printf("%13.6f %13.6f %13.6f\n", Mk.m[i * 3], Mk.m[i * 3 + 1], Mk.m[i * 3 + 2]);
		//}
		if (isnan(meanCurvature[k]))
		{
			std::cout << "vertex " << k << " " << meanCurvature[k] << std::endl;
		}
	}

	
	delete[] shapeOperators;
	delete[] vertexArea;
}

/*
 * ������������Ķ����������
 */
void MeshSaliency::ComputeSupressedSaliency()
{
	// Calculate the incident matrix ( as linked list )
	int* first = NULL;    //��¼��㣬��incidentVertex�в���ʵ�ʵĶ���
	int* next = NULL;     //��¼6 * faces ���ӹ�ϵ
	int* incidentVertex = NULL;   // ��¼�����ӹ�ϵ��ʵ�ʶ���
	first = new int[vertexCnt + 1];
	for (int i = 0; i < vertexCnt; i++)
		first[i] = -1;
	next = new int[6 * faceCnt + 1];
	incidentVertex = new int[6 * faceCnt + 1];
	int edgeCnt = 0;
	for (int k = 0; k < faceCnt; k++) {
		int idx[3];
		for (int i = 0; i < 3; i++)
			idx[i] = Faces[k * 3 + i];
		/*�����Σ������ߣ�����6��*/
		for (int i = 0; i < 3; i++) {                      
			int j1 = idx[(i + 1) % 3], j2 = idx[(i + 2) % 3];
			//incidentVertex��¼���ӵĹ�ϵ
			incidentVertex[++edgeCnt] = j1;
			//next��¼������һ�����Ӷ��㣬-1�൱���յ㣨���ȼ�¼��
			next[edgeCnt] = first[idx[i]]; first[idx[i]] = edgeCnt;  
			incidentVertex[++edgeCnt] = j2;
			next[edgeCnt] = first[idx[i]]; first[idx[i]] = edgeCnt;
		}
	}

	// Calculate the mesh saliency by BFS
	float diagonalLength = sqrt((xMax - xMin)*(xMax - xMin) + (yMax - yMin)*(yMax - yMin) + (zMax - zMin)*(zMax - zMin));
	float sigma = 0.003 * diagonalLength;
	float maxSaliency[7];
	float minSaliency[7];
	for (int i = 0; i <= 6; i++) {
		maxSaliency[i] = FLT_MIN;
		minSaliency[i] = FLT_MAX;
	}

	// Labeled the vertecies whether covered or not.
	bool* used = NULL;
	used = new bool[vertexCnt];
	for (int k = 0; k < vertexCnt; k++) {   //��ÿ�����㶼���й�ȱ���
		// ��ʼ��Saliency�ĸ�˹������
		float gaussianSigma1[7], gaussianSigma2[7], sumSigma1[7], sumSigma2[7];
		for (int i = 0; i <= 6; i++)
			gaussianSigma1[i] = gaussianSigma2[i] = 0.0f,
			sumSigma1[i] = sumSigma2[i] = 0.0f;
		// ��ö������Ϣ
		vec3 vVec = vec3(Points[k * 3], Points[k * 3 + 1], Points[k * 3 + 2]);
		// ��ʼ�����У�Ѱ������
		for (int i = 0; i < vertexCnt; i++)
			used[i] = false;
		std::queue<int> Q;
		Q.push(k);
		used[k] = true;
		// Frsit BFS
		while (!Q.empty()) {
			// ��ö����еĵ�һ��Ԫ��
			int idx = Q.front(); Q.pop();
			if (idx < 0 || idx > vertexCnt)
			{
				std::cout << "idx ��Χ���� " << std::endl;
			}
			vec3 idxVec = vec3(Points[idx * 3], Points[idx * 3 + 1], Points[idx * 3 + 2]);
			// ��һ��ˮƽ�ȵĶ�����������
			for (int e = first[idx]; e != -1; e = next[e]) {
				int idxNext = incidentVertex[e];
				// ��չ�¸�ˮƽ�ȵĶ���
				if (!used[idxNext]) {
					vec3 idxNextVec = vec3(Points[idxNext * 3], Points[idxNext * 3 + 1], Points[idxNext * 3 + 2]);
					/*����ѡ�񣬲������򣬱���������ͨ�Ķ���*/
					if (get_squared_dist(vVec, idxNextVec) <= 36 * sigma*sigma)
						Q.push(incidentVertex[e]),
						used[incidentVertex[e]] = 1;
				}
			}
			// ���¸�˹������
			float dist = get_squared_dist(vVec, idxVec);
			for (int i = 2; i <= 6; i++) {
				float sigmaHere = i * i * sigma * sigma;
				if (dist <= sigmaHere) {
					float factor = exp(-dist / (2 * sigmaHere));
					gaussianSigma1[i] += meanCurvature[idx] * factor;
					sumSigma1[i] += factor;
				}
				if (dist <= 2 * sigmaHere) {
					float factor = exp(-dist / (8 * sigma * sigma));
					gaussianSigma2[i] += meanCurvature[idx] * factor;
					sumSigma2[i] += factor;
				}
			}
		}

		for (int i = 2; i <= 6; i++) {
			Saliency[i][k] = fabs(gaussianSigma1[i] / sumSigma1[i]
				- gaussianSigma2[i] / sumSigma2[i]);
			maxSaliency[i] = fmax(maxSaliency[i], Saliency[i][k]);
			minSaliency[i] = fmin(minSaliency[i], Saliency[i][k]);
		}
	}

	std::cout << "Each Level max Saliency " << std::endl;
	for (int i = 2; i < 7; i++)
	{
		std::cout << "Level" << i << "  " << maxSaliency[i] << std::endl;
	}
	
	float Range[7];
	Range[0] = Range[1] = 0;
	for (int i = 2; i < 7; i++)
	{
		Range[i] = maxSaliency[i] - minSaliency[i];
	}

	for (int v = 0; v < vertexCnt; v++)
	{
		for (int i = 2; i < 7; i++)
		{
			Saliency[i][v] = (Saliency[i][v] - minSaliency[i]) / Range[i];
		}
	}

	// Second BFS and get the non-linear normailization of suppressian's saliency.
	float MaxSmoothSaliency = FLT_MIN;
	for (int k = 0; k < vertexCnt; k++) {
		smoothSaliency[k] = 0.0f;
		float localMaxSaliency[7];
		for (int i = 0; i <= 6; i++)
			localMaxSaliency[i] = FLT_MIN;
		/*��ȡ���еĶ������Ϣ*/
		vec3 vVec = vec3(Points[k * 3], Points[k * 3 + 1], Points[k * 3 + 2]);
		// Initialize the queue to find neighbourhood.
		for (int i = 0; i < vertexCnt; i++)
			used[i] = false;
		std::queue<int> Q;
		Q.push(k);
		used[k] = true;
		while (!Q.empty()) {
			// Get the front element in the queue.
			int idx = Q.front(); Q.pop();;
			// Put the next level vertecies into the queue.
			for (int e = first[idx]; e != -1; e = next[e]) {
				int idxNext = incidentVertex[e];
				// Expand the next level vertecies.
				if (!used[idxNext]) {
					vec3 idxNextVec = vec3(Points[idxNext * 3], Points[idxNext * 3 + 1], Points[idxNext * 3 + 2]);
					if (get_squared_dist(vVec, idxNextVec) <= 36 * sigma*sigma)
						Q.push(incidentVertex[e]),
						used[incidentVertex[e]] = 1;
				}
			}
			// Update Gaussian filter
			for (int i = 2; i <= 6; i++)
				localMaxSaliency[i] = fmax(localMaxSaliency[i], Saliency[i][idx]);
		}
		// Calculate the weighted saliency
		float saliencySum = 0.0f;
		for (int i = 2; i <= 6; i++) {
			float factor = (maxSaliency[i] - localMaxSaliency[i]) * (maxSaliency[i] - localMaxSaliency[i]);
			smoothSaliency[k] += Saliency[i][k] * factor;
			saliencySum += factor;
		}
		if (saliencySum == 0.0f)
		{
			std::cout << "vertex " << k << saliencySum << std::endl;
			saliencySum = 1.0f;
		}
		smoothSaliency[k] /= saliencySum;

		MaxSmoothSaliency = fmax(smoothSaliency[k], MaxSmoothSaliency);
	}


	/*���*/
	for (int k = 0; k < vertexCnt; k++)
	{
		smoothSaliency[k] /= MaxSmoothSaliency;
		//std::cout << "vertex " << k << " " << smoothSaliency[k] << std::endl;
	}

	// Clean up resources
	delete[] first;
	delete[] next;
	delete[] incidentVertex;
}

/*
 * ����������������
 */
void MeshSaliency::Print()
{
	std::cout << "Vertice number: " << vertexCnt << std::endl;
	std::setprecision(6);
	std::cout << "-------------------------------------------------" << std::endl;
	for (int i = 0; i < vertexCnt; i++)
	{
		printf("%13.6f %13.6f %13.6f\n", Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
	}
	std::cout << std::endl;
	std::cout << "Faces number: " << faceCnt << std::endl;
	std::cout << "--------------------------------------------------" << std::endl;
	for (int i = 0; i < faceCnt; i++)
	{
		printf("%10d %10d %10d\n", Faces[i * 3], Faces[i * 3 + 1], Faces[i * 3 + 2]);
	}
	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << "xMin: " << xMin << "  yMin:  " << yMin << " zMin:  " << zMin << std::endl;
	std::cout << "xMax: " << xMax << "  yMax:  " << yMax << " zMax:  " << zMax << std::endl;
	std::cout << "end" << std::endl;
}

/*-------------------------�����޸Ķ�--------------------------------------*/

/*
 * Taubin���������ʵļ��㷽��
 */
void MeshSaliency::ComputeSaliency_Lee()
{
	ComputeNormals_Taubin();
	ComputeMeanCurvature_Taubin();
	ComputeSaliency_Taubin();
}

/*
 * ��������Ķ���ķ�������, ��Ҫ�޸�Ϊ���������������ƽ��ķ�����Ҫ��������������
 */
void MeshSaliency::ComputeNormals_Taubin()
{
	/*����ÿ����ķ��ߣ�����ӵ���Ӧ������������*/
	for (int i = 0; i < faceCnt; i++)
	{
		int idx[3];
		for (int k = 0; k < 3; k++)
			idx[k] = Faces[i * 3 + k];
		float vA[3], vB[3], vC[3];
		for (int j = 0; j < 3; j++)
		{
			vA[j] = Points[idx[0] * 3 + j];
			vB[j] = Points[idx[1] * 3 + j];
			vC[j] = Points[idx[2] * 3 + j];
		}
		//��ıߵ�����
		vec3 faceVec1 = vec3(vB[0] - vA[0], vB[1] - vA[1], vB[2] - vA[2]);
		vec3 faceVec2 = vec3(vC[0] - vB[0], vC[1] - vB[1], vC[2] - vB[2]);
		vec3 crossProd = cross(faceVec1, faceVec2);
		float faceArea = 0.5 * sqrt(crossProd.v[0] * crossProd.v[0] + crossProd.v[1] * crossProd.v[1]
			+ crossProd.v[2] * crossProd.v[2]);
		vec3 Npf = crossProd * faceArea;    //����������ķ������������������
		/*ͬһ�������������ķ�����ͬ����Ҫ��Ӳ�ͬ��ķ���*/
		for (int k = 0; k < 3; k++)
		{
			normals[idx[k] * 3 + 0] += Npf.v[0];
			normals[idx[k] * 3 + 1] += Npf.v[1];
			normals[idx[k] * 3 + 2] += Npf.v[2];
		}
	}

	/*��ÿ������ķ��߽��й�һ��*/
	for (int i = 0; i < vertexCnt; i++)
	{
		float norm = 0.0f;
		for (int k = 0; k < 3; k++)
			norm += normals[i * 3 + k] * normals[i * 3 + k];
		if (norm <= 0.0f)
		{
			std::cout << "vertex " << i << std::endl;
			printf("%13.6f %13.6f %13.6f\n", normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			continue;
		}
		for (int k = 0; k < 3; k++)
			normals[i * 3 + k] /= sqrt(norm);
	}

	/*����*/
	for (int i = 0; i < vertexCnt; i++)
	{
		if (isnan(normals[i * 3]) || isnan(normals[i * 3 + 1]) || isnan(normals[i * 3 + 2]))
		{
			printf("normals %d: %13.6f %13.6f %13.6f\n", i, normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
		}
	}
}

/*
 * ������������ĸ�˹����
 */
void MeshSaliency::ComputeMeanCurvature_Taubin()
{
	/*����ÿ���������״����*/
	mat3* shapeOperators = NULL;
	float* vertexArea = NULL;
	shapeOperators = new mat3[vertexCnt + 1];
	vertexArea = new float[vertexCnt + 1];
	/*��ʼ��*/
	for (int i = 0; i < vertexCnt; i++)
	{
		vertexArea[i] = 0.0f;
		for (int j = 0; j < 9; j++)
			shapeOperators[i].m[j] = 0.0f;
	}
	/*����ÿ����ķ��ߣ�����ӵ���Ӧ������������*/
	for (int f = 0; f < faceCnt; f++)
	{
		int idx[3];
		for (int k = 0; k < 3; k++)
			idx[k] = Faces[f * 3 + k];
		float vA[3], vB[3], vC[3];
		for (int j = 0; j < 3; j++)
		{
			vA[j] = Points[idx[0] * 3 + j];
			vB[j] = Points[idx[1] * 3 + j];
			vC[j] = Points[idx[2] * 3 + j];
		}
		//ÿ��������
		vec3 faceVec1 = vec3(vB[0] - vA[0], vB[1] - vA[1], vB[2] - vA[2]);
		vec3 faceVec2 = vec3(vC[0] - vB[0], vC[1] - vB[1], vC[2] - vB[2]);
		vec3 vecArea = cross(faceVec1, faceVec2);
		float faceArea = sqrt(vecArea.v[0] * vecArea.v[0] +
			vecArea.v[1] * vecArea.v[1] +
			vecArea.v[2] * vecArea.v[2]);

		for (int t = 0; t < 3; t++)
		{
			int i = Faces[f * 3 + t];
			int j = Faces[f * 3 + (t + 1) % 3];  //ѭ��3����
			/*��ȡ����i�붥��j�ķ�������*/
			vec3 Ni = vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			vec3 Nj = vec3(normals[j * 3], normals[j * 3 + 1], normals[j * 3 + 2]);
			/*��ö���i�붥��j��λ��*/
			vec3 Vi = vec3(Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
			vec3 Vj = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);

			/*���ڶ���i��������״���ӵ���ز���*/
			vec3 Tij = (identity_mat3() - wedge(Ni, Ni)) * (Vi - Vj);
			Tij = normalise(Tij);
			float kappa_ij = 2 * dot(Ni, Vj - Vi);
			kappa_ij = kappa_ij / get_squared_dist(Vi, Vj);
			/*ά������Vi����״����*/
			shapeOperators[i] = shapeOperators[i] + (wedge(Tij, Tij) * (kappa_ij * faceArea));
			vertexArea[i] += faceArea;

			/*���ڶ���j��������״���ӵ���ز���*/
			vec3 Tji = (identity_mat3() - wedge(Nj, Nj)) * (Vj - Vi);
			Tji = normalise(Tji);
			float kappa_ji = 2 * dot(Nj, Vi - Vj);
			kappa_ji /= get_squared_dist(Vi, Vj);
			/*ά������Vj����״���� faceArea��Wij�ķ��Ӳ���*/
			shapeOperators[j] = shapeOperators[j] + (wedge(Tji, Tji) * (kappa_ji * faceArea));

			vertexArea[j] += faceArea;
		}
	}

	for (int i = 0; i < vertexCnt; i++) {
		/* 1.0 / vertexArea ��Wij�ķ�ĸ���� */
		shapeOperators[i] = shapeOperators[i] * (1.0f / vertexArea[i]);
	}

	// �Խǻ���״�������󣬻��ƽ������
	meanCurvature = new float[vertexCnt];
	for (int k = 0; k < vertexCnt; k++) {
		vec3 E1 = vec3(1.0f, 0.0f, 0.0f);
		vec3 Nk = vec3(normals[k * 3], normals[k * 3 + 1], normals[k * 3 + 2]);
		bool isMinus = get_squared_dist(E1, Nk) > get_squared_dist(E1 * (-1.0f), Nk);
		vec3 Wk;
		// ͨ��Householder�任���жԽǻ�, ������Խǻ�
		if (!isMinus) // ��֤Wk������
			Wk = E1 + Nk;
		else
			Wk = E1 - Nk;
		Wk = normalise(Wk);
		mat3 Qk = identity_mat3() - (wedge(Wk, Wk) * 2.0f);
		mat3 Mk = transpose(Qk) * shapeOperators[k] * Qk;
		/*ͨ��Mk����ֵ������ƽ������*/
		meanCurvature[k] = Curvature(Mk.m[4], Mk.m[5], Mk.m[7], Mk.m[8]);
	}

	delete[] shapeOperators;
	delete[] vertexArea;
}

/*
 *  �������Saliency
 *  ����2X2���������ֵ http://scipp.ucsc.edu/~haber/ph116A/diag2x2_11.pdf
 */
void MeshSaliency::ComputeSaliency_Taubin()
{
	float diagonalLength = sqrt((xMax - xMin)*(xMax - xMin) + (yMax - yMin)*(yMax - yMin) + (zMax - zMin)*(zMax - zMin));
	float Epsilon = 0.003 * diagonalLength;
	float maxSaliencyValue[7], minSaliencyValue[7];
	int maxSaliencyIndex[7];
	for (int i = 0; i < 7; i++)
	{
		maxSaliencyValue[i] = FLT_MIN;
		minSaliencyValue[i] = FLT_MAX;
		maxSaliencyIndex[i] = 0;
	}
	/*��ͬ��ˮƽ�߶ȵ����*/
	for (int level = 1; level <= 5; level++)
	{
		std::vector<std::vector<int>> vertexIncident1(vertexCnt);
		std::vector<std::vector<float>> vertexDistance1(vertexCnt);
		std::vector<std::vector<int>> vertexIncident2(vertexCnt);
		std::vector<std::vector<float>> vertexDistance2(vertexCnt);
		float sigma = (level + 1) * Epsilon;
		float sigma2 = 2 * sigma;
		float sigma4 = 4 * sigma;
		for (int i = 0; i < vertexCnt; i++)
		{
			vec3 vecI = vec3(Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
			for (int j = 0; j < vertexCnt; j++)
			{
				if (j == i)
				{
					continue; //������ͬһ����������
				}
				vec3 vecJ = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);
				float distanceIJ = get_Euc_dist(vecI, vecJ);
				if (distanceIJ < sigma2)
				{
					vertexIncident1[i].push_back(j);            //��¼����i�����򶥵�j����2 * sigma������
					vertexDistance1[i].push_back(distanceIJ);   //��¼����i�����򶥵�j�ľ���
				}
				if (distanceIJ < sigma4)
				{
					vertexIncident2[i].push_back(j);            //��¼����i�����򶥵�j����4 * sigma������
					vertexDistance2[i].push_back(distanceIJ);   //��¼����i�����򶥵�j�ľ���
				}
			}
			/* Gaussian-weighted average of the mean curvature  sigma */
			float sigmaSquare = sigma * sigma;
			float gaussianSum1 = 0.0f;
			float gaussianSum2 = 0.0f;
			float factorSum1 = 0.0f;
			float factorSum2 = 0.0f;
			for (int k = 0; k < vertexIncident1[i].size(); k++)
			{
				float dist = vertexDistance1[i][k];
				float factor = exp(-dist / (2 * sigmaSquare));
				gaussianSum1 += meanCurvature[i] * factor;
				factorSum1 += factor;
			}
			/* Gaussian-weighted average of the mean curvature  sigma * 2 */
			for (int k = 0; k < vertexIncident2[i].size(); k++)
			{
				float dist = vertexDistance2[i][k];
				float factor = exp(-dist / (8 * sigmaSquare));
				gaussianSum2 += meanCurvature[i] * factor;
				factorSum2 += factor;
			}
			if (factorSum1 == 0.0f)
			{
				factorSum1 = 1.0f;    //���ڳߴ��С�����ֶ���û�����򶥵�
			}
			if (factorSum2 == 0.0f)
			{
				factorSum2 = 1.0f;  //���ڳߴ��С�����ֶ���û�����򶥵�
			}
			/*Saliency ��2��ʼ�洢*/
			Saliency[level + 1][i] = fabs(gaussianSum1 / factorSum1 - gaussianSum2 / factorSum2);
			if (isnan(Saliency[level + 1][i]))
			{
				std::cout << "level " << level << " vertex " << i << " Saliency is nan" << std::endl;
			}
			if (Saliency[level + 1][i] > maxSaliencyValue[level + 1])
			{
				maxSaliencyValue[level + 1] = Saliency[level + 1][i];
				maxSaliencyIndex[level + 1] = i;
			}
			minSaliencyValue[level + 1] = fmin(minSaliencyValue[level + 1], Saliency[level + 1][i]);
		}

		/*applying the non-linear normalization of suppression*/

		/*���������ԵĹ�һ��*/
		float Range = maxSaliencyValue[level + 1] - minSaliencyValue[level + 1];
		for (int i = 0; i < vertexCnt; i++)
		{
			Saliency[level + 1][i] = (Saliency[level + 1][i] - minSaliencyValue[level + 1]) / Range;
		}
		float globalMax = Saliency[level + 1][maxSaliencyIndex[level + 1]];

		int countMax = 0;
		float localMaxSum = 0.0f;
		bool localMaxFlag = true;
		/*��ֲ����ֵ*/
		for (int i = 0; i < vertexCnt; i++)
		{
			for (int v = 0; v < vertexIncident2[i].size(); v++)
			{
				int j = vertexIncident2[i][v];   //��ö���i�����򶥵�
				if (Saliency[level + 1][i] < Saliency[level + 1][j])
				{
					//���Ǿֲ����ֵ,��ǰ�����������С�����򶥵��������
					localMaxFlag = false;
					break;
				}
			}
			if (localMaxFlag)  //��ǰ�����Ǿֲ����������Ķ���
			{
				countMax++;
				localMaxSum += Saliency[level + 1][i];
			}
			localMaxFlag = true;   //������Ϊtrue
		}
		std::cout << "level " << level << " count localMax  " << countMax << std::endl;
		float localMaxMean = localMaxSum / countMax;
		std::cout << "level " << level << " localMaxMean  " << localMaxMean << std::endl;
		std::cout << "level " << level << " globalMax " << globalMax << std::endl;
		float SmoothFactor = (globalMax - localMaxMean) * (globalMax - localMaxMean);

		/*�������Ե�ֵ��ӵ�SmoothSaliency��*/
		for (int i = 0; i < vertexCnt; i++)
		{
			smoothSaliency[i] += Saliency[level + 1][i] * SmoothFactor;
		}
	}

	/*SmoothSaliency�Ĺ�һ��*/
	float minSmoothValue = FLT_MAX;
	float maxSmoothValue = FLT_MIN;
	std::cout << "vertexCnt " << vertexCnt << std::endl;
	for (int i = 0; i < vertexCnt; i++)
	{
		//std::cout << smoothSaliency[i] << std::endl;
		minSmoothValue = fmin(minSmoothValue, smoothSaliency[i]);
		maxSmoothValue = fmax(maxSmoothValue, smoothSaliency[i]);
	}
	std::cout << "minSmoothValue " << minSmoothValue << std::endl;
	std::cout << "maxSmoothValue " << maxSmoothValue << std::endl;
	float Range = maxSmoothValue - minSmoothValue;
	for (int i = 0; i < vertexCnt; i++)
	{
		smoothSaliency[i] = (smoothSaliency[i] - minSmoothValue) / Range;
	}
}


/*
 *  ����2X2���������ֵ http://scipp.ucsc.edu/~haber/ph116A/diag2x2_11.pdf
 */
float MeshSaliency::ComputeGiven_Curvature(mat2& Mvi)
{
	float a = Mvi.m[0];
	float c = Mvi.m[1];
	float b = Mvi.m[3];
	float angle = atan(2 * c / (a - b));
	angle = angle / 2;
	float cosTheta = cos(angle);
	float sinTheta = sin(angle);
	mat2 sInver(cosTheta, -sinTheta, sinTheta, cosTheta);
	mat2 s(cosTheta, sinTheta, -sinTheta, cosTheta);
	mat2 diagMatrix = sInver * Mvi * s;
	float mp11 = diagMatrix.m[0];
	float mp22 = diagMatrix.m[3];
	float kp1 = 3 * mp11 - mp22;
	float kp2 = 3 * mp22 - mp11;
	float curvature = kp1 + kp2;
	return curvature;
}

/*
 * ����2X2���������ֵ http://scipp.ucsc.edu/~haber/ph116A/diag2x2_11.pdf
 */
float MeshSaliency::Curvature(float&a, float&b, float& c, float d)
{
	float delta = (a - b) * (a - b) + 4 * c * c;
	float mp11 = 0.5 * (a + b + std::sqrt(delta));
	float mp22 = 0.5 * (a + b - std::sqrt(delta));
	float kp1 = 3 * mp11 - mp22;
	float kp2 = 3 * mp22 - mp11;
	float curvature = kp1 + kp2;
	return curvature;
}

/*-------------------------------MultiCore------------------------------------*/
/*
 * Saliency 4�˼���
 */
void MeshSaliency::ComputeSaliency_MultiCore()
{
	ComputeNormals_Taubin();
	ComputeMeanCurvature_Taubin();
	Saliency_MultiCore();
}

void MeshSaliency::Saliency_MultiCore()
{
	float diagonalLength = sqrt((xMax - xMin)*(xMax - xMin) + (yMax - yMin)*(yMax - yMin) + (zMax - zMin)*(zMax - zMin));
	float Epsilon = 0.003 * diagonalLength;
	LevelSaliencyArray(5, Epsilon, vertexCnt, LocalMax[4], Points, meanCurvature, Saliency[6], &SmoothFactor[6]);
	//���߳�ֻ�ܵ��þ�̬��
	/*4��4�߳�*/
	std::thread t1(LevelSaliency, 1, Epsilon, vertexCnt, LocalMax[0], Points, meanCurvature, Saliency[2], &SmoothFactor[2]);
	std::thread t2(LevelSaliency, 2, Epsilon, vertexCnt, LocalMax[1], Points, meanCurvature, Saliency[3], &SmoothFactor[3]);
	std::thread t3(LevelSaliency, 3, Epsilon, vertexCnt, LocalMax[2], Points, meanCurvature, Saliency[4], &SmoothFactor[4]);
	std::thread t4(LevelSaliency, 4, Epsilon, vertexCnt, LocalMax[3], Points, meanCurvature, Saliency[5], &SmoothFactor[5]);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	/*4���߳̽����󣬼���ִ��*/
	LevelSaliency(5, Epsilon, vertexCnt, LocalMax[4], Points, meanCurvature, Saliency[6], &SmoothFactor[6]);
	/*���Զ��̵߳���ֵ*/
	std::cout << "SmoothFactor[2]: " << SmoothFactor[2] << std::endl;
	/*�������Ե�ֵ��ӵ�SmoothSaliency��*/
	for (int i = 0; i < vertexCnt; i++)
	{
		for (int level = 1; level <= 5; level++)
		{
			smoothSaliency[i] += Saliency[level + 1][i] * SmoothFactor[level + 1];
		}
	}
	/*SmoothSaliency�Ĺ�һ��*/
	float minSmoothValue = FLT_MAX;
	float maxSmoothValue = FLT_MIN;
	std::cout << "vertexCnt " << vertexCnt << std::endl;
	for (int i = 0; i < vertexCnt; i++)
	{
		//std::cout << smoothSaliency[i] << std::endl;
		minSmoothValue = fmin(minSmoothValue, smoothSaliency[i]);
		maxSmoothValue = fmax(maxSmoothValue, smoothSaliency[i]);
	}
	std::cout << "minSmoothValue " << minSmoothValue << std::endl;
	std::cout << "maxSmoothValue " << maxSmoothValue << std::endl;
	float Range = maxSmoothValue - minSmoothValue;
	for (int i = 0; i < vertexCnt; i++)
	{
		smoothSaliency[i] = (smoothSaliency[i] - minSmoothValue) / Range;
	}
}

void MeshSaliency::LevelSaliency(int level, float Epsilon, int vertexCnt, bool *localMax,
	const float *Points, const float *meanCurvature, float* Saliency, float *SmoothFactor)
{
	float maxSaliencyValue, minSaliencyValue;
	int maxSaliencyIndex;
	maxSaliencyValue = FLT_MIN;
	minSaliencyValue = FLT_MAX;
	maxSaliencyIndex = 0;
	/*��ͬ��ˮƽ�߶ȵ����*/
	
	std::vector<std::vector<int>> vertexIncident1(vertexCnt);
	std::vector<std::vector<float>> vertexDistance1(vertexCnt);
	std::vector<std::vector<int>> vertexIncident2(vertexCnt);
	std::vector<std::vector<float>> vertexDistance2(vertexCnt);
	float sigma = (level + 1) * Epsilon;
	float sigma2 = 2 * sigma;
	float sigma4 = 4 * sigma;
	for (int i = 0; i < vertexCnt; i++)
	{
		vec3 vecI = vec3(Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
		for (int j = 0; j < vertexCnt; j++)
		{
			if (j == i)
			{
				continue; //������ͬһ����������
			}
			vec3 vecJ = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);
			float distanceIJ = get_Euc_dist(vecI, vecJ);
			if (distanceIJ < sigma2)
			{
				vertexIncident1[i].push_back(j);            //��¼����i�����򶥵�j����2 * sigma������
				vertexDistance1[i].push_back(distanceIJ);   //��¼����i�����򶥵�j�ľ���
			}
			if (distanceIJ < sigma4)
			{
				vertexIncident2[i].push_back(j);            //��¼����i�����򶥵�j����4 * sigma������
				vertexDistance2[i].push_back(distanceIJ);   //��¼����i�����򶥵�j�ľ���
			}
		}
		/* Gaussian-weighted average of the mean curvature  sigma */
		float sigmaSquare = sigma * sigma;
		float gaussianSum1 = 0.0f;
		float gaussianSum2 = 0.0f;
		float factorSum1 = 0.0f;
		float factorSum2 = 0.0f;
		for (int k = 0; k < vertexIncident1[i].size(); k++)
		{
			float dist = vertexDistance1[i][k];
			float factor = exp(-dist / (2 * sigmaSquare));
			gaussianSum1 += meanCurvature[i] * factor;
			factorSum1 += factor;
		}
		/* Gaussian-weighted average of the mean curvature  sigma * 2 */
		for (int k = 0; k < vertexIncident2[i].size(); k++)
		{
			float dist = vertexDistance2[i][k];
			float factor = exp(-dist / (8 * sigmaSquare));
			gaussianSum2 += meanCurvature[i] * factor;
			factorSum2 += factor;
		}
		if (factorSum1 == 0.0f)
		{
			factorSum1 = 1.0f;    //���ڳߴ��С�����ֶ���û�����򶥵�
		}
		if (factorSum2 == 0.0f)
		{
			factorSum2 = 1.0f;  //���ڳߴ��С�����ֶ���û�����򶥵�
		}
		/*Saliency ��2��ʼ�洢*/
		Saliency[i] = fabs(gaussianSum1 / factorSum1 - gaussianSum2 / factorSum2);
		if (isnan(Saliency[i]))
		{
			std::cout << "level " << level << " vertex " << i << " Saliency is nan" << std::endl;
		}
		if (Saliency[i] > maxSaliencyValue)
		{
			maxSaliencyValue = Saliency[i];
			maxSaliencyIndex = i;
		}
		minSaliencyValue = fmin(minSaliencyValue, Saliency[i]);
	}

	/*applying the non-linear normalization of suppression*/

	/*���������ԵĹ�һ��*/
	float Range = maxSaliencyValue - minSaliencyValue;
	for (int i = 0; i < vertexCnt; i++)
	{
		Saliency[i] = (Saliency[i] - minSaliencyValue) / Range;
	}
	float globalMax = Saliency[maxSaliencyIndex];

	int countMax = 0;
	float localMaxSum = 0.0f;
	bool localMaxFlag = true;
	/*��ֲ����ֵ�� ��4 * sigma �����ڣ�  2 * sigma�Ĵ��ڹ�С���ڵ�ˮƽ�µľֲ����ֵ̫��*/
	for (int i = 0; i < vertexCnt; i++)
	{
		for (int v = 0; v < vertexIncident1[i].size(); v++)        
		{
			int j = vertexIncident1[i][v];   //��ö���i�����򶥵�  
			if (Saliency[i] < Saliency[j])
			{
				//���Ǿֲ����ֵ,��ǰ�����������С�����򶥵��������
				localMaxFlag = false;
				break;
			}
		}
		if (localMaxFlag)  //��ǰ�����Ǿֲ����������Ķ���
		{
			countMax++;
			localMax[i] = true;     //�õ�Ϊ�ֲ�������ֵ���ĵ㣬��Ϊtrue
			localMaxSum += Saliency[i];
		}
		localMaxFlag = true;   //������Ϊtrue
	}
	std::cout << "level " << level << " count localMax  " << countMax << std::endl;
	float localMaxMean = localMaxSum / countMax;
	std::cout << "level " << level << " localMaxMean  " << localMaxMean << std::endl;
	std::cout << "level " << level << " globalMax " << globalMax << std::endl;
	*SmoothFactor = (globalMax - localMaxMean) * (globalMax - localMaxMean);
}

void MeshSaliency::LevelSaliencyArray(int level, float Epsilon, int vertexCnt, bool *localMax,
	const float *Points, const float *meanCurvature, float* Saliency, float *SmoothFactor)
{
	float maxSaliencyValue, minSaliencyValue;
	int maxSaliencyIndex;
	maxSaliencyValue = FLT_MIN;
	minSaliencyValue = FLT_MAX;
	maxSaliencyIndex = 0;
	/*��ͬ��ˮƽ�߶ȵ����*/

	int **vertexIncident1 = new int *[vertexCnt];
	float **vertexDistance1 = new float*[vertexCnt];
	int **vertexIncident2 = new int *[vertexCnt];
	float **vertexDistance2 = new float*[vertexCnt];
	int *IncidentSize1 = new int[vertexCnt];
	int *IncidentSize2 = new int[vertexCnt];
	float sigma = (level + 1) * Epsilon;
	float sigma2 = 2 * sigma;
	float sigma4 = 4 * sigma;
	for (int i = 0; i < vertexCnt; i++)
	{
		int *tempIncident1 = new int[vertexCnt];
		float *tempDistance1 = new float[vertexCnt];
		int count1 = 0;
		int *tempIncident2 = new int[vertexCnt];
		float *tempDistance2 = new float[vertexCnt];
		int count2 = 0;
		vec3 vecI = vec3(Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
		for (int j = 0; j < vertexCnt; j++)
		{
			if (j == i)
			{
				tempIncident1[j] = j;
				tempDistance1[j] = 0.0f;
				tempIncident2[j] = j;
				tempDistance2[j] = 0.0f;
				count1++;
				count2++;
				continue; //������ͬһ����������
			}
			vec3 vecJ = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);
			float distanceIJ = get_Euc_dist(vecI, vecJ);
			if (distanceIJ < sigma2)
			{
				tempIncident1[j] = j;            //��¼����i�����򶥵�j����2 * sigma������
				tempDistance1[j] = distanceIJ;   //��¼����i�����򶥵�j�ľ���
				count1++;
			}
			if (distanceIJ < sigma4)
			{
				tempIncident2[j] = j;            //��¼����i�����򶥵�j����4 * sigma������
				tempDistance2[j] = distanceIJ;   //��¼����i�����򶥵�j�ľ���
				count2++;
			}
		}  //for
		vertexIncident1[i] = new int[count1];
		vertexDistance1[i] = new float[count1];
		IncidentSize1[i] = count1;
		std::cout << "vertex " << i << " Incident " << std::endl;
		for (int i = 0; i < count1; i++)
		{
			std::cout << tempIncident1[i] << std::endl;
		}
		std::cout << "vertex " << i << " Distance " << std::endl;
		for (int i = 0; i < count1; i++)
		{
			std::cout << tempDistance1[i] << std::endl;
		}
		for (int j = 0; j < count1; j++)
		{
			vertexIncident1[i][j] = tempIncident1[j];
			vertexDistance1[i][j] = tempDistance1[j];
		}
		delete[] tempIncident1;
		delete[] tempDistance1;
		for (int j = 0; j < count1; j++)
		{
			std::cout << vertexIncident1[i][j] << std::endl;
			std::cout << vertexDistance1[i][j] << std::endl;
		}
		vertexIncident2[i] = new int[count2];
		vertexDistance2[i] = new float[count2];
		IncidentSize2[i] = count2;
		for (int j = 0; j < count2; j++)
		{
			vertexIncident2[i][j] = tempIncident2[j];
			vertexDistance2[i][j] = tempDistance2[j];
		}
		delete[] tempIncident2;
		delete[] tempDistance2;
		/* Gaussian-weighted average of the mean curvature  sigma */
		float sigmaSquare = sigma * sigma;
		float gaussianSum1 = 0.0f;
		float gaussianSum2 = 0.0f;
		float factorSum1 = 0.0f;
		float factorSum2 = 0.0f;
		for (int k = 0; k < IncidentSize1[i]; k++)
		{
			float dist = vertexDistance1[i][k];
			std::cout << "vertexDistance1 " << i << "  " << k <<  " " << vertexDistance1[i][k] << std::endl;
			if (isnan(vertexDistance1[i][k]))
			{
				std::cout << "vertexDistance1 " << i << " " << k << std::endl;
			}
			float factor = exp(-dist / (2 * sigmaSquare));
			std::cout << "meanCurvature " << i << "  " << meanCurvature[i] << std::endl;
			gaussianSum1 += meanCurvature[i] * factor;
			if (isnan(gaussianSum1))
			{
				std::cout << "gaussianSum1 " << gaussianSum1 << std::endl;
			}
			factorSum1 += factor;
		}
		/* Gaussian-weighted average of the mean curvature  sigma * 2 */
		for (int k = 0; k < IncidentSize2[i]; k++)
		{
			float dist = vertexDistance2[i][k];
			if (isnan(vertexDistance2[i][k]))
			{
				std::cout << "vertexDistance2 is nan " << i << " " << k << std::endl;
			}
			float factor = exp(-dist / (8 * sigmaSquare));
			gaussianSum2 += meanCurvature[i] * factor;
			if (isnan(gaussianSum2))
			{
				std::cout <<  "vertex" << i << " gaussianSum2 is nan "  << std::endl;
			}
			factorSum2 += factor;
		}
		if (factorSum1 == 0.0f)
		{
			factorSum1 = 1.0f;    //���ڳߴ��С�����ֶ���û�����򶥵�
		}
		if (factorSum2 == 0.0f)
		{
			factorSum2 = 1.0f;  //���ڳߴ��С�����ֶ���û�����򶥵�
		}
		/*Saliency ��2��ʼ�洢*/
		if (isnan(gaussianSum1) || isnan(gaussianSum2))
		{
			std::cout << "gaussianSum is nan " << std::endl;
		}
		if (isnan(factorSum1) || isnan(factorSum2))
		{
			std::cout << "factorSum is nan " << std::endl;
		}
		std::cout << "gaussianSum1 " << gaussianSum1 << std::endl;
		std::cout << "gaussianSum2 " << gaussianSum2 << std::endl;
		std::cout << "factorSum1 " << factorSum1 << std::endl;
		std::cout << "factorSum2 " << factorSum2 << std::endl;
		float test1 = gaussianSum1 / factorSum1;
		float test2 = gaussianSum2 / factorSum2;
		if (isnan(test1))
		{
			std::cout << "vertex " << i << " gaussianSum1 / factorSum1 is nan" << std::endl;
		}
		if (isnan(test2))
		{
			std::cout << "vertex " << i << " gaussianSum2 / factorSum2 is nan" << std::endl;
		}
		Saliency[i] = fabs(gaussianSum1 / factorSum1 - gaussianSum2 / factorSum2);
		if (isnan(Saliency[i]))
		{
			std::cout << "level " << level << " vertex " << i << " Saliency is nan" << std::endl;
		}
		if (Saliency[i] > maxSaliencyValue)
		{
			maxSaliencyValue = Saliency[i];
			maxSaliencyIndex = i;
		}
		minSaliencyValue = fmin(minSaliencyValue, Saliency[i]);
	}

	/*applying the non-linear normalization of suppression*/

	/*���������ԵĹ�һ��*/
	float Range = maxSaliencyValue - minSaliencyValue;
	for (int i = 0; i < vertexCnt; i++)
	{
		Saliency[i] = (Saliency[i] - minSaliencyValue) / Range;
	}
	float globalMax = Saliency[maxSaliencyIndex];

	int countMax = 0;
	float localMaxSum = 0.0f;
	bool localMaxFlag = true;
	/*��ֲ����ֵ�� ��4 * sigma �����ڣ�  2 * sigma�Ĵ��ڹ�С���ڵ�ˮƽ�µľֲ����ֵ̫��*/
	for (int i = 0; i < vertexCnt; i++)
	{
		for (int v = 0; v < IncidentSize1[i]; v++)
		{
			int j = vertexIncident1[i][v];   //��ö���i�����򶥵�  
			if (Saliency[i] < Saliency[j])
			{
				//���Ǿֲ����ֵ,��ǰ�����������С�����򶥵��������
				localMaxFlag = false;
				break;
			}
		}
		if (localMaxFlag)  //��ǰ�����Ǿֲ����������Ķ���
		{
			countMax++;
			localMax[i] = true;     //�õ�Ϊ�ֲ�������ֵ���ĵ㣬��Ϊtrue
			localMaxSum += Saliency[i];
		}
		localMaxFlag = true;   //������Ϊtrue
	}
	std::cout << "level " << level << " count localMax  " << countMax << std::endl;
	float localMaxMean = localMaxSum / countMax;
	std::cout << "level " << level << " localMaxMean  " << localMaxMean << std::endl;
	std::cout << "level " << level << " globalMax " << globalMax << std::endl;
	*SmoothFactor = (globalMax - localMaxMean) * (globalMax - localMaxMean);

	/*�����ڴ�*/
	delete[] IncidentSize1;
	delete[] IncidentSize2;
	for (int i = 0; i < vertexCnt; i++)
	{
		delete[] vertexIncident1[i];
		delete[] vertexIncident2[i];
		delete[] vertexDistance1[i];
		delete[] vertexDistance2[i];
	}
	delete[] vertexIncident1;
	delete[] vertexIncident2;
	delete[] vertexDistance1;
	delete[] vertexDistance2;
}


/*-----------------��ֵ���ã���һ��Level��������--------------------------------*/
void MeshSaliency::ComputeSaliency_OneLevel()
{
	ComputeNormals_Taubin();
	ComputeMeanCurvature_Taubin();
	float diagonalLength = sqrt((xMax - xMin)*(xMax - xMin) + (yMax - yMin)*(yMax - yMin) + (zMax - zMin)*(zMax - zMin));
	float Epsilon = 0.003 * diagonalLength;
	/*ִ�е�һ��ˮƽ�ȵĶ���ļ���*/
	LevelSaliency(1, Epsilon, vertexCnt, LocalMax[0], Points, meanCurvature, Saliency[2], &SmoothFactor[2]);
}

void MeshSaliency::WriteOneLevel(const char* SaliencyPath, const char* LocalMaxPath)
{
	/*ˮƽ��Ϊ1��ǰ30%�������Ե�����*/
	FILE* fp = NULL;//��Ҫע��
	fp = fopen(SaliencyPath, "w");  //�����ļ�
	if (NULL == fp)
		return ;//Ҫ���ش������
	float *SortSaliency = new float[vertexCnt];
	memcpy(SortSaliency, Saliency[2], vertexCnt * sizeof(float));
	std::sort(SortSaliency, SortSaliency + vertexCnt, std::greater<float>());
	int GreateIndex = ceil(vertexCnt * 0.3);
	float Threshold = SortSaliency[GreateIndex];

	for (int i = 0; i < vertexCnt; i++)
	{
		if (Saliency[2][i] > Threshold)
		{
			fprintf(fp, "%.6f %.6f %.6f\n", Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
		}
	}
	fclose(fp);
	fp = NULL;//��Ҫָ��գ������ָ��ԭ���ļ���ַ
	delete[] SortSaliency;

	/*ˮƽ��Ϊ1�ľֲ����ֵ������*/
	fp = fopen(LocalMaxPath, "w");  //�����ļ�
	if (NULL == fp)
		return;//Ҫ���ش������
	for (int i = 0; i < vertexCnt; i++)
	{
		if (LocalMax[0][i])   //true �������Ե������õ�
		{
			fprintf(fp, "%.6f %.6f %.6f\n", Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
		}
	}
	fclose(fp);
	fp = NULL;//��Ҫָ��գ������ָ��ԭ���ļ���ַ
}

void MeshSaliency::WriteOneLevel(const char* SaliencyAndLocalMaxPath)
{
	/*ˮƽ��Ϊ1��ǰ10%�������Ե�;ֲ����ֵ������*/
	FILE* fp = NULL;//��Ҫע��
	fp = fopen(SaliencyAndLocalMaxPath, "w");  //�����ļ�
	if (NULL == fp)
		return;//Ҫ���ش������
	float *SortSaliency = new float[vertexCnt];
	memcpy(SortSaliency, Saliency[2], vertexCnt * sizeof(float));
	std::sort(SortSaliency, SortSaliency + vertexCnt, std::greater<float>());
	int GreateIndex = ceil(vertexCnt * 0.2);
	float Threshold = SortSaliency[GreateIndex];

	for (int i = 0; i < vertexCnt; i++)
	{
		if (Saliency[2][i] > Threshold && LocalMax[0][i] == false)
		{
			fprintf(fp, "%.6f %.6f %.6f\n", Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
		}
	}
	delete[] SortSaliency;

	for (int i = 0; i < vertexCnt; i++)
	{
		if (LocalMax[0][i])   //true �������Ե������õ�
		{
			fprintf(fp, "%.6f %.6f %.6f\n", Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
		}
	}
	fclose(fp);
	fp = NULL;//��Ҫָ��գ������ָ��ԭ���ļ���ַ
}

/*-----------------��ֵ���ã���һ��Level��������--------------------------------*/


/*------------------------------VECTOR FUNCTIONS------------------------------*/
vec3::vec3() {}

/*
 * �����ĳ�ʼ��
 */
vec3::vec3(float x, float y, float z) {
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

/*
 * �������
 */
vec3 vec3::operator+ (const vec3& rhs) {
	vec3 vc;
	vc.v[0] = v[0] + rhs.v[0];
	vc.v[1] = v[1] + rhs.v[1];
	vc.v[2] = v[2] + rhs.v[2];
	return vc;
}

/*
 * ������ӣ�����+=����
 */
vec3& vec3::operator+= (const vec3& rhs) {
	v[0] += rhs.v[0];
	v[1] += rhs.v[1];
	v[2] += rhs.v[2];
	return *this; // return self
}

/*
 * �������
 */
vec3 vec3::operator- (const vec3& rhs) {
	vec3 vc;
	vc.v[0] = v[0] - rhs.v[0];
	vc.v[1] = v[1] - rhs.v[1];
	vc.v[2] = v[2] - rhs.v[2];
	return vc;
}

/*
 * �������������-=
 */
vec3& vec3::operator-= (const vec3& rhs) {
	v[0] -= rhs.v[0];
	v[1] -= rhs.v[1];
	v[2] -= rhs.v[2];
	return *this;
}

/*
 * ������ÿ��Ԫ�ؼ���һ������
 */
vec3 vec3::operator+ (float rhs) {
	vec3 vc;
	vc.v[0] = v[0] + rhs;
	vc.v[1] = v[1] + rhs;
	vc.v[2] = v[2] + rhs;
	return vc;
}

/*
 * ������ÿ��Ԫ�ؼ�ȥһ������
 */
vec3 vec3::operator- (float rhs) {
	vec3 vc;
	vc.v[0] = v[0] - rhs;
	vc.v[1] = v[1] - rhs;
	vc.v[2] = v[2] - rhs;
	return vc;
}

/*
 * ������ÿ��Ԫ�س���һ������
 */
vec3 vec3::operator* (float rhs) {
	vec3 vc;
	vc.v[0] = v[0] * rhs;
	vc.v[1] = v[1] * rhs;
	vc.v[2] = v[2] * rhs;
	return vc;
}

/*
 * ������ÿ��Ԫ�س���һ������
 */
vec3 vec3::operator/ (float rhs) {
	vec3 vc;
	vc.v[0] = v[0] / rhs;
	vc.v[1] = v[1] / rhs;
	vc.v[2] = v[2] / rhs;
	return vc;
}

/*
 * ������ÿ��Ԫ�س���һ������, ����*=
 */
vec3& vec3::operator*= (float rhs) {
	v[0] = v[0] * rhs;
	v[1] = v[1] * rhs;
	v[2] = v[2] * rhs;
	return *this;
}

/*
 * ��һ��������ֵ����һ������
 */
vec3& vec3::operator= (const vec3& rhs) {
	v[0] = rhs.v[0];
	v[1] = rhs.v[1];
	v[2] = rhs.v[2];
	return *this;
}

/*
 * ����֮��Ĳ��
 */
vec3 cross(const vec3& a, const vec3& b) {
	float x = a.v[1] * b.v[2] - a.v[2] * b.v[1];
	float y = a.v[2] * b.v[0] - a.v[0] * b.v[2];
	float z = a.v[0] * b.v[1] - a.v[1] * b.v[0];
	return vec3(x, y, z);
}

/*-----------------------------MATRIX FUNCTIONS-------------------------------*/
mat3::mat3() {}

/* ע��������еķ�ʽ������� */
mat3::mat3(float a, float b, float c,
	float d, float e, float f,
	float g, float h, float i) {
	m[0] = a; m[1] = b; m[2] = c;
	m[3] = d; m[4] = e; m[5] = f;
	m[6] = g; m[7] = h; m[8] = i;
}

/*
 * ��ʼ��ȫ��3�׾���
 */
mat3 zero_mat3() {
	return mat3(
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f
	);
}

/*
 * ��ʼ��3�׵�λ����
 */
mat3 identity_mat3() {
	return mat3(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	);
}

/*
 * 1X3��������3X3���������
 */
vec3 mat3::operator* (const vec3& rhs) {
	// 0x + 3y + 6z
	float x = m[0] * rhs.v[0] +
		m[3] * rhs.v[1] +
		m[6] * rhs.v[2];
	// 1x + 4y + 7z
	float y = m[1] * rhs.v[0] +
		m[4] * rhs.v[1] +
		m[7] * rhs.v[2];
	// 2x + 5y + 8z
	float z = m[2] * rhs.v[0] +
		m[5] * rhs.v[1] +
		m[8] * rhs.v[2];
	return vec3(x, y, z);
}

/*
 * 3X3�ľ������
 */
mat3 mat3::operator* (const mat3& rhs) {
	mat3 r = zero_mat3();
	int r_index = 0;
	for (int col = 0; col < 3; col++) {
		for (int row = 0; row < 3; row++) {
			float sum = 0.0f;
			for (int i = 0; i < 3; i++) {
				sum += rhs.m[i + col * 3] * m[row + i * 3];
			}
			r.m[r_index] = sum;
			r_index++;
		}
	}
	return r;
}

/*
 * �����볣�����
 */
mat3 mat3::operator* (const float& rhs) {
	mat3 r = zero_mat3();
	for (int i = 0; i < 9; i++)
		r.m[i] = rhs * m[i];
	return r;
}

/*
 * �����볣�����
 */
mat3 mat3::operator+ (const mat3& rhs) {
	mat3 r = zero_mat3();
	for (int i = 0; i < 9; i++)
		r.m[i] = m[i] + rhs.m[i];
	return r;
}

/*
 * �����볣�����
 */
mat3 mat3::operator- (const mat3& rhs) {
	mat3 r = zero_mat3();
	for (int i = 0; i < 9; i++)
		r.m[i] = m[i] - rhs.m[i];
	return r;
}
/*
 * ����ֵ
 */
mat3& mat3::operator= (const mat3& rhs) {
	for (int i = 0; i < 9; i++) {
		m[i] = rhs.m[i];
	}
	return *this;
}

/*
 * ����������ˣ���Ϊ3 * 3�ľ���
 * 3X1 * 1X3 = 3X3
 */
mat3 wedge(const vec3& a, const vec3& b) {
	mat3 r = zero_mat3();
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			r.m[3 * i + j] = a.v[i] * b.v[j];
	return r;
}

/*
 * �������ĳ���
 */
float length(const vec3& v) {
	return sqrt(v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2]);
}

/*
 * ��������һ��
 */
vec3 normalise(const vec3& v) {
	vec3 vb;
	float L = length(v);
	if (0.0f == L) {
		return vec3(0.0f, 0.0f, 0.0f);
	}
	vb.v[0] = v.v[0] / L;
	vb.v[1] = v.v[1] / L;
	vb.v[2] = v.v[2] / L;
	return vb;
}

/*
 * ����֮��ĵ�����
 */
float dot(const vec3& a, const vec3& b) {
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

/*
 * ��������ά�����֮��ľ���
 */
float get_squared_dist(vec3 from, vec3 to) {
	float x = (to.v[0] - from.v[0]) * (to.v[0] - from.v[0]);
	float y = (to.v[1] - from.v[1]) * (to.v[1] - from.v[1]);
	float z = (to.v[2] - from.v[2]) * (to.v[2] - from.v[2]);
	return x + y + z;
}

/*
 * ��������ά�����֮���ŷ����þ���
 */
float get_Euc_dist(vec3 from, vec3 to)
{
	float x = (to.v[0] - from.v[0]) * (to.v[0] - from.v[0]);
	float y = (to.v[1] - from.v[1]) * (to.v[1] - from.v[1]);
	float z = (to.v[2] - from.v[2]) * (to.v[2] - from.v[2]);
	return std::sqrt(x + y + z);
}

/*
 * �Ծ������ת��
 */
mat3 transpose(const mat3& mm) {
	return mat3(
		mm.m[0], mm.m[3], mm.m[6],
		mm.m[1], mm.m[4], mm.m[7],
		mm.m[2], mm.m[5], mm.m[8]
	);
}


/*-------���׾��������----------------*/
mat2::mat2() {}

/* ע��������еķ�ʽ������� */
mat2::mat2(float a, float b, 
	float c,float d) {
	m[0] = a; m[1] = b; 
	m[2] = c; m[3] = d; 
}

/*
 * ��ʼ��ȫ��2�׾���
 */
mat2 zero_mat2() {
	return mat2(
		0.0f, 0.0f,
		0.0f, 0.0f
	);
}

/*
 * ��ʼ��3�׵�λ����
 */
mat2 identity_mat2() {
	return mat2(
		1.0f, 0.0f, 
		0.0f, 1.0f
	);
}

/*
 * 2X2�ľ������
 */
mat2 mat2::operator* (const mat2& rhs) {
	mat2 r = zero_mat2();
	int r_index = 0;
	for (int col = 0; col < 2; col++) {
		for (int row = 0; row < 2; row++) {
			float sum = 0.0f;
			for (int i = 0; i < 2; i++) {
				sum += rhs.m[i + col * 2] * m[row + i * 2];
			}
			r.m[r_index] = sum;
			r_index++;
		}
	}
	return r;
}

/*
 * �����볣�����
 */
mat2 mat2::operator* (const float& rhs) {
	mat2 r = zero_mat2();
	for (int i = 0; i < 4; i++)
		r.m[i] = rhs * m[i];
	return r;
}

/*
 * �����볣�����
 */
mat2 mat2::operator+ (const mat2& rhs) {
	mat2 r = zero_mat2();
	for (int i = 0; i < 4; i++)
		r.m[i] = m[i] + rhs.m[i];
	return r;
}

/*
 * �����볣�����
 */
mat2 mat2::operator- (const mat2& rhs) {
	mat2 r = zero_mat2();
	for (int i = 0; i < 4; i++)
		r.m[i] = m[i] - rhs.m[i];
	return r;
}

/*
 * ����ֵ
 */
mat2& mat2::operator= (const mat2& rhs) {
	for (int i = 0; i < 4; i++) {
		m[i] = rhs.m[i];
	}
	return *this;
}

/*
 * ����������ǰ30%�ĵ����
 */
void MeshSaliency::WriteSaliencyPoints(const char* filepath)
{
	std::ofstream outfile;   //�����
	outfile.open(filepath, std::ios::app);
	if (!outfile.is_open())
		std::cout << "Open file failure" << std::endl;

	float *SortSaliency = new float[vertexCnt];
	memcpy(SortSaliency, smoothSaliency, vertexCnt * sizeof(float));
	std::sort(SortSaliency, SortSaliency + vertexCnt, std::greater<float>());
	int GreateIndex = ceil(vertexCnt * 0.3);
	float Threshold = SortSaliency[GreateIndex];

	for (int i = 0; i < vertexCnt; i++)
	{
		if (smoothSaliency[i] > Threshold)
		{
			outfile << Points[i * 3] << "   " << Points[i * 3 + 1] << "  ";
			outfile << Points[i * 3 + 2] << std::endl;
		}
	}

	outfile.close();
	delete[] SortSaliency;
}

/*
 * ������ˮƽ����������ǰ30%�ĵ����
 */
void MeshSaliency::WriteLevelPoints(std::string filePath[])
{
	for (int index = 0; index < 5; index++)
	{
		std::ofstream outfile;   //�����
		outfile.open(filePath[index].c_str(), std::ios::app);
		if (!outfile.is_open())
			std::cout << "Open file failure" << std::endl;

		float *SortSaliency = new float[vertexCnt];
		memcpy(SortSaliency, Saliency[index + 2], vertexCnt * sizeof(float));
		std::sort(SortSaliency, SortSaliency + vertexCnt, std::greater<float>());
		int GreateIndex = ceil(vertexCnt * 0.3);
		float Threshold = SortSaliency[GreateIndex];

		for (int i = 0; i < vertexCnt; i++)
		{
			if (Saliency[index + 2][i] > Threshold)
			{
				outfile << Points[i * 3] << "   " << Points[i * 3 + 1] << "  ";
				outfile << Points[i * 3 + 2] << std::endl;
			}
		}
		outfile.close();
		delete[] SortSaliency;
	}
}

/*
 * �������������µľֲ����������ֵ�ĵ����
 */
void MeshSaliency::WriteLocalMaxPoints(std::string filePath[])
{
	for (int index = 0; index < 5; index++)
	{
		std::ofstream outfile;   //�����
		outfile.open(filePath[index].c_str(), std::ios::app);
		if (!outfile.is_open())
			std::cout << "Open file failure" << std::endl;

		for (int i = 0; i < vertexCnt; i++)
		{
			if (LocalMax[index][i])   //true �������Ե������õ�
			{
				outfile << Points[i * 3] << "   " << Points[i * 3 + 1] << "  ";
				outfile << Points[i * 3 + 2] << std::endl;
			}
		}
		outfile.close();
	}
}


/*---------------------------------ComputeSaliency2-----------------------------------------------------*/

/*
 *  ��Ҫ���ģ��ӻ��ڶ���ı���������Ϊ������ı����������ظ����������ļ�����̣��ӿ������ٶ�
              ������ȥ��ԭ�еĹ�������Ĺ��̣��������ʾ��VTK��ɣ�ֻ��ȡ�����Եļ���
 */
void MeshSaliency::ComputeSaliency2()
{
	ComputeMeanCurvature2();
}


/*
 * �����ڶ������������ʵĹ��̣�����Ϊ������ı���������ʵĹ��̣��ӿ��ٶ�
 */
void MeshSaliency::ComputeMeanCurvature2()
{

}

/*
 * �����붥�������������˫��Ԫ�������
 * ������������������ߣ�һ��
 */
float MeshSaliency::dualArea()
{

	return 0;
}