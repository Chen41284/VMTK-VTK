#include "Mesh_Saliency.h"
#include <vtkPLYReader.h>

/*
 * 输入顶点与面的数据
 * 格式：
 *   XXX -- 顶点的数量，整数
 *   XXX -- 面的数量，整数
 *   A B C -- 顶点的坐标，浮点数
 *   ......
 *   D E F -- 每个面的顶点编号
 * 注意事项：面的顶点编号从0开始
 */ 
MeshSaliency::MeshSaliency(std::string filePath)
{
	std::ifstream infile(filePath.c_str());
	infile >> vertexCnt;  //顶点的数量
	infile >> faceCnt;    //面的数量
	xMin = FLT_MAX, yMin = FLT_MAX, zMin = FLT_MAX;
	xMax = FLT_MIN, yMax = FLT_MIN, zMax = FLT_MIN;
	Points = new float[3 * vertexCnt];
	Faces = new int[3 * faceCnt];
	normals = new float[3 * vertexCnt];
	/*顶点法线的初始化*/
	for (int i = 0; i < 3 * vertexCnt; i++)
	{
		normals[i] = 0.0f;
	}
	/*不同的水平的显著性*/
	for (int i = 0; i < 7; i++)
	{
		Saliency[i] = new float[vertexCnt];
		SmoothFactor[i] = 0.0f;
	}
	/*不同水平度的局部最大值点*/
	for (int i = 0; i < 5; i++)
	{
		LocalMax[i] = new bool[vertexCnt];
	}
	meanCurvature = new float[vertexCnt];
	smoothSaliency = new float[vertexCnt];
	/*曲率和显著性的初始化*/
	for (int i = 0; i < vertexCnt; i++)
	{
		meanCurvature[i] = 0.0f;
		smoothSaliency[i] = 0.0f;
		Saliency[0][i] = 0.0f, Saliency[1][i] = 0.0f, Saliency[2][i] = 0.0f;
		Saliency[3][i] = 0.0f, Saliency[4][i] = 0.0f, Saliency[5][i] = 0.0f;
		Saliency[6][i] = 0.0f;
		LocalMax[0][i] = LocalMax[1][i] = LocalMax[2][i] = LocalMax[3][i] = LocalMax[4][i] = false;
	}
	/*输入顶点数据*/
	for (int i = 0; i < vertexCnt; i++)
	{
		infile >> Points[i * 3] >> Points[i * 3 + 1] >> Points[i * 3 + 2]; 
		xMin = fmin(xMin, Points[i * 3]), xMax = fmax(xMax, Points[i * 3]);
		yMin = fmin(yMin, Points[i * 3 + 1]), yMax = fmax(yMax, Points[i * 3 + 1]);
		zMin = fmin(zMin, Points[i * 3 + 2]), zMax = fmax(zMax, Points[i * 3 + 2]);
	}
	/*输入面的数据*/
	for (int i = 0; i < faceCnt; i++)
	{
		infile >> Faces[i * 3] >> Faces[i * 3 + 1] >> Faces[i * 3 + 2];
	}

	
	infile.close();
}

/*
 * 输入PLY文件
 */
