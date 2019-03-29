#include "vtkAutoInit.h" 
#include <vtkVersion.h>
#include <vtkUnsignedCharArray.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkFloatArray.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include <vtkNamedColors.h>
#include <vtkProperty.h>
#include <vtkUnsignedCharArray.h>
#include <vtkLookupTable.h>
#include <vector>
#include <iostream>
#include "Mesh_Saliency.h"
#include <vtkFeatureEdges.h>

//VTK_MODULE_INIT(vtkRenderingOpenGL); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);

// For compatibility with new VTK generic data arrays
#ifdef vtkGenericDataArray_h
#define InsertNextTupleValue InsertNextTypedTuple
#endif

/*��ӡ����*/
template<typename T>
void Print(T *arr, int len, int wordsEachLine);

vtkPolyData * custom_reader(std::string & filePath, int &vertices, int &faces)
{
	std::ifstream infile(filePath.c_str());
	vtkIdType number_of_points, number_of_triangles;
	infile >> number_of_points >> number_of_triangles;
	vertices = number_of_points;   //���涥�������
	faces = number_of_triangles;   //���������ε�����

	vtkSmartPointer<vtkPoints> points
		= vtkSmartPointer<vtkPoints>::New();
	points->SetNumberOfPoints(number_of_points);
	for (vtkIdType i = 0; i < number_of_points; i++)
	{
		double x, y, z;
		infile >> x >> y >> z;
		points->SetPoint(i, x, y, z);
	}

	vtkSmartPointer<vtkCellArray> polys
		= vtkSmartPointer<vtkCellArray>::New();
	for (vtkIdType i = 0; i < number_of_triangles; i++)
	{
		vtkIdType a, b, c;
		infile >> a >> b >> c;
		polys->InsertNextCell(3);
		/*bumpy*/
		/*polys->InsertCellPoint(a);
		polys->InsertCellPoint(b);
		polys->InsertCellPoint(c);*/

		/*Armadillo*/
		polys->InsertCellPoint(a - 1);
		polys->InsertCellPoint(b - 1);
		polys->InsertCellPoint(c - 1);
	}
	vtkPolyData * polydata = vtkPolyData::New();
	polydata->SetPoints(points);
	polydata->SetPolys(polys);

	infile.close();
	return polydata;
}

/*��ȡ��ɫ������*/
float* readColorArray(std::string &filePath, int number)
{
	std::ifstream infile(filePath.c_str());
	float *ColorArray;
	ColorArray = new float[3 * number];
	for (int i = 0; i < number; i++)
	{
		float a, b, c;
		infile >> a >> b >> c; //pass
		infile >> ColorArray[i * 3] >> ColorArray[i * 3 + 1] >> ColorArray[i * 3 + 2]; //Read Color
	}
	infile.close();
	return ColorArray;
}

/*������ɫ������*/
float *createColorArray(std::string &SaliencyPath, int number)
{
	std::ifstream infile(SaliencyPath.c_str());
	float *ColorArray;
	ColorArray = new float[3 * number];
	for (int i = 0; i < number; i++)
	{
		float Saliency = 0;
		infile >> Saliency;
		if (Saliency <= 0.5)
		{
			ColorArray[i * 3] = 0;                 //Red
			ColorArray[i * 3 + 1] = Saliency * 2;   //Green
			ColorArray[i * 3 + 2] = 1;                 //Blue
		}
		else // > 0.5
		{
			//ColorArray[i * 3] = (Saliency - 1) * 2;     //Red
			//ColorArray[i * 3 + 1] = 1;                         //Green
			//ColorArray[i * 3 + 2] = (1 - Saliency) * 2;     //Blue
			ColorArray[i * 3] = (1 - Saliency) * 2;      //Red
			ColorArray[i * 3 + 1] = (1 - Saliency) * 2;    //Green
			ColorArray[i * 3 + 2] = 0;                       //Blue
		}		
	}
	infile.close();
	return ColorArray;
}

/*
 * ������ɫ���飬��MatLab��jet(256)������������ɫ����
 */
