#include "stdafx.h"
#include "myResliceCube.h"


void myResliceCube::SetEverything()
{
	if (!rendererLayerCubo || !rendererLayerImagem || !imageSource)
		return;
	//cria a geometria do cubo
	CreateCubeGeometry();
	//Cria o filtro do reslice.
	CreateReslice();
	//Cria o filtro do window/level
	CreateWindowLevelFilter();
	//Cria a geometria que vai exibir o reslice na tela.
	CreateImageGeometry();
}


void myResliceCube::CreateReslice()
{
}

void myResliceCube::CreateCubeGeometry()
{
	vtkSmartPointer<vtkCubeSource> src = vtkSmartPointer<vtkCubeSource>::New();
	src->SetCenter(imageSource->GetOutput()->GetCenter());//O cubo começa no centro da imagem.
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(src->GetOutputPort());
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetRepresentationToWireframe();
	actor->GetProperty()->SetColor(1, 0, 0);
	rendererLayerCubo->AddActor(actor);
	rendererLayerCubo->ResetCamera();
	cubeActor = actor;
}

void myResliceCube::CreateWindowLevelFilter()
{
}

void myResliceCube::CreateImageGeometry()
{
}

myResliceCube::myResliceCube()
{
	resliceFilter = nullptr;
	rendererLayerCubo = nullptr;
	rendererLayerImagem = nullptr;
	imageSource = nullptr;
	cubeActor = nullptr;
}

void myResliceCube::SetRenderers(vtkRenderer * rc, vtkRenderer * ri)
{
	rendererLayerCubo = rc;
	rendererLayerImagem = ri;
	SetEverything();
}

void myResliceCube::SetSource(vtkImageImport * stc)
{
	imageSource = stc;
	SetEverything();
}