MeshSaliency::MeshSaliency(const char* plyfilePath)
{
	vtkSmartPointer<vtkPLYReader> reader =
		vtkSmartPointer<vtkPLYReader>::New();
	reader->SetFileName(plyfilePath);
	reader->Update();
	vtkSmartPointer<vtkPolyData> polydata = reader->GetOutput();
	vertexCnt = polydata->GetNumberOfPoints();  //顶点的数量
	faceCnt = polydata->GetNumberOfCells();    //面的数量
	xMin = FLT_MAX, yMin = FLT_MAX, zMin = FLT_MAX;
	xMax = FLT_MIN, yMax = FLT_MIN, zMax = FLT_MIN;
	Points = new float[3 * vertexCnt];
	Faces = new int[3 * faceCnt];
	normals = new float[3 * vertexCnt];
	/*顶点法线的初始化*/
	for (int i = 0; i < 3 * vertexCnt; i++)
	{
		normals[i] = 0.0f;
	}
	/*不同的水平的显著性*/
	for (int i = 0; i < 7; i++)
	{
		Saliency[i] = new float[vertexCnt];
		SmoothFactor[i] = 0.0f;
	}
	/*不同水平度的局部最大值点*/
	for (int i = 0; i < 5; i++)
	{
		LocalMax[i] = new bool[vertexCnt];
	}
	meanCurvature = new float[vertexCnt];
	smoothSaliency = new float[vertexCnt];
	/*曲率和显著性的初始化*/
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
 * 开启面的数据减1, 所有的顶点编号均从0开始/
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
 *析构函数，释放内存空间
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
 * 计算三角网格每个顶点的显著性
 */
void MeshSaliency::ComputeSaliency()
{
	ComputeNormals();
	ComputeMeanCurvature();
	ComputeSupressedSaliency();
}


/*
 * 计算网格的顶点的法线向量
 */
void MeshSaliency::ComputeNormals()
{
	/*计算每个面的法线，并添加到相应的三个顶点中*/
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
		//面的边的向量
		vec3 faceVec1 = vec3(vB[0] - vA[0], vB[1] - vA[1], vB[2] - vA[2]);
		vec3 faceVec2 = vec3(vC[0] - vB[0], vC[1] - vB[1], vC[2] - vB[2]);
		vec3 crossProd = cross(faceVec1, faceVec2);
		/*同一个面的三个顶点的法线相同，需要添加不同面的法线*/
		for (int k = 0; k < 3; k++)
		{
			normals[idx[k] * 3 + 0] += crossProd.v[0];
			normals[idx[k] * 3 + 1] += crossProd.v[1];
			normals[idx[k] * 3 + 2] += crossProd.v[2];
		}
	}

	/*对每个顶点的法线进行归一化*/
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

	/*测试*/
	for (int i = 0; i < vertexCnt; i++)
	{
		if (isnan(normals[i * 3]) || isnan(normals[i * 3 + 1]) || isnan(normals[i * 3 + 2]))
		{
			printf("normals %d: %13.6f %13.6f %13.6f\n", i, normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
		}
	}
}

/*
 * 计算三角网格的高斯曲率
 */
void MeshSaliency::ComputeMeanCurvature()
{
	/*计算每个顶点的形状算子*/
	mat3* shapeOperators = NULL;
	float* vertexArea = NULL;
	shapeOperators = new mat3[vertexCnt + 1];
	vertexArea = new float[vertexCnt + 1];
	/*初始化*/
	for (int i = 0; i < vertexCnt; i++)
	{
		vertexArea[i] = 0.0f;
		for (int j = 0; j < 9; j++)
			shapeOperators[i].m[j] = 0.0f;
	}
	/*计算每个面的法线，并添加到相应的三个顶点中*/
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
		//每个面的面积
		vec3 faceVec1 = vec3(vB[0] - vA[0], vB[1] - vA[1], vB[2] - vA[2]);
		vec3 faceVec2 = vec3(vC[0] - vB[0], vC[1] - vB[1], vC[2] - vB[2]);
		vec3 vecArea = cross(faceVec1, faceVec2);
		float faceArea = sqrt(vecArea.v[0] * vecArea.v[0] +
			vecArea.v[1] * vecArea.v[1] +
			vecArea.v[2] * vecArea.v[2]);

		for (int t = 0; t < 3; t++)
		{
			int i = Faces[f * 3 + t];
			int j = Faces[f * 3 + (t + 1) % 3];  //循环3条边

			if (i < 0 || i > vertexCnt)
			{
				std::cout << "顶点 " << i << "范围出错 " << std::endl;
			}
			if (j < 0 || j > vertexCnt)
			{
				std::cout << "顶点 " << j << "范围出错 " << std::endl;
			}
			/*获取顶点i与顶点j的法线向量*/
			vec3 Ni = vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			vec3 Nj = vec3(normals[j * 3], normals[j * 3 + 1], normals[j * 3 + 2]);
			/*获得顶点i与顶点j的位置*/
			vec3 Vi = vec3(Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
			vec3 Vj = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);

			/*对于顶点i，跟新形状算子的相关部分*/
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
			/*维护顶点Vi的形状算子*/
			shapeOperators[i] = shapeOperators[i] + (wedge(Tij, Tij) * (kappa_ij * faceArea));
			vertexArea[i] += faceArea;

			/*对于顶点j，跟新形状算子的相关部分*/
			vec3 Tji = (identity_mat3() - wedge(Nj, Nj)) * (Vj - Vi);
			Tji = normalise(Tji);
			float kappa_ji = 2 * dot(Nj, Vi - Vj);
			if (isnan(kappa_ji))
			{
				std::cout << " face " << f << " " << std::endl;
			}
			kappa_ji /= get_squared_dist(Vi, Vj);
			/*维护顶点Vj的形状算子*/
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

	// 对角化形状参数矩阵，获得平均曲率
	meanCurvature = new float[vertexCnt];
	for (int k = 0; k < vertexCnt; k++) {
		vec3 E1 = vec3(1.0f, 0.0f, 0.0f);
		vec3 Nk = vec3(normals[k * 3], normals[k * 3 + 1], normals[k * 3 + 2]);
		bool isMinus = get_squared_dist(E1, Nk) > get_squared_dist(E1 * (-1.0f), Nk);
		vec3 Wk;
		// 通过Householder变换进行对角化, 将矩阵对角化
		if (!isMinus) // 保证Wk是正数
			Wk = E1 + Nk;
		else
			Wk = E1 - Nk;
		Wk = normalise(Wk);  
		mat3 Qk = identity_mat3() - (wedge(Wk, Wk) * 2.0f);
		mat3 Mk = transpose(Qk) * shapeOperators[k] * Qk;
		/*通过Mk矩阵的迹来计算平均曲率*/
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
 * 计算三角网格的顶点的显著性
 */
void MeshSaliency::ComputeSupressedSaliency()
{
	// Calculate the incident matrix ( as linked list )
	int* first = NULL;    //记录起点，从incidentVertex中查找实际的顶点
	int* next = NULL;     //记录6 * faces 链接关系
	int* incidentVertex = NULL;   // 记录个链接关系的实际顶点
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
		/*三角形，三条边，无向，6对*/
		for (int i = 0; i < 3; i++) {                      
			int j1 = idx[(i + 1) % 3], j2 = idx[(i + 2) % 3];
			//incidentVertex记录链接的关系
			incidentVertex[++edgeCnt] = j1;
			//next记录的是下一个链接顶点，-1相当于终点（最先记录）
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
	for (int k = 0; k < vertexCnt; k++) {   //对每个顶点都进行广度遍历
		// 初始化Saliency的高斯过滤器
		float gaussianSigma1[7], gaussianSigma2[7], sumSigma1[7], sumSigma2[7];
		for (int i = 0; i <= 6; i++)
			gaussianSigma1[i] = gaussianSigma2[i] = 0.0f,
			sumSigma1[i] = sumSigma2[i] = 0.0f;
		// 获得顶点的信息
		vec3 vVec = vec3(Points[k * 3], Points[k * 3 + 1], Points[k * 3 + 2]);
		// 初始化队列，寻找领域
		for (int i = 0; i < vertexCnt; i++)
			used[i] = false;
		std::queue<int> Q;
		Q.push(k);
		used[k] = true;
		// Frsit BFS
		while (!Q.empty()) {
			// 获得队列中的第一个元素
			int idx = Q.front(); Q.pop();
			if (idx < 0 || idx > vertexCnt)
			{
				std::cout << "idx 范围出错 " << std::endl;
			}
			vec3 idxVec = vec3(Points[idx * 3], Points[idx * 3 + 1], Points[idx * 3 + 2]);
			// 将一个水平度的顶点放入队列中
			for (int e = first[idx]; e != -1; e = next[e]) {
				int idxNext = incidentVertex[e];
				// 拓展下个水平度的顶点
				if (!used[idxNext]) {
					vec3 idxNextVec = vec3(Points[idxNext * 3], Points[idxNext * 3 + 1], Points[idxNext * 3 + 2]);
					/*窗口选择，不设邻域，遍历所有连通的顶点*/
					if (get_squared_dist(vVec, idxNextVec) <= 36 * sigma*sigma)
						Q.push(incidentVertex[e]),
						used[incidentVertex[e]] = 1;
				}
			}
			// 更新高斯过滤器
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
		/*获取现有的顶点的信息*/
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


	/*添加*/
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
 * 输出顶点与面的数据
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

/*-------------------------代码修改二--------------------------------------*/

/*
 * Taubin的网格曲率的计算方法
 */
void MeshSaliency::ComputeSaliency_Lee()
{
	ComputeNormals_Taubin();
	ComputeMeanCurvature_Taubin();
	ComputeSaliency_Taubin();
}

/*
 * 计算网格的顶点的法线向量, 主要修改为，计算的三角网格平面的法线需要乘以三角面的面积
 */
void MeshSaliency::ComputeNormals_Taubin()
{
	/*计算每个面的法线，并添加到相应的三个顶点中*/
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
		//面的边的向量
		vec3 faceVec1 = vec3(vB[0] - vA[0], vB[1] - vA[1], vB[2] - vA[2]);
		vec3 faceVec2 = vec3(vC[0] - vB[0], vC[1] - vB[1], vC[2] - vB[2]);
		vec3 crossProd = cross(faceVec1, faceVec2);
		float faceArea = 0.5 * sqrt(crossProd.v[0] * crossProd.v[0] + crossProd.v[1] * crossProd.v[1]
			+ crossProd.v[2] * crossProd.v[2]);
		vec3 Npf = crossProd * faceArea;    //三角网格面的法线向量乘以面积常数
		/*同一个面的三个顶点的法线相同，需要添加不同面的法线*/
		for (int k = 0; k < 3; k++)
		{
			normals[idx[k] * 3 + 0] += Npf.v[0];
			normals[idx[k] * 3 + 1] += Npf.v[1];
			normals[idx[k] * 3 + 2] += Npf.v[2];
		}
	}

	/*对每个顶点的法线进行归一化*/
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

	/*测试*/
	for (int i = 0; i < vertexCnt; i++)
	{
		if (isnan(normals[i * 3]) || isnan(normals[i * 3 + 1]) || isnan(normals[i * 3 + 2]))
		{
			printf("normals %d: %13.6f %13.6f %13.6f\n", i, normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
		}
	}
}

/*
 * 计算三角网格的高斯曲率
 */
void MeshSaliency::ComputeMeanCurvature_Taubin()
{
	/*计算每个顶点的形状算子*/
	mat3* shapeOperators = NULL;
	float* vertexArea = NULL;
	shapeOperators = new mat3[vertexCnt + 1];
	vertexArea = new float[vertexCnt + 1];
	/*初始化*/
	for (int i = 0; i < vertexCnt; i++)
	{
		vertexArea[i] = 0.0f;
		for (int j = 0; j < 9; j++)
			shapeOperators[i].m[j] = 0.0f;
	}
	/*计算每个面的法线，并添加到相应的三个顶点中*/
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
		//每个面的面积
		vec3 faceVec1 = vec3(vB[0] - vA[0], vB[1] - vA[1], vB[2] - vA[2]);
		vec3 faceVec2 = vec3(vC[0] - vB[0], vC[1] - vB[1], vC[2] - vB[2]);
		vec3 vecArea = cross(faceVec1, faceVec2);
		float faceArea = sqrt(vecArea.v[0] * vecArea.v[0] +
			vecArea.v[1] * vecArea.v[1] +
			vecArea.v[2] * vecArea.v[2]);

		for (int t = 0; t < 3; t++)
		{
			int i = Faces[f * 3 + t];
			int j = Faces[f * 3 + (t + 1) % 3];  //循环3条边
			/*获取顶点i与顶点j的法线向量*/
			vec3 Ni = vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			vec3 Nj = vec3(normals[j * 3], normals[j * 3 + 1], normals[j * 3 + 2]);
			/*获得顶点i与顶点j的位置*/
			vec3 Vi = vec3(Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
			vec3 Vj = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);

			/*对于顶点i，跟新形状算子的相关部分*/
			vec3 Tij = (identity_mat3() - wedge(Ni, Ni)) * (Vi - Vj);
			Tij = normalise(Tij);
			float kappa_ij = 2 * dot(Ni, Vj - Vi);
			kappa_ij = kappa_ij / get_squared_dist(Vi, Vj);
			/*维护顶点Vi的形状算子*/
			shapeOperators[i] = shapeOperators[i] + (wedge(Tij, Tij) * (kappa_ij * faceArea));
			vertexArea[i] += faceArea;

			/*对于顶点j，跟新形状算子的相关部分*/
			vec3 Tji = (identity_mat3() - wedge(Nj, Nj)) * (Vj - Vi);
			Tji = normalise(Tji);
			float kappa_ji = 2 * dot(Nj, Vi - Vj);
			kappa_ji /= get_squared_dist(Vi, Vj);
			/*维护顶点Vj的形状算子 faceArea，Wij的分子部分*/
			shapeOperators[j] = shapeOperators[j] + (wedge(Tji, Tji) * (kappa_ji * faceArea));

			vertexArea[j] += faceArea;
		}
	}

	for (int i = 0; i < vertexCnt; i++) {
		/* 1.0 / vertexArea 是Wij的分母部分 */
		shapeOperators[i] = shapeOperators[i] * (1.0f / vertexArea[i]);
	}

	// 对角化形状参数矩阵，获得平均曲率
	meanCurvature = new float[vertexCnt];
	for (int k = 0; k < vertexCnt; k++) {
		vec3 E1 = vec3(1.0f, 0.0f, 0.0f);
		vec3 Nk = vec3(normals[k * 3], normals[k * 3 + 1], normals[k * 3 + 2]);
		bool isMinus = get_squared_dist(E1, Nk) > get_squared_dist(E1 * (-1.0f), Nk);
		vec3 Wk;
		// 通过Householder变换进行对角化, 将矩阵对角化
		if (!isMinus) // 保证Wk是正数
			Wk = E1 + Nk;
		else
			Wk = E1 - Nk;
		Wk = normalise(Wk);
		mat3 Qk = identity_mat3() - (wedge(Wk, Wk) * 2.0f);
		mat3 Mk = transpose(Qk) * shapeOperators[k] * Qk;
		/*通过Mk特征值来计算平均曲率*/
		meanCurvature[k] = Curvature(Mk.m[4], Mk.m[5], Mk.m[7], Mk.m[8]);
	}

	delete[] shapeOperators;
	delete[] vertexArea;
}

/*
 *  单核求解Saliency
 *  计算2X2矩阵的特征值 http://scipp.ucsc.edu/~haber/ph116A/diag2x2_11.pdf
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
	/*不同的水平尺度的求解*/
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
					continue; //跳过是同一个顶点的情况
				}
				vec3 vecJ = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);
				float distanceIJ = get_Euc_dist(vecI, vecJ);
				if (distanceIJ < sigma2)
				{
					vertexIncident1[i].push_back(j);            //记录顶点i的邻域顶点j，在2 * sigma距离内
					vertexDistance1[i].push_back(distanceIJ);   //记录顶点i与邻域顶点j的距离
				}
				if (distanceIJ < sigma4)
				{
					vertexIncident2[i].push_back(j);            //记录顶点i的邻域顶点j，在4 * sigma距离内
					vertexDistance2[i].push_back(distanceIJ);   //记录顶点i与邻域顶点j的距离
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
				factorSum1 = 1.0f;    //创口尺寸过小，部分顶点没有邻域顶点
			}
			if (factorSum2 == 0.0f)
			{
				factorSum2 = 1.0f;  //创口尺寸过小，部分顶点没有邻域顶点
			}
			/*Saliency 从2开始存储*/
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

		/*网格显著性的归一化*/
		float Range = maxSaliencyValue[level + 1] - minSaliencyValue[level + 1];
		for (int i = 0; i < vertexCnt; i++)
		{
			Saliency[level + 1][i] = (Saliency[level + 1][i] - minSaliencyValue[level + 1]) / Range;
		}
		float globalMax = Saliency[level + 1][maxSaliencyIndex[level + 1]];

		int countMax = 0;
		float localMaxSum = 0.0f;
		bool localMaxFlag = true;
		/*求局部最大值*/
		for (int i = 0; i < vertexCnt; i++)
		{
			for (int v = 0; v < vertexIncident2[i].size(); v++)
			{
				int j = vertexIncident2[i][v];   //获得顶点i的领域顶点
				if (Saliency[level + 1][i] < Saliency[level + 1][j])
				{
					//不是局部最大值,当前顶点的显著性小于邻域顶点的显著性
					localMaxFlag = false;
					break;
				}
			}
			if (localMaxFlag)  //当前顶点是局部显著性最大的顶点
			{
				countMax++;
				localMaxSum += Saliency[level + 1][i];
			}
			localMaxFlag = true;   //重新置为true
		}
		std::cout << "level " << level << " count localMax  " << countMax << std::endl;
		float localMaxMean = localMaxSum / countMax;
		std::cout << "level " << level << " localMaxMean  " << localMaxMean << std::endl;
		std::cout << "level " << level << " globalMax " << globalMax << std::endl;
		float SmoothFactor = (globalMax - localMaxMean) * (globalMax - localMaxMean);

		/*将显著性的值添加到SmoothSaliency中*/
		for (int i = 0; i < vertexCnt; i++)
		{
			smoothSaliency[i] += Saliency[level + 1][i] * SmoothFactor;
		}
	}

	/*SmoothSaliency的归一化*/
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
 *  计算2X2矩阵的特征值 http://scipp.ucsc.edu/~haber/ph116A/diag2x2_11.pdf
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
 * 计算2X2矩阵的特征值 http://scipp.ucsc.edu/~haber/ph116A/diag2x2_11.pdf
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
 * Saliency 4核计算
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
	//多线程只能调用静态类
	/*4核4线程*/
	std::thread t1(LevelSaliency, 1, Epsilon, vertexCnt, LocalMax[0], Points, meanCurvature, Saliency[2], &SmoothFactor[2]);
	std::thread t2(LevelSaliency, 2, Epsilon, vertexCnt, LocalMax[1], Points, meanCurvature, Saliency[3], &SmoothFactor[3]);
	std::thread t3(LevelSaliency, 3, Epsilon, vertexCnt, LocalMax[2], Points, meanCurvature, Saliency[4], &SmoothFactor[4]);
	std::thread t4(LevelSaliency, 4, Epsilon, vertexCnt, LocalMax[3], Points, meanCurvature, Saliency[5], &SmoothFactor[5]);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	/*4个线程结束后，继续执行*/
	LevelSaliency(5, Epsilon, vertexCnt, LocalMax[4], Points, meanCurvature, Saliency[6], &SmoothFactor[6]);
	/*测试多线程的数值*/
	std::cout << "SmoothFactor[2]: " << SmoothFactor[2] << std::endl;
	/*将显著性的值添加到SmoothSaliency中*/
	for (int i = 0; i < vertexCnt; i++)
	{
		for (int level = 1; level <= 5; level++)
		{
			smoothSaliency[i] += Saliency[level + 1][i] * SmoothFactor[level + 1];
		}
	}
	/*SmoothSaliency的归一化*/
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
	/*不同的水平尺度的求解*/
	
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
				continue; //跳过是同一个顶点的情况
			}
			vec3 vecJ = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);
			float distanceIJ = get_Euc_dist(vecI, vecJ);
			if (distanceIJ < sigma2)
			{
				vertexIncident1[i].push_back(j);            //记录顶点i的邻域顶点j，在2 * sigma距离内
				vertexDistance1[i].push_back(distanceIJ);   //记录顶点i与邻域顶点j的距离
			}
			if (distanceIJ < sigma4)
			{
				vertexIncident2[i].push_back(j);            //记录顶点i的邻域顶点j，在4 * sigma距离内
				vertexDistance2[i].push_back(distanceIJ);   //记录顶点i与邻域顶点j的距离
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
			factorSum1 = 1.0f;    //创口尺寸过小，部分顶点没有邻域顶点
		}
		if (factorSum2 == 0.0f)
		{
			factorSum2 = 1.0f;  //创口尺寸过小，部分顶点没有邻域顶点
		}
		/*Saliency 从2开始存储*/
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

	/*网格显著性的归一化*/
	float Range = maxSaliencyValue - minSaliencyValue;
	for (int i = 0; i < vertexCnt; i++)
	{
		Saliency[i] = (Saliency[i] - minSaliencyValue) / Range;
	}
	float globalMax = Saliency[maxSaliencyIndex];

	int countMax = 0;
	float localMaxSum = 0.0f;
	bool localMaxFlag = true;
	/*求局部最大值， 在4 * sigma 距离内，  2 * sigma的窗口过小，在低水平下的局部最大值太多*/
	for (int i = 0; i < vertexCnt; i++)
	{
		for (int v = 0; v < vertexIncident1[i].size(); v++)        
		{
			int j = vertexIncident1[i][v];   //获得顶点i的领域顶点  
			if (Saliency[i] < Saliency[j])
			{
				//不是局部最大值,当前顶点的显著性小于邻域顶点的显著性
				localMaxFlag = false;
				break;
			}
		}
		if (localMaxFlag)  //当前顶点是局部显著性最大的顶点
		{
			countMax++;
			localMax[i] = true;     //该点为局部显著性值最大的点，置为true
			localMaxSum += Saliency[i];
		}
		localMaxFlag = true;   //重新置为true
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
	/*不同的水平尺度的求解*/

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
				continue; //跳过是同一个顶点的情况
			}
			vec3 vecJ = vec3(Points[j * 3], Points[j * 3 + 1], Points[j * 3 + 2]);
			float distanceIJ = get_Euc_dist(vecI, vecJ);
			if (distanceIJ < sigma2)
			{
				tempIncident1[j] = j;            //记录顶点i的邻域顶点j，在2 * sigma距离内
				tempDistance1[j] = distanceIJ;   //记录顶点i与邻域顶点j的距离
				count1++;
			}
			if (distanceIJ < sigma4)
			{
				tempIncident2[j] = j;            //记录顶点i的邻域顶点j，在4 * sigma距离内
				tempDistance2[j] = distanceIJ;   //记录顶点i与邻域顶点j的距离
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
			factorSum1 = 1.0f;    //创口尺寸过小，部分顶点没有邻域顶点
		}
		if (factorSum2 == 0.0f)
		{
			factorSum2 = 1.0f;  //创口尺寸过小，部分顶点没有邻域顶点
		}
		/*Saliency 从2开始存储*/
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

	/*网格显著性的归一化*/
	float Range = maxSaliencyValue - minSaliencyValue;
	for (int i = 0; i < vertexCnt; i++)
	{
		Saliency[i] = (Saliency[i] - minSaliencyValue) / Range;
	}
	float globalMax = Saliency[maxSaliencyIndex];

	int countMax = 0;
	float localMaxSum = 0.0f;
	bool localMaxFlag = true;
	/*求局部最大值， 在4 * sigma 距离内，  2 * sigma的窗口过小，在低水平下的局部最大值太多*/
	for (int i = 0; i < vertexCnt; i++)
	{
		for (int v = 0; v < IncidentSize1[i]; v++)
		{
			int j = vertexIncident1[i][v];   //获得顶点i的领域顶点  
			if (Saliency[i] < Saliency[j])
			{
				//不是局部最大值,当前顶点的显著性小于邻域顶点的显著性
				localMaxFlag = false;
				break;
			}
		}
		if (localMaxFlag)  //当前顶点是局部显著性最大的顶点
		{
			countMax++;
			localMax[i] = true;     //该点为局部显著性值最大的点，置为true
			localMaxSum += Saliency[i];
		}
		localMaxFlag = true;   //重新置为true
	}
	std::cout << "level " << level << " count localMax  " << countMax << std::endl;
	float localMaxMean = localMaxSum / countMax;
	std::cout << "level " << level << " localMaxMean  " << localMaxMean << std::endl;
	std::cout << "level " << level << " globalMax " << globalMax << std::endl;
	*SmoothFactor = (globalMax - localMaxMean) * (globalMax - localMaxMean);

	/*回收内存*/
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


/*-----------------移值所用，仅一个Level的显著点--------------------------------*/
void MeshSaliency::ComputeSaliency_OneLevel()
{
	ComputeNormals_Taubin();
	ComputeMeanCurvature_Taubin();
	float diagonalLength = sqrt((xMax - xMin)*(xMax - xMin) + (yMax - yMin)*(yMax - yMin) + (zMax - zMin)*(zMax - zMin));
	float Epsilon = 0.003 * diagonalLength;
	/*执行第一个水平度的顶点的计算*/
	LevelSaliency(1, Epsilon, vertexCnt, LocalMax[0], Points, meanCurvature, Saliency[2], &SmoothFactor[2]);
}

void MeshSaliency::WriteOneLevel(const char* SaliencyPath, const char* LocalMaxPath)
{
	/*水平度为1的前30%的显著性点的输出*/
	FILE* fp = NULL;//需要注意
	fp = fopen(SaliencyPath, "w");  //创建文件
	if (NULL == fp)
		return ;//要返回错误代码
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
	fp = NULL;//需要指向空，否则会指向原打开文件地址
	delete[] SortSaliency;

	/*水平度为1的局部最大值点的输出*/
	fp = fopen(LocalMaxPath, "w");  //创建文件
	if (NULL == fp)
		return;//要返回错误代码
	for (int i = 0; i < vertexCnt; i++)
	{
		if (LocalMax[0][i])   //true 是显著性点就输出该点
		{
			fprintf(fp, "%.6f %.6f %.6f\n", Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
		}
	}
	fclose(fp);
	fp = NULL;//需要指向空，否则会指向原打开文件地址
}

void MeshSaliency::WriteOneLevel(const char* SaliencyAndLocalMaxPath)
{
	/*水平度为1的前10%的显著性点和局部最大值点的输出*/
	FILE* fp = NULL;//需要注意
	fp = fopen(SaliencyAndLocalMaxPath, "w");  //创建文件
	if (NULL == fp)
		return;//要返回错误代码
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
		if (LocalMax[0][i])   //true 是显著性点就输出该点
		{
			fprintf(fp, "%.6f %.6f %.6f\n", Points[i * 3], Points[i * 3 + 1], Points[i * 3 + 2]);
		}
	}
	fclose(fp);
	fp = NULL;//需要指向空，否则会指向原打开文件地址
}

/*-----------------移值所用，仅一个Level的显著点--------------------------------*/


/*------------------------------VECTOR FUNCTIONS------------------------------*/
vec3::vec3() {}

/*
 * 向量的初始化
 */
vec3::vec3(float x, float y, float z) {
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

/*
 * 向量相加
 */
vec3 vec3::operator+ (const vec3& rhs) {
	vec3 vc;
	vc.v[0] = v[0] + rhs.v[0];
	vc.v[1] = v[1] + rhs.v[1];
	vc.v[2] = v[2] + rhs.v[2];
	return vc;
}

/*
 * 向量相加，重载+=符号
 */
vec3& vec3::operator+= (const vec3& rhs) {
	v[0] += rhs.v[0];
	v[1] += rhs.v[1];
	v[2] += rhs.v[2];
	return *this; // return self
}

/*
 * 向量相减
 */
vec3 vec3::operator- (const vec3& rhs) {
	vec3 vc;
	vc.v[0] = v[0] - rhs.v[0];
	vc.v[1] = v[1] - rhs.v[1];
	vc.v[2] = v[2] - rhs.v[2];
	return vc;
}

/*
 * 向量相减，重载-=
 */
vec3& vec3::operator-= (const vec3& rhs) {
	v[0] -= rhs.v[0];
	v[1] -= rhs.v[1];
	v[2] -= rhs.v[2];
	return *this;
}

/*
 * 向量的每个元素加上一个常数
 */
vec3 vec3::operator+ (float rhs) {
	vec3 vc;
	vc.v[0] = v[0] + rhs;
	vc.v[1] = v[1] + rhs;
	vc.v[2] = v[2] + rhs;
	return vc;
}

/*
 * 向量的每个元素减去一个常数
 */
vec3 vec3::operator- (float rhs) {
	vec3 vc;
	vc.v[0] = v[0] - rhs;
	vc.v[1] = v[1] - rhs;
	vc.v[2] = v[2] - rhs;
	return vc;
}

/*
 * 向量的每个元素乘上一个常数
 */
vec3 vec3::operator* (float rhs) {
	vec3 vc;
	vc.v[0] = v[0] * rhs;
	vc.v[1] = v[1] * rhs;
	vc.v[2] = v[2] * rhs;
	return vc;
}

/*
 * 向量的每个元素除以一个常数
 */
vec3 vec3::operator/ (float rhs) {
	vec3 vc;
	vc.v[0] = v[0] / rhs;
	vc.v[1] = v[1] / rhs;
	vc.v[2] = v[2] / rhs;
	return vc;
}

/*
 * 向量的每个元素乘上一个常数, 重载*=
 */
vec3& vec3::operator*= (float rhs) {
	v[0] = v[0] * rhs;
	v[1] = v[1] * rhs;
	v[2] = v[2] * rhs;
	return *this;
}

/*
 * 将一个向量赋值给另一个向量
 */
vec3& vec3::operator= (const vec3& rhs) {
	v[0] = rhs.v[0];
	v[1] = rhs.v[1];
	v[2] = rhs.v[2];
	return *this;
}

/*
 * 向量之间的差乘
 */
vec3 cross(const vec3& a, const vec3& b) {
	float x = a.v[1] * b.v[2] - a.v[2] * b.v[1];
	float y = a.v[2] * b.v[0] - a.v[0] * b.v[2];
	float z = a.v[0] * b.v[1] - a.v[1] * b.v[0];
	return vec3(x, y, z);
}

/*-----------------------------MATRIX FUNCTIONS-------------------------------*/
mat3::mat3() {}

/* 注意事项：案列的方式输入矩阵 */
mat3::mat3(float a, float b, float c,
	float d, float e, float f,
	float g, float h, float i) {
	m[0] = a; m[1] = b; m[2] = c;
	m[3] = d; m[4] = e; m[5] = f;
	m[6] = g; m[7] = h; m[8] = i;
}

/*
 * 初始化全零3阶矩阵
 */
mat3 zero_mat3() {
	return mat3(
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f
	);
}

/*
 * 初始化3阶单位矩阵
 */
mat3 identity_mat3() {
	return mat3(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	);
}

/*
 * 1X3的向量与3X3的向量相乘
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
 * 3X3的矩阵相乘
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
 * 矩阵与常数相乘
 */
mat3 mat3::operator* (const float& rhs) {
	mat3 r = zero_mat3();
	for (int i = 0; i < 9; i++)
		r.m[i] = rhs * m[i];
	return r;
}

/*
 * 矩阵与常数相加
 */
mat3 mat3::operator+ (const mat3& rhs) {
	mat3 r = zero_mat3();
	for (int i = 0; i < 9; i++)
		r.m[i] = m[i] + rhs.m[i];
	return r;
}

/*
 * 矩阵与常数相减
 */
mat3 mat3::operator- (const mat3& rhs) {
	mat3 r = zero_mat3();
	for (int i = 0; i < 9; i++)
		r.m[i] = m[i] - rhs.m[i];
	return r;
}
/*
 * 矩阵赋值
 */
mat3& mat3::operator= (const mat3& rhs) {
	for (int i = 0; i < 9; i++) {
		m[i] = rhs.m[i];
	}
	return *this;
}

/*
 * 两个向量相乘，变为3 * 3的矩阵
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
 * 求向量的长度
 */
float length(const vec3& v) {
	return sqrt(v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2]);
}

/*
 * 对向量归一化
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
 * 向量之间的点乘求积
 */
float dot(const vec3& a, const vec3& b) {
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

/*
 * 求两个三维坐标点之间的距离
 */
float get_squared_dist(vec3 from, vec3 to) {
	float x = (to.v[0] - from.v[0]) * (to.v[0] - from.v[0]);
	float y = (to.v[1] - from.v[1]) * (to.v[1] - from.v[1]);
	float z = (to.v[2] - from.v[2]) * (to.v[2] - from.v[2]);
	return x + y + z;
}

/*
 * 求两个三维坐标点之间的欧几里得距离
 */
float get_Euc_dist(vec3 from, vec3 to)
{
	float x = (to.v[0] - from.v[0]) * (to.v[0] - from.v[0]);
	float y = (to.v[1] - from.v[1]) * (to.v[1] - from.v[1]);
	float z = (to.v[2] - from.v[2]) * (to.v[2] - from.v[2]);
	return std::sqrt(x + y + z);
}

/*
 * 对矩阵进行转置
 */
mat3 transpose(const mat3& mm) {
	return mat3(
		mm.m[0], mm.m[3], mm.m[6],
		mm.m[1], mm.m[4], mm.m[7],
		mm.m[2], mm.m[5], mm.m[8]
	);
}


/*-------二阶矩阵的运算----------------*/
mat2::mat2() {}

/* 注意事项：案列的方式输入矩阵 */
mat2::mat2(float a, float b, 
	float c,float d) {
	m[0] = a; m[1] = b; 
	m[2] = c; m[3] = d; 
}

/*
 * 初始化全零2阶矩阵
 */
mat2 zero_mat2() {
	return mat2(
		0.0f, 0.0f,
		0.0f, 0.0f
	);
}

/*
 * 初始化3阶单位矩阵
 */
mat2 identity_mat2() {
	return mat2(
		1.0f, 0.0f, 
		0.0f, 1.0f
	);
}

/*
 * 2X2的矩阵相乘
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
 * 矩阵与常数相乘
 */
mat2 mat2::operator* (const float& rhs) {
	mat2 r = zero_mat2();
	for (int i = 0; i < 4; i++)
		r.m[i] = rhs * m[i];
	return r;
}

/*
 * 矩阵与常数相加
 */
mat2 mat2::operator+ (const mat2& rhs) {
	mat2 r = zero_mat2();
	for (int i = 0; i < 4; i++)
		r.m[i] = m[i] + rhs.m[i];
	return r;
}

/*
 * 矩阵与常数相减
 */
mat2 mat2::operator- (const mat2& rhs) {
	mat2 r = zero_mat2();
	for (int i = 0; i < 4; i++)
		r.m[i] = m[i] - rhs.m[i];
	return r;
}

/*
 * 矩阵赋值
 */
mat2& mat2::operator= (const mat2& rhs) {
	for (int i = 0; i < 4; i++) {
		m[i] = rhs.m[i];
	}
	return *this;
}

/*
 * 将显著性在前30%的点输出
 */
void MeshSaliency::WriteSaliencyPoints(const char* filepath)
{
	std::ofstream outfile;   //输出流
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
 * 将各个水平度显著性在前30%的点输出
 */
void MeshSaliency::WriteLevelPoints(std::string filePath[])
{
	for (int index = 0; index < 5; index++)
	{
		std::ofstream outfile;   //输出流
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
 * 将各个显著度下的局部显著性最大值的点输出
 */
void MeshSaliency::WriteLocalMaxPoints(std::string filePath[])
{
	for (int index = 0; index < 5; index++)
	{
		std::ofstream outfile;   //输出流
		outfile.open(filePath[index].c_str(), std::ios::app);
		if (!outfile.is_open())
			std::cout << "Open file failure" << std::endl;

		for (int i = 0; i < vertexCnt; i++)
		{
			if (LocalMax[index][i])   //true 是显著性点就输出该点
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
 *  主要更改：从基于顶点的遍历，更改为基于面的遍历，减少重复的面的面积的计算过程，加快运算速度
              本更改去除原有的构建网格的过程，网格的显示由VTK完成，只提取显著性的计算
 */
void MeshSaliency::ComputeSaliency2()
{
	ComputeMeanCurvature2();
}


/*
 * 将基于顶点遍历求解曲率的过程，更改为基于面的遍历求解曲率的过程，加快速度
 */
void MeshSaliency::ComputeMeanCurvature2()
{

}

/*
 * 返回与顶点相关联的重心双单元格的区域
 * 遍历三角网格的三条边，一次
 */
float MeshSaliency::dualArea()
{

	return 0;
}