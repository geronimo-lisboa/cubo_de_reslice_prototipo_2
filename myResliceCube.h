#pragma once
#include "stdafx.h"

class myResliceCube {
private:
	vtkRenderer *rendererLayerCubo, *rendererLayerImagem;
	vtkImageImport *imageSource;
	vtkSmartPointer<vtkActor> cubeActor;
	vtkSmartPointer<vtkImageSlabReslice> resliceFilter;
	void SetEverything();
	void CreateReslice();
	void CreateCubeGeometry();
	void CreateWindowLevelFilter();
	void CreateImageGeometry();
public:
	myResliceCube();
	void SetRenderers(vtkRenderer *rc, vtkRenderer *ri);
	void SetSource(vtkImageImport* stc);

};