#include "vtkvmtkCenterlines.h"

//VTK

//STD

//VMTK-Src

//VMTK-New


//Class vtkvmtkSeedSelector
vtkStandardNewMacro(vtkvmtkSeedSelector);
vtkvmtkSeedSelector::vtkvmtkSeedSelector()
{
	this->_Surface = nullptr;
	this->_SeedIds = nullptr;
	this->_SourceSeedIds = vtkSmartPointer<vtkIdList>::New();
	this->_TargetSeedIds = vtkSmartPointer<vtkIdList>::New();
	//self.PrintError = None;
	//self.PrintLog = None;
	this->InputText = nullptr;
	this->OutputText = nullptr;
	this->InputInfo = nullptr;
}

vtkvmtkSeedSelector::~vtkvmtkSeedSelector()
{
	if (this->InputText != nullptr)
	{
		delete[] this->InputText;
		this->InputText = nullptr;
	}
	if (this->OutputText != nullptr)
	{
		delete[] this->OutputText;
		this->OutputText = nullptr;
	}
	if (this->InputInfo != nullptr)
	{
		delete[] this->InputInfo;
		this->InputInfo = nullptr;
	}
}

void vtkvmtkSeedSelector::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Seed Selector\n";
}

//-----------------------------------------------------
//class vtkvmtkIdListSeedSelector
vtkStandardNewMacro(vtkvmtkIdListSeedSelector);

vtkvmtkIdListSeedSelector::vtkvmtkIdListSeedSelector() : vtkvmtkSeedSelector()
{
	//默认调用父类的无参构造函数
	this->SourceIds = nullptr;
	this->TargetIds = nullptr;
}


vtkvmtkIdListSeedSelector::~vtkvmtkIdListSeedSelector()
{
	//
}

void vtkvmtkIdListSeedSelector::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Id List Seed Selector\n";
}

void vtkvmtkIdListSeedSelector::Execute(void)
{
	if (this->_Surface == nullptr)
	{
		std::cout << "vmtkIdListSeedSelector Error: Surface not set." << std::endl;
		return;
	}
	if (this->SourceIds == nullptr)
	{
		std::cout << "vmtkIdListSeedSelector Error: SourceIds not set." << std::endl;
		return;
	}
	if (this->TargetIds == nullptr)
	{
		std::cout << "vmtkIdListSeedSelector Error: TargetIds not set." << std::endl;
		return;
	}
	this->_SourceSeedIds->Initialize();
	this->_TargetSeedIds->Initialize();

	int maxId = this->_Surface->GetNumberOfPoints() - 1;
	//SourceIds
	for (int i = 0; i < this->SourceIds->GetNumberOfIds(); ++i)
	{
		int id = this->SourceIds->GetId(i);
		if (id > maxId)
		{
			std::cout << "vmtkIdListSeedSelector Error: invalid SourceId." << std::endl;
			return;
		}
		this->_SourceSeedIds->InsertNextId(id);
	}
	//TargetIds
	for (int i = 0; i < this->TargetIds->GetNumberOfIds(); ++i)
	{
		int id = this->TargetIds->GetId(i);
		if (id > maxId)
		{
			std::cout << "vmtkIdListSeedSelector Error: invalid TargetId." << std::endl;
			return;
		}
		this->_TargetSeedIds->InsertNextId(id);
	}
}

//--------------------------------------------------------------------
//class vtkvmtkPointListSeedSelector
vtkStandardNewMacro(vtkvmtkPointListSeedSelector);

vtkvmtkPointListSeedSelector::vtkvmtkPointListSeedSelector() : vtkvmtkSeedSelector()
{
	
}

vtkvmtkPointListSeedSelector::~vtkvmtkPointListSeedSelector()
{

}

void vtkvmtkPointListSeedSelector::PrintSelf(ostream& os, vtkIndent indent)
{

}

void vtkvmtkPointListSeedSelector::Execute()
{

}