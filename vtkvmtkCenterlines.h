/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkCenterlines,v $
Language:  C++
Date:      $Date: 2019/04/29 17:35:47 $
Version:   $Revision: 1.0 $

Modified: adapted from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkcenterlines.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __vtkvmtkCenterlines_H
#define __vtkvmtkCenterlines_H

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkAlgorithm.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkSetGet.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkActor.h>
#include <vtkIdList.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>
#include <set>

//VMTK-NEW
#include "vtkvmtkRenderer.h"

//make SeedSelector a separate pype script to be used in other contexts
class vtkvmtkSeedSelector : public vtkAlgorithm
{
public:
	static vtkvmtkSeedSelector *New();
	vtkTypeMacro(vtkvmtkSeedSelector, vtkAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//Set/Get the InputText
	vtkSetStringMacro(InputText);
	vtkGetStringMacro(InputText);

	//Set/Get the OutputText
	vtkSetStringMacro(OutputText);
	vtkGetStringMacro(OutputText);

	//Set/Get the InputInfo
	vtkSetStringMacro(InputInfo);
	vtkGetStringMacro(InputInfo);

	//Set/Get the _Surface
	void Set_Surface(vtkSmartPointer<vtkPolyData> _Surface) { this->_Surface = _Surface; };
	vtkSmartPointer<vtkPolyData> Get_Surface(void) { return this->_Surface; };

	//Get the  _SourceSeedIds / _TargetSeedIds
	vtkSmartPointer<vtkIdList> Get_SourceSeedIds(void) { return this->_SourceSeedIds; };
	vtkSmartPointer<vtkIdList> Get_TargetSeedIds(void) { return this->_TargetSeedIds; };

	virtual void Execute(void) {};

protected:
	vtkvmtkSeedSelector();
	~vtkvmtkSeedSelector();
	vtkSmartPointer<vtkPolyData> _Surface;
	vtkSmartPointer<vtkIdList> _SeedIds;
	vtkSmartPointer<vtkIdList> _SourceSeedIds;
	vtkSmartPointer<vtkIdList> _TargetSeedIds;
	//self.PrintError = None;
	//self.PrintLog = None;
	char* InputText;
	char* OutputText;
	char* InputInfo;

private:
	vtkvmtkSeedSelector(const vtkvmtkSeedSelector&) = delete;
	void operator== (const vtkvmtkSeedSelector&) = delete;
};

class vtkvmtkIdListSeedSelector : public vtkvmtkSeedSelector
{
public:
	static vtkvmtkIdListSeedSelector *New();
	vtkTypeMacro(vtkvmtkIdListSeedSelector, vtkvmtkSeedSelector);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	void Execute(void);

	//Set SourceIds / TargetIds
	void SetSourceIds(vtkSmartPointer<vtkIdList> SourceIds) { this->SourceIds = SourceIds; };
	void SetTargetIds(vtkSmartPointer<vtkIdList> TargetIds) { this->TargetIds = TargetIds; };

protected:
	vtkvmtkIdListSeedSelector();
	~vtkvmtkIdListSeedSelector();
	vtkSmartPointer<vtkIdList> SourceIds;
	vtkSmartPointer<vtkIdList> TargetIds;

private:
	vtkvmtkIdListSeedSelector(const vtkvmtkIdListSeedSelector&) = delete;
	void operator== (const vtkvmtkIdListSeedSelector&) = delete;
};

class vtkvmtkPointListSeedSelector : public vtkvmtkSeedSelector
{
public:
	static vtkvmtkPointListSeedSelector *New();
	vtkTypeMacro(vtkvmtkPointListSeedSelector, vtkvmtkSeedSelector);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	void Execute(void);

protected:
	vtkvmtkPointListSeedSelector();
	~vtkvmtkPointListSeedSelector();
	std::vector<double> SourcePoints;
	std::vector<double> TargetPoints;

private:
	vtkvmtkPointListSeedSelector(const vtkvmtkPointListSeedSelector&) = delete;
	void operator== (const vtkvmtkPointListSeedSelector&) = delete;
};

#endif