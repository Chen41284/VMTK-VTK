#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkXMLPolyDataWriter.h>
#include <iostream>

vtkPolyData * custom_reader2(std::istream & infile)
{
	vtkIdType number_of_points, number_of_triangles;
	infile >> number_of_points >> number_of_triangles;

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
		polys->InsertCellPoint(a - 1);
		polys->InsertCellPoint(b - 1);
		polys->InsertCellPoint(c - 1);
	}
	vtkPolyData * polydata = vtkPolyData::New();
	polydata->SetPoints(points);
	polydata->SetPolys(polys);
	return polydata;
}

int main_1(int argc, char * argv[])
{
	
	std::string inputFilename = "data\\STL\\WU_AO_30K.txt";

	std::ifstream fin(inputFilename.c_str());

	vtkSmartPointer<vtkPolyData> polydata
		= vtkSmartPointer<vtkPolyData>::Take(
			custom_reader2(fin));

	/*vtkSmartPointer<vtkXMLPolyDataWriter> writer =
		vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	writer->SetInputData(polydata);
	writer->SetFileName("C:\\Users\\chenjiaxing\\Desktop\\Test\\x.vtp");
	writer->Write();*/

	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polydata);
	mapper->SetScalarRange(0, 1249);

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