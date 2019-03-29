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
#include <vtkPLYWriter.h>
#include <vtkPLYReader.h>
#include<cstdio>


//VTK_MODULE_INIT(vtkRenderingOpenGL); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);

// For compatibility with new VTK generic data arrays
#ifdef vtkGenericDataArray_h
#define InsertNextTupleValue InsertNextTypedTuple
#endif

vtkPolyData * Triangle_Reader(std::string & filePath, int &vertices, int &faces)
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

/*
 * �����ǰ��smf  #$SMF 1.0 ��ͷ
 */
vtkPolyData *Smf_Reader1(const char* filePath, int &vertices, int &faces)
{
	std::ifstream infile(filePath);
	char SmfVersion[20];
	float version;
	infile >> SmfVersion >> version;
	char verticesName[20]; int verticesNumber;
	infile >> verticesName >> verticesNumber;
	char facesName[20]; int facesNumber;
	infile >> facesName >> facesNumber;

	std::cout << SmfVersion << "  " << version << std::endl;
	std::cout << verticesName << "  " << verticesNumber << std::endl;
	std::cout << facesName << "  " << facesNumber << std::endl;

	vtkIdType number_of_points, number_of_triangles;
	number_of_points = verticesNumber;
	number_of_triangles = facesNumber;
	vertices = number_of_points;   //���涥�������
	faces = number_of_triangles;   //���������ε�����

	vtkSmartPointer<vtkPoints> points
		= vtkSmartPointer<vtkPoints>::New();
	points->SetNumberOfPoints(number_of_points);
	for (vtkIdType i = 0; i < number_of_points; i++)
	{
		char v[4];
		infile >> v; //pass first v
		double x, y, z;
		infile >> x >> y >> z;
		points->SetPoint(i, x, y, z);
	}

	vtkSmartPointer<vtkCellArray> polys
		= vtkSmartPointer<vtkCellArray>::New();
	for (vtkIdType i = 0; i < number_of_triangles; i++)
	{
		char f[4];
		infile >> f;  //pass f
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

/*
 * ����򻯺��smf  begin��ͷ end��β
 */
vtkPolyData *Smf_Reader2(const char* filePath, int &vertices, int &faces)
{
	std::ifstream infile(filePath);
	char begin[10];
	infile >> begin;    //������ͷ

	vtkIdType IDi = 0;
	vtkIdType IDj = 0;

	vtkSmartPointer<vtkPoints> points
		= vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> polys
		= vtkSmartPointer<vtkCellArray>::New();
	bool flag = true;
	while (flag)
	{
		char first[5];   //��¼ÿ�еĿ�ͷ���ж��ǲ���end
		infile >> first;
		if (first[0] == 'v') //��������
		{
			double x, y, z;
			infile >> x >> y >> z;
			points->InsertPoint(IDi, x, y, z);
			IDi++;
		}
		else if (first[0] == 'f')  //�����������
		{
			IDj++;
			vtkIdType a, b, c;
			infile >> a >> b >> c;
			polys->InsertNextCell(3);
			polys->InsertCellPoint(a - 1);      //smf�Ķ����Ŵ�1��ʼ
			polys->InsertCellPoint(b - 1);
			polys->InsertCellPoint(c - 1);
		}
		else  //end��β
		{
			flag = false;   //����ѭ��
		}
	}

	vtkPolyData * polydata = vtkPolyData::New();
	polydata->SetPoints(points);
	polydata->SetPolys(polys);

	infile.close();
	vertices = IDi;    //���涥�������
	faces = IDj;       //���������ε�����
	return polydata;
}

/*
 * smf�ļ���1��ʼ
 */
int Ply2Smf(const char* sourcePlyPath, const char* tmpBefore, int &facesNumber)
{
	vtkSmartPointer<vtkPLYReader> reader =
		vtkSmartPointer<vtkPLYReader>::New();
	reader->SetFileName(sourcePlyPath);
	reader->Update();
	vtkSmartPointer<vtkPolyData> polydata = reader->GetOutput();
	FILE*fp = NULL;//��Ҫע��
	fp = fopen(tmpBefore, "w");  //�����ļ�
	if (NULL == fp) 
		return -1;//Ҫ���ش������
	/*std::cout << "polydata points: " << polydata->GetNumberOfPoints() << std::endl;
	std::cout << "polydata cells: " << polydata->GetNumberOfCells() << std::endl;
	std::cout << "Points " << std::endl;
	 = polydata->GetCell(0);
	std::cout << cell->GetNumberOfPoints() << std::endl;*/
	facesNumber = polydata->GetNumberOfCells();
	fprintf(fp, "#$SMF 1.0\n");
	fprintf(fp, "#$vertices %d\n#$faces %d\n", polydata->GetNumberOfPoints(), polydata->GetNumberOfCells());
	for (vtkIdType i = 0; i < polydata->GetNumberOfPoints(); i++)
	{
		double p[3];
		polydata->GetPoint(i, p);
		fprintf(fp, "v %.6f %.6f %.6f\n", p[0], p[1], p[2]);
	}
	for (vtkIdType i = 0; i < polydata->GetNumberOfCells(); i++)
	{
		vtkSmartPointer<vtkCell> cell = polydata->GetCell(i);
		int Id1 = cell->GetPointId(0) + 1;
		int Id2 = cell->GetPointId(1) + 1;
		int Id3 = cell->GetPointId(2) + 1;
		fprintf(fp, "f %d %d %d\n", Id1, Id2, Id3);
	}
	std::cout << "Ply2smfdone!" << std::endl;
	fclose(fp);
	fp = NULL;//��Ҫָ��գ������ָ��ԭ���ļ���ַ
	return 0;
}

/*
 * �����
 */
void Simply(int targetFaces)
{
	std::string cmd = "qslim.exe -o tmpAfter.smf -t  " + std::to_string(targetFaces) + " tmpBefore.smf";
	//system("qslim.exe -o tmpAfter.smf -t  50000 tmpBefore.smf ");
	system(cmd.c_str());
	std::cout << "Simplydone!" << std::endl;
}

/*
 * ���PLYģ��
 */
void WritePly(const char* tmpAfter, const char* destPath)
{
	int vertices = 0;
	int faces = 0;
	vtkSmartPointer<vtkPolyData> polydata
		= vtkSmartPointer<vtkPolyData>::Take(
			Smf_Reader2(tmpAfter, vertices, faces));
	// Write ply file
	vtkSmartPointer<vtkPLYWriter> plyWriter =
		vtkSmartPointer<vtkPLYWriter>::New();
	plyWriter->SetFileName(destPath);
	plyWriter->SetInputData(polydata);
	plyWriter->Write();
}

/*����򻯵ĺ�������*/
void PlySimply(const char* sourcePlyPath, const char* destPlyPath, float SimpliedFactor)
{
	int facesNumber = 0;
	const char* tmpBefore = "tmpBefore.smf";
	const char* tmpAfter = "tmpAfter.smf";
	Ply2Smf(sourcePlyPath, tmpBefore, facesNumber);
	std::cout << facesNumber << std::endl;
	int targetFaces = ceil(facesNumber * SimpliedFactor);
	std::cout << targetFaces << std::endl;
	Simply(targetFaces);
	WritePly(tmpAfter, destPlyPath);
	remove(tmpBefore);
	remove(tmpAfter);
	std::cout << "done!" << std::endl;
}

int main_ReadSmf(int argc, char * argv[])
{
	const char* inputFilename = "C:\\Users\\chenjiaxing\\Desktop\\HeartModel50000.smf";
	const char* plyfile = "C:\\Users\\chenjiaxing\\Desktop\\TestSmf.ply";
	/*��ǰ��ȡ����ö�����������������*/
	int vertices = 0;
	int faces = 0;
	vtkSmartPointer<vtkPolyData> polydata
		= vtkSmartPointer<vtkPolyData>::Take(
			Smf_Reader2(inputFilename, vertices, faces));

	std::cout << "vertices number " << vertices << std::endl;
	std::cout << "faces number " << faces << std::endl;

	// Write ply file
	vtkSmartPointer<vtkPLYWriter> plyWriter =
		vtkSmartPointer<vtkPLYWriter>::New();
	plyWriter->SetFileName(plyfile);
	plyWriter->SetInputData(polydata);
	plyWriter->Write();

	// Create a sphere
	vtkSmartPointer<vtkNamedColors> colors =
		vtkSmartPointer<vtkNamedColors>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polydata);

	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(actor);
	renderer->SetBackground(.3, .6, .3); // Background color green

	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}

int main_Triangle(int argc, char * argv[])
{
	const char* plyBefore = "C:\\Users\\chenjiaxing\\Desktop\\HeartModel.ply";
	const char* plyAfter = "C:\\Users\\chenjiaxing\\Desktop\\HeartModel01.ply";
	float factor = 0.1f;
	PlySimply(plyBefore, plyAfter, factor);
	getchar();
	return 0;
}