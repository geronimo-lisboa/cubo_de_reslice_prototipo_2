#pragma once
#include "stdafx.h"

class myResliceCube {
private:
	vtkRenderer *rendererLayerCubo, *rendererLayerImagem;
	vtkImageImport *imageSource;
	vtkSmartPointer<vtkActor> cubeActor;
	void SetEverything();
public:
	myResliceCube();
	void SetRenderers(vtkRenderer *rc, vtkRenderer *ri);
	void SetSource(vtkImageImport* stc);

};