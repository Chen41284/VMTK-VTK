#include "vtkvmtkSurfaceViewer.h"

//VTK-Include

//STD

vtkStandardNewMacro(vtkvmtkSurfaceViewer);

vtkvmtkSurfaceViewer::vtkvmtkSurfaceViewer()
{
	this->Surface = nullptr;
	this->vmtkRenderer = nullptr;
	this->OwnRenderer = false;
	this->Display = false;
	this->Opacity = 1.0;
	this->ArrayName = nullptr;
	this->ScalarRange[0] = 0.0; this->ScalarRange[1] = 0.0;
	this->ColorMap = new char[strlen("cooltowarm")+1];
	strcpy_s(this->ColorMap, strlen("cooltowarm")+1, "cooltowarm");
	this->NumberOfColors = 256;
	this->Legend = false;
	this->LegendTitle = nullptr;
	this->Grayscale = false;
	this->FlatInterpolation = false;
	this->DisplayCellData = false;
	this->Color[0] = -1.0; this->Color[1] = -1.0; this->Color[2] = -1.0;
	this->LineWidth = 1;
	this->Representation = new char[strlen("surface")+1];
	strcpy_s(this->Representation, strlen("surface") + 1, "surface");
	this->DisplayTag = false;
	this->RegionTagArrayName = new char[strlen("RegionTagArray")+1];
	strcpy_s(this->RegionTagArrayName, strlen("RegionTagArray") + 1, "RegionTagArray");
	this->NumberOfRegions = 0;
}

vtkvmtkSurfaceViewer::~vtkvmtkSurfaceViewer()
{
	delete[] this->ColorMap;
	this->ColorMap = nullptr;
	delete[] this->Representation;
	this->Representation = nullptr;
	delete[] this->RegionTagArrayName;
	this->RegionTagArrayName = nullptr;
	if (this->ArrayName != nullptr)
	{
		delete[] this->ArrayName;
		this->ArrayName = nullptr;
	}
	if (this->LegendTitle != nullptr)
	{
		delete[] this->LegendTitle;
		this->LegendTitle = nullptr;
	}
	if (this->vmtkRenderer != nullptr)
	{
		this->vmtkRenderer->Delete();
		this->vmtkRenderer = nullptr;
	}
}

void vtkvmtkSurfaceViewer::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Surface Viewer\n";
}