float *createColorArray(std::string &SaliencyPath, std::string &colorIndexPath, int number)
{
	/*������ɫ�������飬256*/
	std::ifstream inColor(colorIndexPath.c_str());
	float *ColorIndex = new float[3 * 256];
	for (int i = 0; i < 256; i++)
	{
		inColor >> ColorIndex[i * 3] >> ColorIndex[i * 3 + 1] >> ColorIndex[i * 3 + 2];
	}
	/*����ɫ���뵽������ɫ������*/
	std::ifstream infile(SaliencyPath.c_str());
	float *ColorArray;
	ColorArray = new float[3 * number];
	for (int i = 0; i < number; i++)
	{
		float Saliency = 0;
		infile >> Saliency;
		if (Saliency > 1 || Saliency < 0)
		{
			std::cerr << "�����������������ݳ���:ӦΪ[0 1]" << std::endl;
		}
		int index = ceil(Saliency * 256) - 1;

		ColorArray[i * 3] = ColorIndex[index * 3];                 //Red
		ColorArray[i * 3 + 1] = ColorIndex[index * 3 + 1];         //Green
		ColorArray[i * 3 + 2] = ColorIndex[index * 3 + 2];         //Blue
	}
	inColor.close();
	infile.close();
	return ColorArray;
}

/*
 * ������ɫ���飬��MatLab��jet(256)������������ɫ����
 */
float *createColorArray(float* SaliencyArray, std::string &colorIndexPath, int number)
{
	/*������ɫ�������飬256*/
	std::ifstream inColor(colorIndexPath.c_str());
	float *ColorIndex = new float[3 * 256];
	for (int i = 0; i < 256; i++)
	{
		inColor >> ColorIndex[i * 3] >> ColorIndex[i * 3 + 1] >> ColorIndex[i * 3 + 2];
	}
	/*����ɫ���뵽������ɫ������*/
	float *ColorArray;
	ColorArray = new float[3 * number];
	for (int i = 0; i < number; i++)
	{
		float Saliency = 0;
		Saliency = SaliencyArray[i];
		if (Saliency > 1 || Saliency < 0)
		{
			std::cerr << "�����������������ݳ���:ӦΪ[0 1]" << std::endl;
		}
		int index = ceil(Saliency * 256) - 1;

		ColorArray[i * 3] = ColorIndex[index * 3];                 //Red
		ColorArray[i * 3 + 1] = ColorIndex[index * 3 + 1];         //Green
		ColorArray[i * 3 + 2] = ColorIndex[index * 3 + 2];         //Blue
	}
	inColor.close();
	return ColorArray;
}

/*��ӡ����*/
template<typename T>
void Print(T *arr, int len, int wordsEachLine)
{
	for (int i = 0; i < len; i++)
	{
		cout << *(arr + i) << "  ";
		if ((i + 1) % wordsEachLine == 0)
			cout << endl;       //ÿ����Ԫ�����һ��
	}
}

/*��������������뵽*/
float* readglobalMax(std::string &filePath, int &number)
{
	std::ifstream infile(filePath.c_str());
	float *globalArray;
	infile >> number;
	globalArray = new float[3 * number];
	for (int i = 0; i < number; i++)
	{
		infile >> globalArray[i * 3] >> globalArray[i * 3 + 1] >> globalArray[i * 3 + 2]; //Read Color
	}
	infile.close();
	return globalArray;	
}


