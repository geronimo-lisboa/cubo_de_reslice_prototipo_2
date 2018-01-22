#pragma once
#include "stdafx.h"


class myResliceCube {
private:
	class OnAfterRenderCallback : public vtkCommand {
		myResliceCube *owner;
	public:
		static OnAfterRenderCallback* New() { return new OnAfterRenderCallback(); }
		void SetOwner(myResliceCube *o) { owner = o; }
		void Execute(vtkObject * caller, unsigned long event, void* calldata);
	};

	double *boundsDoVolume;
	vtkRenderer *rendererLayerCubo, *rendererLayerImagem;
	vtkImageImport *imageSource;
	vtkSmartPointer<vtkActor> cubeActor;
	vtkSmartPointer<vtkImageSlabReslice> resliceFilter;
	vtkSmartPointer<vtkMatrix4x4> resliceMatrix;
	void SetEverything();
	void CreateReslice();
	void CreateCubeGeometry();
	void CreateWindowLevelFilter();
	void CreateImageGeometry();
	vtkSmartPointer<OnAfterRenderCallback> callbackDeModificacaoDoCubo;
	static inline std::array<double, 2> CalculateResliceExtent(myResliceCube *rc);
	static inline std::pair<std::array<double, 3>, double> MarchVectorUntilBorder(std::array<double, 3> c, std::array<double, 3> v, double* bounds);
	static inline bool isInsideBounds(std::array<double, 3> p, double bounds[6]);
public:
	myResliceCube();
	void SetRenderers(vtkRenderer *rc, vtkRenderer *ri);
	void SetSource(vtkImageImport* stc);
	void SetBoundsDoVolume(double * b);

};