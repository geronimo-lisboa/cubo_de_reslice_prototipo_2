#include "stdafx.h"
#include "myResliceCube.h"


void myResliceCube::SetEverything()
{
	if (!rendererLayerCubo || !rendererLayerImagem || !imageSource || !boundsDoVolume)
		return;
	//cria a geometria do cubo
	CreateCubeGeometry();
	//Cria o filtro do reslice.
	CreateReslice();
	//Cria o filtro do window/level
	CreateWindowLevelFilter();
	//Cria a geometria que vai exibir o reslice na tela.
	CreateImageGeometry();
	//o callback de quando o cubo é rotacionado
	callbackDeModificacaoDoCubo = vtkSmartPointer<OnAfterRenderCallback>::New();
	rendererLayerCubo->AddObserver(vtkCommand::EndEvent, callbackDeModificacaoDoCubo);
}

bool myResliceCube::isInsideBounds(std::array<double, 3> p, double bounds[6]) {
	assert(bounds && "Tem que definir as bounds...");
	if ((bounds[0] < p[0] && p[0] < bounds[1]) &&
		(bounds[2] < p[1] && p[1] < bounds[3]) &&
		(bounds[4] < p[2] && p[2] < bounds[5])) {
		return true;
	}
	else {
		return false;
	}
}

std::pair<std::array<double, 3>, double> myResliceCube::MarchVectorUntilBorder(std::array<double, 3> c, std::array<double, 3> v, double* bounds) {
	double gamma = 0.0;
	std::array<double, 3> p;
	while (true) {
		p = c + v * gamma;
		if (isInsideBounds(p, bounds)) {
			//incrementa e continua
			gamma = gamma + 0.1;
		}
		else {
			//para
			break;
		}
	}
	return std::make_pair(p, gamma);
}

std::array<double, 2> myResliceCube::CalculateResliceExtent(myResliceCube *rc) {
	////qual é o centro?
	const std::array<double, 3> center = { { rc->cubeActor->GetCenter()[0], 
											 rc->cubeActor->GetCenter()[1], 
											 rc->cubeActor->GetCenter()[2], } };
	////qual é o vetor?
	const std::array<double, 3> u = { { rc->cubeActor->GetMatrix()->Element[0][0], 
										rc->cubeActor->GetMatrix()->Element[1][0],
										rc->cubeActor->GetMatrix()->Element[2][0], } };
	const auto uMarch = MarchVectorUntilBorder(center, u, rc->boundsDoVolume);
	const std::array<double, 3> uNeg = { { -rc->cubeActor->GetMatrix()->Element[0][0], 
										   -rc->cubeActor->GetMatrix()->Element[1][0], 
										   -rc->cubeActor->GetMatrix()->Element[2][0], } };
	const auto uNegMarch = MarchVectorUntilBorder(center, uNeg, rc->boundsDoVolume);

	const std::array<double, 3> v = { { rc->cubeActor->GetMatrix()->Element[0][1], 
										rc->cubeActor->GetMatrix()->Element[1][1],
										rc->cubeActor->GetMatrix()->Element[2][1], } };
	const auto vMarch = MarchVectorUntilBorder(center, v, rc->boundsDoVolume);
	const std::array<double, 3> vNeg = { { -rc->cubeActor->GetMatrix()->Element[0][1], 
										   -rc->cubeActor->GetMatrix()->Element[1][1], 
										   -rc->cubeActor->GetMatrix()->Element[2][1], } };
	const auto vNegMarch = MarchVectorUntilBorder(center, vNeg, rc->boundsDoVolume);

	const std::array<double, 3> xVector = { { abs(uMarch.first[0] - uNegMarch.first[0]),
		abs(uMarch.first[1] - uNegMarch.first[1]),
		abs(uMarch.first[2] - uNegMarch.first[2]),
		} };
	const std::array<double, 3> yVector = { { abs(vMarch.first[0] - vNegMarch.first[0]),
		abs(vMarch.first[1] - vNegMarch.first[1]),
		abs(vMarch.first[2] - vNegMarch.first[2]),
		} };
	const double normH = sqrt(xVector[0] * xVector[0] + xVector[1] * xVector[1] + xVector[2] * xVector[2]);
	const double normV = sqrt(yVector[0] * yVector[0] + yVector[1] * yVector[1] + yVector[2] * yVector[2]);
	return{ {normH, normV} };
}

void myResliceCube::CreateReslice()
{
	resliceFilter = vtkSmartPointer<vtkImageSlabReslice>::New();
	resliceFilter->SetInputConnection(imageSource->GetOutputPort());
	resliceFilter->SetOutputSpacing(1, 1, 1);
	resliceMatrix->DeepCopy(cubeActor->GetMatrix());
	resliceFilter->SetResliceAxes(resliceMatrix);
	resliceFilter->SetResliceAxesOrigin(cubeActor->GetCenter()[0], cubeActor->GetCenter()[1], cubeActor->GetCenter()[2]);
	resliceFilter->SetOutputDimensionality(2);
	auto resliceExtent = CalculateResliceExtent(this);
	resliceFilter->SetOutputExtent(0, resliceExtent[0]*1.125, 0, resliceExtent[1]*1.125, 0, 1);
	resliceFilter->Update();
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
	boundsDoVolume = nullptr;
	resliceMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
}

void myResliceCube::SetBoundsDoVolume(double * b) {
	boundsDoVolume = b;
	SetEverything();
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

void myResliceCube::OnAfterRenderCallback::Execute(vtkObject * caller, unsigned long event, void * calldata)
{
	std::cout << "foobar" << std::endl;
}