int main_Saliency(int argc, char * argv[])
{
	std::string ColorIndexPath = "data\\ColorMap\\MatlabRGB256.txt";
	std::string inputFilename = "data\\armadillo\\armadillo2.txt";
	std::string SaliencyfilePath = "data\\armadillo\\armadillo2.txt";
	const char* StoreSaliency = "C:\\Users\\chenjiaxing\\Desktop\\Armadilo_Taubin.txt.";
	std::string levelPath[5];
	levelPath[0] = "C:\\Users\\chenjiaxing\\Desktop\\Level\\1.txt";
	levelPath[1] = "C:\\Users\\chenjiaxing\\Desktop\\Level\\2.txt";
	levelPath[2] = "C:\\Users\\chenjiaxing\\Desktop\\Level\\3.txt";
	levelPath[3] = "C:\\Users\\chenjiaxing\\Desktop\\Level\\4.txt";
	levelPath[4] = "C:\\Users\\chenjiaxing\\Desktop\\Level\\5.txt";
	std::string LocalMaxPath[5];
	LocalMaxPath[0] = "C:\\Users\\chenjiaxing\\Desktop\\Points\\1.txt";
	LocalMaxPath[1] = "C:\\Users\\chenjiaxing\\Desktop\\Points\\2.txt";
	LocalMaxPath[2] = "C:\\Users\\chenjiaxing\\Desktop\\Points\\3.txt";
	LocalMaxPath[3] = "C:\\Users\\chenjiaxing\\Desktop\\Points\\4.txt";
	LocalMaxPath[4] = "C:\\Users\\chenjiaxing\\Desktop\\Points\\5.txt";
	/*�����Եļ���*/
	MeshSaliency mesh(SaliencyfilePath);
	mesh.setFaceReduceOne();  //Armadilo
	mesh.ComputeSaliency_MultiCore();
	int vertexCnt = mesh.getVertexNumber();
	float **SaliencyLevel = mesh.getSaliency();
	float *SmootheSaliency = mesh.getSmoothSaliency();
	mesh.WriteSaliencyPoints(StoreSaliency);
	mesh.WriteLevelPoints(levelPath);
	mesh.WriteLocalMaxPoints(LocalMaxPath);

	int count05 = 0;  //< 0.05
	int count05_1 = 0; // 0.05 - 0.1
	int count1_2 = 0;
	int count2_3 = 0;
	int count3_4 = 0;
	int count4_6 = 0;
	int count6_10 = 0;
	for (int index = 0; index < vertexCnt; index++)
	{
		if (SmootheSaliency[index] < 0.05)
		{
			count05++;
		}
		else if (SmootheSaliency[index] >= 0.05 && SmootheSaliency[index] <= 0.1)
		{
			//SmootheSaliency[index] *= 3;
			//SmootheSaliency[index] = 0.8;
			count05_1++;
		}
		else if (SmootheSaliency[index] > 0.1 && SmootheSaliency[index] <= 0.2)
		{
			//SmootheSaliency[index] *= 3;
			//SmootheSaliency[index] = 0.8;
			count1_2++;
		}
		else if (SmootheSaliency[index] > 0.2 && SmootheSaliency[index] <= 0.3)
		{
			//SmootheSaliency[index] *= 3;
			//SmootheSaliency[index] = 0.8;
			count2_3++;
		}
		else if (SmootheSaliency[index] > 0.3 && SmootheSaliency[index] <= 0.4)
		{
			//SmootheSaliency[index] *= 2;
			//SmootheSaliency[index] = 0.8;
			count3_4++;
		}
		else if (SmootheSaliency[index] >= 0.4 && SmootheSaliency[index] <= 0.6)
		{
			//SmootheSaliency[index] = 0.8;
			count4_6++;
		}
		else
		{
			//SmootheSaliency[index] = 0.8;
			//SmootheSaliency[index] = 0.9;
			count6_10++;
		}
	}

	std::cout << "SmoothSaliency in 0 - 0.05 " << count05 << std::endl;
	std::cout << "SmoothSaliency in 0.05 - 0.1 " << count05_1 << std::endl;
	std::cout << "SmoothSaliency in 0.1 - 0.2 " << count1_2 << std::endl;
	std::cout << "SmoothSaliency in 0.2 - 0.3 " << count2_3 << std::endl;
	std::cout << "SmoothSaliency in 0.3 - 0.4 " << count3_4 << std::endl;
	std::cout << "SmoothSaliency in 0.4 - 0.6 " << count4_6 << std::endl;
	std::cout << "SmoothSaliency in 0.6 - 1 " << count6_10 << std::endl;
	

	/*��ǰ��ȡ����ö�����������������*/
	int vertices = 0;
	int faces = 0;
	vtkSmartPointer<vtkPolyData> polydata
		= vtkSmartPointer<vtkPolyData>::Take(
			custom_reader(inputFilename, vertices, faces));

	std::cout << "vertices number " << vertices << std::endl;
	std::cout << "faces number " << faces << std::endl;

	std::string inputglobalMax = "data\\Armadillo\\NullPoints.txt";
	int n = 0;
	float *points = readglobalMax(inputglobalMax, n);
	std::cout << "n " << n << std::endl;
	//Print(points, n * 3, 3);

	std::string inputColor = "data\\Armadillo\\meshSaliency.txt";
	int number = vertices;   //���������
	//float *Color = readColorArray(inputColor, number);
	//float *Color = createColorArray(inputColor, number);
	//float *Color = createColorArray(inputColor, ColorIndexPath, number);
	float *Color = createColorArray(SmootheSaliency, ColorIndexPath, number);
	//Print(Color, number * 3, 3);

	// Create a sphere
	vtkSmartPointer<vtkNamedColors> colors =
		vtkSmartPointer<vtkNamedColors>::New();

	std::vector<vtkSmartPointer<vtkSphereSource>> spheres;
	std::vector<vtkSmartPointer<vtkPolyDataMapper>> mappers;
	std::vector<vtkSmartPointer<vtkActor>> actors;

	// Create a source, renderer, mapper, and actor
  // for each object
	for (unsigned int i = 0; i < n; i++)
	{
		spheres.push_back(vtkSmartPointer<vtkSphereSource>::New());
		spheres[i]->SetCenter(points[i * 3], points[i * 3 + 1], points[i * 3 + 2]);
		spheres[i]->SetRadius(0.1);
		// Make the surface smooth.
		spheres[i]->SetPhiResolution(10);
		spheres[i]->SetThetaResolution(10);

		mappers.push_back(vtkSmartPointer<vtkPolyDataMapper>::New());
		mappers[i]->SetInputConnection(
			spheres[i]->GetOutputPort());

		actors.push_back(vtkSmartPointer<vtkActor>::New());
		actors[i]->SetMapper(mappers[i]);
		actors[i]->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
	}

	// Visualize
    //������ɫӳ���
	//�洢����ֵ

	vtkFloatArray *scalars = vtkFloatArray::New();

	for (int i = 0; i < number; i++)
		scalars->InsertTuple1(i, i);
	polydata->GetPointData()->SetScalars(scalars);

	vtkLookupTable *pColorTable = vtkLookupTable::New();
	pColorTable->SetNumberOfColors(number);
	for (int i = 0; i < number; i++)
	{
		//pColorTable->SetTableValue(i, abs(Color[i * 3]), abs(Color[i * 3 + 1]), abs(Color[i * 3 + 2]), 1.0);
		pColorTable->SetTableValue(i, Color[i * 3], Color[i * 3 + 1], Color[i * 3 + 2]), 1.0;
	}
	pColorTable->Build();

	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polydata);
	mapper->SetLookupTable(pColorTable);
	mapper->SetScalarRange(0, number);   //�ǵø���

	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(actor);
	
	for (int i = 0; i < n; i++)
	{
		renderer->AddActor(actors[i]);
	}
	renderer->SetBackground(.3, .6, .3); // Background color green

	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	renderWindowInteractor->Start();

	delete[] points;

	return EXIT_SUCCESS;
}

mat2 ComputeCurvature(mat2 &Mvi)
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
	mat2 Result = sInver * Mvi * s;
	return Result;
}

void testFor()
{
	int count = 0;
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			if (j % 5 == 0)
			{
				count++;
				break;
			}
		}
	}
	std::cout << "count " << count << std::endl;
}

int main_Saliency()
{
	//std::string SaliencyfilePath = "data\\armadillo\\armadillo2.txt";
	const char* plyPath = "C:\\Users\\chenjiaxing\\Desktop\\TestSmf.ply";
	const char* levelPath = "C:\\Users\\chenjiaxing\\Desktop\\Level1.fp";
	const char* LocalMaxPath = "C:\\Users\\chenjiaxing\\Desktop\\LocalMax1.lp";
	const char* SaliencyAndLM = "C:\\Users\\chenjiaxing\\Desktop\\SaliencyLM.slp";
	/*�����Եļ���*/
	MeshSaliency mesh(plyPath);
	//mesh.setFaceReduceOne();  //Armadilo
	mesh.ComputeSaliency_OneLevel();
	mesh.WriteOneLevel(SaliencyAndLM);
	std::cout << "compute done!" << std::endl;
	getchar();
	return 0;
}
