#pragma once
#include "stdafx.h"


class myResliceCube {
private:
	class OnAfterRenderCallback : public vtkCommand {
		myResliceCube *owner;
		void SaveDebug();
	public:
		static OnAfterRenderCallback* New() { return new OnAfterRenderCallback(); }
		void SetOwner(myResliceCube *o) { owner = o; }
		void Execute(vtkObject * caller, unsigned long event, void* calldata);
	};

	bool gotTheInitialCameraDistance;
	double *boundsDoVolume;
	vtkRenderer *rendererLayerCubo, *rendererLayerImagem;
	vtkImageImport *imageSource;
	vtkSmartPointer<vtkActor> cubeActor;
	vtkSmartPointer<vtkImageSlabReslice> resliceFilter;
	vtkSmartPointer<vtkImageMapToWindowLevelColors> windowLevelFilter;
	vtkSmartPointer<vtkMatrix4x4> resliceMatrix;
	vtkSmartPointer<vtkActor> planeActor;
	vtkSmartPointer<vtkTexture> texture;
	void MakeCameraFollowTranslation();
	void SetEverything();
	void CreateReslice();
	void CreateCubeGeometry();
	void CreateWindowLevelFilter();
	void CreateImageGeometry();
	vtkSmartPointer<OnAfterRenderCallback> callbackDeModificacaoDoCubo;
	static inline std::array<double, 2> CalculateResliceExtent(myResliceCube *rc, std::array<std::array<double,3>,4>& vetores );
	static inline std::pair<std::array<double, 3>, double> MarchVectorUntilBorder(std::array<double, 3> c, std::array<double, 3> v, double* bounds);
	static inline bool isInsideBounds(std::array<double, 3> p, double bounds[6]);
public:
	myResliceCube();
	void SetRenderers(vtkRenderer *rc, vtkRenderer *ri);
	void SetSource(vtkImageImport* stc);
	void SetBoundsDoVolume(double * b);
	vtkSmartPointer<vtkImageSlabReslice> GetResliceFilter();
	vtkSmartPointer<vtkImageMapToWindowLevelColors> GetWindowLevelFilter();
	void SetWindowAndLevel(double w, double l);
};